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
#include "stdio.h"

/*============================================================================*/
/* PRIVATE STRUCTURES */

typedef struct
{
    CAN_TxHeaderTypeDef tx_header;
    uint8_t data[CAN_MAX_DATAFRAME_BYTES];
} CAN_TxMessage_t;

typedef struct
{
    CAN_RxHeaderTypeDef rx_header;
    uint8_t data[CAN_MAX_DATAFRAME_BYTES];
    uint32_t timestamp;
    bool new_rx_message;
} CAN_RxMessage_t;

typedef struct
{
    CAN_HandleTypeDef *can_handle;
    CAN_RxMessage_t rx_message;
    bool new_charger_msg_received;
} CAN_Data_t;

/*============================================================================*/
/* PRIVATE DATA */

static CAN_Data_t CAN_data;

/*============================================================================*/
/* PRIVATE FUNCTION PROTOTYPES */

static void initFilter0x18FF50E5(void);

/*============================================================================*/
/* PUBLIC FUNCTIONS */

/**
 * @brief Initialize CAN data, configures filters and interrupts, and starts the CAN hardware
 *
 * Call this function once before sending any CAN messages
 */
void CAN_Init(CAN_HandleTypeDef *hcan)
{

    memset((uint8_t *)&CAN_data, 0, sizeof(CAN_data));

    CAN_data.can_handle = hcan;

    initFilter0x18FF50E5();

    // any additional filter configuration functions should go here

    HAL_CAN_Start(CAN_data.can_handle);
}

/**
 * @brief Sends a CAN message with ID 0x450 containing battery and low voltage system (LVS) current information.
 *        The function calculates and rescales the battery current and LVS current values and configures the CAN message.
 *
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
    uint32_t tx_mailbox;
    CAN_TxMessage_t txMessage = {0};
    txMessage.tx_header.StdId = 0x450U;
    txMessage.tx_header.DLC = 5U;
    HAL_StatusTypeDef status;

    int16_t batt_current_rescaled = (int16_t)((ecu_data.adc_data.ADC_batt_current * (int16_t)MESSAGE_450_BATT_CURRENT_SCALE_FACTOR) / (int16_t)BATT_CURRENT_LSB_PER_V); // must cast scale_factor and BATT_CURRENT_LSB_PER_V as int16_t or else conversion will not work
    // uint8_t lvs_current_rescaled = (uint8_t)((ecu_data.adc_data.ADC_lvs_current * MESSAGE_450_LVS_CURRENT_SCALE_FACTOR) / LVS_CURRENT_LSB_PER_V); //Tigran says not needed, unsure why but will comment out for now

    txMessage.data[0] = (uint8_t)batt_current_rescaled;
    txMessage.data[1] = (uint8_t)(batt_current_rescaled) >> 8;
    // txMessage.data[2] = lvs_current_rescaled; // Tigran says not needed, unsure why but will comment out for now
    txMessage.data[2] = (uint8_t)ecu_data.adc_data.ADC_supp_batt_volt;
    txMessage.data[3] = (uint8_t)(ecu_data.adc_data.ADC_supp_batt_volt) >> 8;
    txMessage.data[4] = (uint8_t)ecu_data.status.raw;

    do
    {
        status = HAL_CAN_AddTxMessage(CAN_data.can_handle, &txMessage.tx_header, txMessage.data, &tx_mailbox);
    } while (status != HAL_OK && HAL_GetTick() - begin_tick <= CAN_TIMEOUT);
}

/**
 * @brief Sends a CAN message with ID 0x1806E5F4 containing charging parameters and control flags.
 *        The function configures the CAN message with maximum charging current, maximum charging voltage,
 *        charger enable/disable status, and charging mode information.
 *
 * @note This function toggles the charger enable status for subsequent calls, starting the charger when
 *       the enable status is set to 0.
 *
 * @date 2023/12/02
 * @author Harris Mai (harristmai)
 */
