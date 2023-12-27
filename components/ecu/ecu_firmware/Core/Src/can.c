/**
 * @file can.c
 * @brief CAN communication functions for ECU
 * 
 * @date 2021/01/30
 * @author Blake Shular (blake-shular)
 * @author Harris Mai (harristmai)
 */

#include "can.h"
#include <float.h>
#include <string.h>
#include "common.h"

/*============================================================================*/
/* DEFINITIONS */

#define MAX_CHARGING_CURRENT 130U  // 21 A 0.1A/bit offset //update to 13 max outlet current is 15 A
#define MAX_CHARGING_VOLTAGE 1344U  // 134.4 V 0.1V/bit offset

#define CAN_MAX_DATAFRAME_BYTES 8U
#define OBC_MESSAGE_ID 0x3E5U

#define MESSAGE_450_LVS_CURRENT_MAX_VALUE 30U //30A
#define MESSAGE_450_BATT_CURRENT_MAX_VALUE 100U //100A

#define LVS_CURRENT_LSB_PER_V 1000U // LVS current reading from ADC in mA
#define BATT_CURRENT_LSB_PER_V 10000U //BATT current reading from ADC in mA but must divide by extra 10 to fit into int16_t because it is signed

#define MESSAGE_450_LVS_CURRENT_SCALE_FACTOR (0xFFU / (MESSAGE_450_LVS_CURRENT_MAX_VALUE))
#define MESSAGE_450_BATT_CURRENT_SCALE_FACTOR (0xFFFFU / (MESSAGE_450_BATT_CURRENT_MAX_VALUE))

#define OBC_STATUS_MESSAGE_ID 0x3E5U

typedef struct {
    CAN_TxHeaderTypeDef tx_header;
    uint8_t data[CAN_MAX_DATAFRAME_BYTES];
} CAN_TxMessage_t;

typedef struct {
    CAN_RxHeaderTypeDef rx_header;
    uint8_t data[CAN_MAX_DATAFRAME_BYTES];
    uint32_t timestamp;
    bool new_rx_message;
} CAN_RxMessage_t;

typedef struct {
    CAN_HandleTypeDef *can_handle;
    volatile CAN_RxMessage_t rx_message_0x3E5;
} CAN_Data_t;

/*============================================================================*/
/* PRIVATE DATA */

static CAN_Data_t CAN_data;

/*============================================================================*/
/* PRIVATE FUNCTIONS */

/**
 * @brief Configure recieve filters for message 0x450 (from the ECU)
 * Additional functions should be created to configure different filter banks
 * This function is SPECIFICALLY for the filter bank for message 0x450
 */
static void initFilter0x3E5(void)
{
    /*
    Filter Information (see page 644 onward of stm32f103 reference manual)

        In ARM, CAN subsystem known as bxCAN (basic-extended)
        14 configurable filter banks (STM32F103C8 has only one CAN interface)
        Each filter bank consists of two, 32-bit registers, CAN_FxR0 and CAN_FxR1
        Depending on filter scale, filter bank provides one 32-bit filter for mask and id, or two 16 bit filters (each) for id

    Mask mode: Use mask to enable/disable the bits of the filter you want to check the CAN ID against
    Identifier list mode: mask registers used as identifier registers (incoming ID must match exactly)

    For 32 bit filter, [31:21] map to STID [10:0], other bits we don't care about

    Good explanation on mask mode: https://www.microchip.com/forums/m456043.aspx
    What I followed for filter config: https://controllerstech.com/can-protocol-in-stm32/
    */

    CAN_FilterTypeDef filter_config;

    filter_config.FilterActivation = CAN_FILTER_ENABLE;             // enable filters
    filter_config.SlaveStartFilterBank = 14;                        // only one CAN interface, parameter meaningless (all filter banks for the one controller)
    filter_config.FilterBank = 0;                                   // settings applied for filterbank 0
    filter_config.FilterFIFOAssignment = CAN_FILTER_FIFO0;          // rx'd message will be placed into this FIFO
    filter_config.FilterMode = CAN_FILTERMODE_IDLIST;               // identifier list mode
    filter_config.FilterScale = CAN_FILTERSCALE_32BIT;              // don't need double layer of filters, not using EXTID, 32bit fine (if rx'ing many messages with diff ID's, could use double layer of filters)
    filter_config.FilterMaskIdHigh = OBC_MESSAGE_ID << 5;   // ID upper 16 bits (not using mask), bit shift per bit order (see large comment above)
    filter_config.FilterMaskIdLow = 0;                              // ID lower 16 bits (not using mask), all 0 means standard ID, RTR mode = data
    filter_config.FilterIdHigh = OBC_MESSAGE_ID << 5;       // filter ID upper 16 bits (list mode, mask = ID)
    filter_config.FilterIdLow = 0;                                  // filter ID lower 16 bits, all 0 means standard ID, RTR mode = data

    if (HAL_CAN_ConfigFilter(CAN_data.can_handle, &filter_config) != HAL_OK)
    {
        Error_Handler();
    }
}