void CAN_SendMessage1806E5F4()
{
    uint32_t begin_tick = HAL_GetTick();
    uint32_t tx_mailbox;
    CAN_TxMessage_t txMessage = {0};
    txMessage.tx_header.IDE = CAN_ID_EXT;
    txMessage.tx_header.ExtId = CHARGER_ENABLE_MESSAGE_ID;
    txMessage.tx_header.DLC = 6U; // technically 8 bytes but last 2 are reserved, note this when testing
    HAL_StatusTypeDef status;
    static uint8_t charger_enable = 0;
    static uint8_t charger_switch = 0; // for now charger will always be outputting

    txMessage.data[0] = (uint8_t)(MAX_CHARGING_VOLTAGE >> 8);
    txMessage.data[1] = (uint8_t)MAX_CHARGING_VOLTAGE;
    txMessage.data[2] = (uint8_t)(MAX_CHARGING_CURRENT >> 8);
    txMessage.data[3] = (uint8_t)(MAX_CHARGING_CURRENT);
    txMessage.data[4] = charger_switch; // Charger Switch: 0 = Close output (Voltage and current will flow), 1: Open output (Voltage and current will slowly drop to 0)
    txMessage.data[5] = charger_enable; // Charger Enable: 0 = Charger receives CAN message but will not close output even if byte 4 tells it to (stop/standby mode), 1: Charger will process request from byte 4 (start)

    do
    {
        status = HAL_CAN_AddTxMessage(CAN_data.can_handle, &txMessage.tx_header, txMessage.data, &tx_mailbox);
    } while (status != HAL_OK && HAL_GetTick() - begin_tick <= CAN_TIMEOUT);

    if (charger_enable == 0)
    {
        charger_enable = 1;
    }
}

/**
 * @brief Checks whether a message with a specific ID is pending in a specified RX FIFO (rx mailbox).
 * If the mailbox is not empty, retrieve all messages in the mailbox, and check their IDs.
 *
 * @arg rx_fifo Received mailbox (fifo) in which to check for a pending message, can be of value @arg CAN_receive_FIFO_number
 *
 * @note If a retrieved message's contents is to be used, the message must be stored somewhere
 */
void CAN_CheckRxMessages(uint32_t rx_fifo)
{
    uint8_t rx_fifo_fill_level;

    // Get number of pending messages
    rx_fifo_fill_level = HAL_CAN_GetRxFifoFillLevel(CAN_data.can_handle, rx_fifo);
    if (rx_fifo_fill_level > 0)
    {
        for (int i = 0; i < rx_fifo_fill_level; i++)
        {
            // Retrieve message
            if (HAL_CAN_GetRxMessage(CAN_data.can_handle, rx_fifo, (CAN_RxHeaderTypeDef *)&CAN_data.rx_message.rx_header, (uint8_t *)CAN_data.rx_message.data) != HAL_OK) // retrieve message
            {
                Error_Handler();
            }

            // Preform checks/store retrieved message
            if (CAN_data.rx_message.rx_header.ExtId == CHARGER_STATUS_MESSAGE_ID)
            {
                CAN_data.new_charger_msg_received = true;
            }
        }
    }
}

/**
 * @brief Checks whether the HK-MF-103-32's status message has been received
 *
 * @returns true if the message was received, otherwise false.
 */
bool CAN_CheckRxChargerMessage()
{
    if (CAN_data.new_charger_msg_received)
    {
        CAN_data.new_charger_msg_received = false;
        return true;
    }

    CAN_data.new_charger_msg_received = false;
    return false;
}

/*============================================================================*/
/* PRIVATE FUNCTIONS */

/**
 * @brief Configure recieve filters for message 0x450 (from the ECU)
 * Additional functions should be created to configure different filter banks
 * This function is SPECIFICALLY for the filter bank for message 0x450
 */
static void initFilter0x18FF50E5(void)
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

    filter_config.FilterActivation = CAN_FILTER_ENABLE;                          // enable filters
    filter_config.SlaveStartFilterBank = 0;                                      // only one CAN interface, parameter meaningless (all filter banks for the one controller)
    filter_config.FilterBank = 0;                                                // settings applied for filterbank 0
    filter_config.FilterFIFOAssignment = CAN_FILTER_FIFO0;                       // rx'd message will be placed into this FIFO
    filter_config.FilterMode = CAN_FILTERMODE_IDLIST;                            // identifier list mode
    filter_config.FilterScale = CAN_FILTERSCALE_32BIT;                           // don't need double layer of filters (if rx'ing many messages with diff ID's, could use double layer of filters)
    filter_config.FilterMaskIdHigh = 0xFFFF;                                     // ID upper 16 bits (not using mask), bit shift per bit order (see large comment above)
    filter_config.FilterMaskIdLow = 0xFFFC;                                      // ID lower 16 bits (not using mask), all 0 means standard ID, RTR mode = data
    filter_config.FilterIdHigh = (CHARGER_STATUS_MESSAGE_ID >> 13) & 0xFFFF;     // filter ID upper 16 bits (list mode, mask = ID)
    filter_config.FilterIdLow = ((CHARGER_STATUS_MESSAGE_ID << 3) & 0xFFF8) | 4; // filter ID lower 16 bits, all 0 means standard ID, RTR mode = data

    if (HAL_CAN_ConfigFilter(CAN_data.can_handle, &filter_config) != HAL_OK)
    {
        Error_Handler();
    }
}