/*============================================================================*/
/* PUBLIC FUNCTION IMPLEMENTATIONS */

/**
 * @brief Initialize CAN data, configures filters and interrupts, and starts the CAN hardware
 * 
 * Call this function once before sending any CAN messages
 */
void CAN_Init(CAN_HandleTypeDef *hcan)
{
    memset((uint8_t *) &CAN_data, 0, sizeof(CAN_data));

    CAN_data.can_handle = hcan;

    // Activate interrupt for completion of message transmission
    if (HAL_CAN_ActivateNotification(CAN_data.can_handle, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK)
    {
        Error_Handler();
    }

    initFilter0x3E5();
    // any additional filter configuration functions should go here

    HAL_CAN_Start(CAN_data.can_handle);
}

/**
 * @brief Sends a CAN message with ID 0x450 containing battery and low voltage system (LVS) current information.
 *        The function calculates and rescales the battery current and LVS current values and configures the CAN message.
 *
 * @param ECU Pointer to the ECU_t structure containing ECU-related data, including ADC readings.
 *
 * @note The function performs rescaling on ADC readings and configures the CAN message with the rescaled values.
 * @note If retrieving batt_current_rescaled from CAN bus, you must combine the upper and lower byte and cast it back to data type int16_t before use.
 *
 * @date 2022/12/02
 * @author Harris Mai (harristmai)
 */
void CAN_SendMessage450()
{
    uint32_t begin_tick = HAL_GetTick();
    uint32_t mailbox = 0;
    CAN_TxMessage_t txMessage = {0};
    txMessage.tx_header.StdId = 0x450U;
    txMessage.tx_header.DLC = 6U;
    HAL_StatusTypeDef status;

    int16_t batt_current_rescaled = (int16_t)((ecu_data.adc_data.ADC_batt_current * (int16_t)MESSAGE_450_BATT_CURRENT_SCALE_FACTOR) / (int16_t)BATT_CURRENT_LSB_PER_V); //must cast scale_factor and BATT_CURRENT_LSB_PER_V as int16_t or else conversion will not work
    uint8_t lvs_current_rescaled = (uint8_t)((ecu_data.adc_data.ADC_lvs_current * MESSAGE_450_LVS_CURRENT_SCALE_FACTOR) / LVS_CURRENT_LSB_PER_V);

    txMessage.data[0] = (uint8_t)batt_current_rescaled; 
    txMessage.data[1] = (uint8_t)(batt_current_rescaled) >> 8; 
    txMessage.data[2] = lvs_current_rescaled; 
    txMessage.data[3] = (uint8_t)ecu_data.adc_data.ADC_supp_batt_volt;
    txMessage.data[4] = (uint8_t)(ecu_data.adc_data.ADC_supp_batt_volt) >> 8;
    txMessage.data[5] = (uint8_t)ecu_data.status.raw;

    do {
        status = HAL_CAN_AddTxMessage(CAN_data.can_handle, &txMessage.tx_header, txMessage.data, &mailbox);
    } while (status != HAL_OK && HAL_GetTick() - begin_tick <= CAN_TIMEOUT);
}


/**
 * @brief Sends a CAN message with ID 0x3F4 containing charging parameters and control flags.
 *        The function configures the CAN message with maximum charging current, maximum charging voltage,
 *        charger enable/disable status, and charging mode information. 
 *
 * @note This function toggles the charger enable status for subsequent calls, starting the charger when
 *       the enable status is set to 0. 
 * 
 * @date 2023/12/02
 * @author Harris Mai (harristmai)
 */
void CAN_SendMessage3F4()
{
    uint32_t begin_tick = HAL_GetTick();
    uint32_t mailbox;
    CAN_TxMessage_t txMessage = {0};
    txMessage.tx_header.StdId = 0x3F4U;
    txMessage.tx_header.DLC = 6U;
    HAL_StatusTypeDef status;
    static uint8_t charger_enable = 0;

    txMessage.data[0] = (uint8_t) MAX_CHARGING_CURRENT;
    txMessage.data[1] = (uint8_t) (MAX_CHARGING_CURRENT >> 8);
    txMessage.data[2] = (uint8_t)MAX_CHARGING_VOLTAGE;
    txMessage.data[3] = (uint8_t)(MAX_CHARGING_VOLTAGE >> 8);
    txMessage.data[4] = charger_enable; //Charger Enable: 0 = Start Charger (no current flowing?), 1: Close output of charger
    txMessage.data[5] = 0; //Charging Mode: 0 = Charging Mode

    do {
        status = HAL_CAN_AddTxMessage(CAN_data.can_handle, &txMessage.tx_header, txMessage.data, &mailbox);
    } while (status != HAL_OK && HAL_GetTick() - begin_tick <= CAN_TIMEOUT);

    if (charger_enable == 0){
        charger_enable = 1;
    }
    //TODO: Check w/ PCAN
}



/**
 * THIS FUNCTION IS NOT COMPLETE
 * @brief Getter for data contained in the last received ECU current data CAN message
 * 
 * @param[out] pack_current Signed pack current in amps
 * @param[out] low_voltage_current Signed low voltage circuits current; LSB = (30/255) amps
 * @param[out] overcurrent_status True if discharge or charge over-current condition has been triggered
 * @param[out] rx_timestamp Time since board power on in ms at which last ECU CAN message was received
 * @returns Whether a CAN message has been received (and there is new data) since the last time this function was called
*/
bool CAN_GetMessage0x3E5Data(int16_t *pack_current, uint8_t *low_voltage_current, bool *overcurrent_status, uint32_t *rx_timestamp)
{
    HAL_NVIC_DisableIRQ(USB_LP_CAN1_RX0_IRQn); // Start critical section - do not want a CAN RX complete interrupt to be serviced during this function call
    bool new_rx_message = CAN_data.rx_message_0x3E5.new_rx_message;
    CAN_data.rx_message_0x3E5.new_rx_message = false;

    *pack_current = (int8_t) CAN_data.rx_message_0x450.data[0];
    *low_voltage_current = CAN_data.rx_message_0x450.data[1];
    *overcurrent_status = CAN_data.rx_message_0x450.data[2] & 0x1U;
    *rx_timestamp = CAN_data.rx_message_0x450.timestamp;

    HAL_NVIC_DisableIRQ(USB_LP_CAN1_RX0_IRQn); // Start critical section
    return new_rx_message;
}

/*============================================================================*/
/* PUBLIC CALLBACK FUNCTION IMPLEMENTATIONS */

/**
 * @brief Handle a CAN RX message pending interrupt for the FIFO configured to receive ECU current message
 * 
 * This function needs to be called from the CAN RX message pending interrupt callback for the appropriate FIFO
 * (as determined) by the filter configuration; eg. HAL_CAN_RxFifo0MsgPendingCallback()
 */
void CAN_RecievedMessageCallback(void)
{
    if (HAL_CAN_GetRxMessage(CAN_data.can_handle, 0, (CAN_RxHeaderTypeDef *) &CAN_data.rx_message_0x3E5.rx_header, (uint8_t *) CAN_data.rx_message_0x3E5.data) != HAL_OK) // retrieve message
    {
        Error_Handler();
    }

    if (CAN_data.rx_message_0x3E5.rx_header.StdId != OBC_STATUS_MESSAGE_ID)
    {
        // There is likely a problem with the filter configuration
        Error_Handler();
    }

    CAN_data.rx_message_0x3E5.timestamp = HAL_GetTick();
    CAN_data.rx_message_0x3E5.new_rx_message = true;
}