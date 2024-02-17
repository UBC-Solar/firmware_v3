/**
 * @file can.c
 * @brief Functions for sending and receiving messages over the Master board's CAN bus
 * 
 * Multi-byte values are generally sent in big-endian byte order (network order) = most significant byte first
 *
 * @date October 1, 2022
 * @author Edward Ma, Jung Yi Cau, Mischa Johal
 */

#include "can.h"
#include "main.h"
#include <float.h>
#include <string.h>

/*============================================================================*/
/* DEFINITIONS */

#define CAN_MAX_DATAFRAME_BYTES 8U
#define NUM_CAN_TX_MAILBOXES 3U

// Change as needed to accomodate all BMS CAN messages transmitted around the same time if set of messages is changed
#define TX_QUEUE_MAX_NUM_MESSAGES 32U

#define MESSAGE_623_MODULE_VOLTAGE_MAX_VALUE 5U
#define MESSAGE_623_PACK_VOLTAGE_MAX_VALUE 140U

#define MESSAGE_623_MODULE_VOLTAGE_SCALE_FACTOR (0xFFU / (MESSAGE_623_MODULE_VOLTAGE_MAX_VALUE))
#define MESSAGE_623_PACK_VOLTAGE_SCALE_FACTOR (0xFFFFU / (MESSAGE_623_PACK_VOLTAGE_MAX_VALUE))

#define MODULES_PER_MULTIPLEXED_DATA_MESSAGE 4U
#define NUM_MULTIPLEXED_DATA_MESSAGES (PACK_NUM_BATTERY_MODULES / MODULES_PER_MULTIPLEXED_DATA_MESSAGE)

#define ECU_CURRENT_MESSAGE_ID 0x450U

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
    volatile CAN_TxMessage_t tx_queue[TX_QUEUE_MAX_NUM_MESSAGES];
    volatile uint32_t tx_queue_push_index;
    volatile uint32_t tx_queue_pop_index;
    volatile CAN_RxMessage_t rx_message_0x450;
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
static void initFilter0x450(void)
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
    filter_config.FilterMaskIdHigh = ECU_CURRENT_MESSAGE_ID << 5;   // ID upper 16 bits (not using mask), bit shift per bit order (see large comment above)
    filter_config.FilterMaskIdLow = 0;                              // ID lower 16 bits (not using mask), all 0 means standard ID, RTR mode = data
    filter_config.FilterIdHigh = ECU_CURRENT_MESSAGE_ID << 5;       // filter ID upper 16 bits (list mode, mask = ID)
    filter_config.FilterIdLow = 0;                                  // filter ID lower 16 bits, all 0 means standard ID, RTR mode = data

    if (HAL_CAN_ConfigFilter(CAN_data.can_handle, &filter_config) != HAL_OK)
    {
        Error_Handler();
    }
}

static void populateCanTxMailbox(void)
{
    uint32_t mailbox;

    if (HAL_CAN_GetTxMailboxesFreeLevel(CAN_data.can_handle) == 0)
    {
        return;
    }

    CAN_TxMessage_t *next_message = (CAN_TxMessage_t *) &CAN_data.tx_queue[CAN_data.tx_queue_pop_index];

    // Initiate CAN transfer
    if (HAL_OK != HAL_CAN_AddTxMessage(CAN_data.can_handle, &next_message->tx_header, next_message->data, &mailbox))
    {
        Error_Handler();
    }
    CAN_data.tx_queue_pop_index = (CAN_data.tx_queue_pop_index + 1U) % TX_QUEUE_MAX_NUM_MESSAGES;
}

/**
 * Queue a message to send over CAN. Commands will be sent in order of queuing as soon as bus is available after calling startTx().
 *
 * This function is non-blocking and commands will be transmitted asynchronously.
 * The message buffer is copied into a queue buffer before the function returns.
 *
 * @param message Pointer to buffer containing message to queue
 */
static void queueCanMessage(CAN_TxMessage_t *message)
{
    HAL_NVIC_DisableIRQ(USB_HP_CAN1_TX_IRQn); // Start critical section - do not want a CAN TX complete interrupt to be serviced during this function call

    uint32_t next_push_index = (CAN_data.tx_queue_push_index + 1U) % TX_QUEUE_MAX_NUM_MESSAGES;

    // Check if there is space in queue
    if (next_push_index == CAN_data.tx_queue_pop_index)
    {
        Error_Handler();
    }

    // Add command to transmit queue
    volatile CAN_TxMessage_t *next_free_queue_slot = &CAN_data.tx_queue[CAN_data.tx_queue_push_index];
    memcpy((uint8_t *) next_free_queue_slot, (uint8_t *) message, sizeof(CAN_TxMessage_t));

    // Move push index forward
    CAN_data.tx_queue_push_index = next_push_index;

    // Initiate message transmission if possible
    populateCanTxMailbox();

    HAL_NVIC_EnableIRQ(USB_HP_CAN1_TX_IRQn); // End critical section
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
    if (HAL_CAN_ActivateNotification(CAN_data.can_handle, CAN_IT_TX_MAILBOX_EMPTY | CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK)
    {
        Error_Handler();
    }

    initFilter0x450();
    // any additional filter configuration functions should go here

    HAL_CAN_Start(CAN_data.can_handle);
}

/**
 * @brief Get data for message 622, construct CAN message and send it
 *
 * @param pack pack data structure that data will be read from
 */
void CAN_SendMessage622(Pack_t *pack)
{
    CAN_TxMessage_t txMessage = {0};
    txMessage.tx_header.StdId = 0x622U;
    txMessage.tx_header.DLC = 7;

    uint32_t status = pack->status.raw;
    txMessage.data[0] = (uint8_t)status;         // bits 0-7
    txMessage.data[1] = (uint8_t)(status >> 8);  // 8-15
    txMessage.data[2] = (uint8_t)(status >> 16); // 16, 17 (garbage beyond this)

    queueCanMessage(&txMessage);
}

/**
 * @brief Get data for message 623, construct CAN message and send it
 *
 * @param pack pack data structure that data will be read from
 */
void CAN_SendMessage623(Pack_t *pack)
{
    CAN_TxMessage_t txMessage = {0};
    txMessage.tx_header.StdId = 0x623U;
    txMessage.tx_header.DLC = 6;

    uint16_t min_module_voltage = 0xFFFFU;
    uint16_t max_module_voltage = 0;
    uint16_t module_voltage;
    // locations of min and max voltage
    uint8_t min_module = 0;
    uint8_t max_module = 0;

    uint16_t total_pack_voltage = (uint16_t) ((Pack_GetPackVoltage(pack) * MESSAGE_623_PACK_VOLTAGE_SCALE_FACTOR) / PACK_MODULE_VOLTAGE_LSB_PER_V);

    for (uint8_t module_num = 0; module_num < PACK_NUM_BATTERY_MODULES; module_num++)
    {
        module_voltage = pack->module[module_num].voltage;
        // check and store minimum voltage
        if (module_voltage < min_module_voltage)
        {
            min_module_voltage = module_voltage;    // store minimum voltage
            min_module = module_num;                // store module with minimum voltage
        }
        // check and store maximum voltage
        if (module_voltage > max_module_voltage)
        {
            max_module_voltage = module_voltage;
            max_module = module_num;
        }
    }

    // Rescale voltages to fit 8 bits of data
    uint8_t min_volt_rescaled = (uint8_t) ((min_module_voltage * MESSAGE_623_MODULE_VOLTAGE_SCALE_FACTOR) / PACK_MODULE_VOLTAGE_LSB_PER_V);
    uint8_t max_volt_rescaled = (uint8_t) ((max_module_voltage * MESSAGE_623_MODULE_VOLTAGE_SCALE_FACTOR) / PACK_MODULE_VOLTAGE_LSB_PER_V);

    // Store in message 623 data array
    txMessage.data[0] = (uint8_t)total_pack_voltage;        // LSB
    txMessage.data[1] = (uint8_t)(total_pack_voltage >> 8); // MSB
    txMessage.data[2] = min_volt_rescaled;
    txMessage.data[3] = min_module; // add one because of indexing from zero
    txMessage.data[4] = max_volt_rescaled;
    txMessage.data[5] = max_module;

    queueCanMessage(&txMessage);
}

/**
 * @brief Get data for message 624, construct CAN message and send it
 *
 * @param pack pack data structure that data will be read from
 */
void CAN_SendMessage624(Pack_t *pack)
{
    CAN_TxMessage_t txMessage = {0};
    txMessage.tx_header.StdId = 0x624U;
    txMessage.tx_header.DLC = 7;

    float soc = Pack_GetPackStateOfCharge(pack);
    float capacity = SOC_getCapacity(soc);
    float dod = SOC_getDOD(capacity);

    uint16_t capacity_encoded = (uint16_t) capacity;
    uint16_t dod_encoded = (uint16_t) dod;

    txMessage.data[0] = (uint8_t) soc;
    txMessage.data[1] = (uint8_t) (dod_encoded >> 8);
    txMessage.data[2] = (uint8_t) dod_encoded;
    txMessage.data[3] = (uint8_t) (capacity_encoded >> 8);
    txMessage.data[4] = (uint8_t) capacity_encoded;

    queueCanMessage(&txMessage);
}

/**
 * @brief Get data for message 625, construct CAN message and send it
 *
 * @param pack pack data structure that data will be read from
 */
void CAN_SendMessage625(Pack_t *pack)
{
    CAN_TxMessage_t txMessage = {0};
    txMessage.tx_header.StdId = 0x625U;
    txMessage.tx_header.DLC = 5;

    // Temperature measured in degrees celcius
    float sum = 0.0;
    float average_pack_temperature;
    float min_module_temperature = FLT_MIN;
    float max_module_temperature = FLT_MAX;
    float module_temperature;
    // indices of modules with min and max temperatures
    uint8_t min_module = 0;
    uint8_t max_module = 0;

    for (int module_num = 0; module_num < PACK_NUM_BATTERY_MODULES; module_num++)
    {
        module_temperature = (pack->module[module_num].temperature);

        sum += module_temperature; // Get sum of temperature

        // check and store minimum temperature
        if (module_temperature < min_module_temperature)
        {
            min_module_temperature = module_temperature;   // store minimum temperature
            min_module = module_num;                // store index of module with minimum temperature
        }
        // check and store maximum temperature
        if (module_temperature > max_module_temperature)
        {
            max_module_temperature = module_temperature;
            max_module = module_num;
        }
    }

    average_pack_temperature = sum / PACK_NUM_BATTERY_MODULES;

    // Populate data array
    // Cast signed values to a signed integer format first in order to preserve value while converting number format
    txMessage.data[0] = (int8_t) average_pack_temperature;
    txMessage.data[1] = (int8_t) min_module_temperature;
    txMessage.data[2] = min_module;
    txMessage.data[3] = (int8_t) max_module_temperature;
    txMessage.data[4] = max_module;

    queueCanMessage(&txMessage);
}

/**
 * @brief Get data for set of messages for ID 626, construct CAN messages and send them
 *
 * @param pack pack data structure that data will be read from
 */
void CAN_SendMessages626(Pack_t *pack)
{
    CAN_TxMessage_t txMessage = {0};
    txMessage.tx_header.StdId = 0x626U;
    txMessage.tx_header.DLC = 5;

    for (uint8_t multiplex_index = 0; multiplex_index < NUM_MULTIPLEXED_DATA_MESSAGES; multiplex_index++)
    {
        uint32_t base_module_num = multiplex_index * MODULES_PER_MULTIPLEXED_DATA_MESSAGE;
        txMessage.data[0] = multiplex_index;
        txMessage.data[1] = ((pack->module[base_module_num     ].voltage * MESSAGE_623_MODULE_VOLTAGE_SCALE_FACTOR) / PACK_MODULE_VOLTAGE_LSB_PER_V);
        txMessage.data[2] = ((pack->module[base_module_num + 1U].voltage * MESSAGE_623_MODULE_VOLTAGE_SCALE_FACTOR) / PACK_MODULE_VOLTAGE_LSB_PER_V);
        txMessage.data[3] = ((pack->module[base_module_num + 2U].voltage * MESSAGE_623_MODULE_VOLTAGE_SCALE_FACTOR) / PACK_MODULE_VOLTAGE_LSB_PER_V);
        txMessage.data[4] = ((pack->module[base_module_num + 3U].voltage * MESSAGE_623_MODULE_VOLTAGE_SCALE_FACTOR) / PACK_MODULE_VOLTAGE_LSB_PER_V);
        queueCanMessage(&txMessage);
    }
}

/**
 * @brief Get data for set of messages for ID 627, construct CAN messages and send them
 *
 * @param pack pack data structure that data will be read from
 */
void CAN_SendMessages627(Pack_t *pack)
{
    CAN_TxMessage_t txMessage = {0};
    txMessage.tx_header.StdId = 0x627U;
    txMessage.tx_header.DLC = 5;

    for (uint8_t multiplex_index = 0; multiplex_index < NUM_MULTIPLEXED_DATA_MESSAGES; multiplex_index++)
    {
        uint32_t base_module_num = multiplex_index * MODULES_PER_MULTIPLEXED_DATA_MESSAGE;
        txMessage.data[0] = multiplex_index;
        // Cast to int8_t in order for sign to be dealt with correctly (before implicit cast to uint8_t)
        txMessage.data[1] = (int8_t) (pack->module[base_module_num     ].temperature);
        txMessage.data[2] = (int8_t) (pack->module[base_module_num + 1U].temperature);
        txMessage.data[3] = (int8_t) (pack->module[base_module_num + 2U].temperature);
        txMessage.data[4] = (int8_t) (pack->module[base_module_num + 3U].temperature);
        queueCanMessage(&txMessage);
    }
}

/**
 * @brief Get data for set of messages for ID 628, construct CAN messages and send them
 *
 * @param pack pack data structure that data will be read from
 */
void CAN_SendMessages628(Pack_t *pack)
{
    CAN_TxMessage_t txMessage = {0};
    txMessage.tx_header.StdId = 0x628U;
    txMessage.tx_header.DLC = 5;
    uint8_t status_bits = 0;
    
    for(uint8_t multiplex_index = 0; multiplex_index < NUM_MULTIPLEXED_DATA_MESSAGES; multiplex_index++)
    {
        uint32_t base_module_num = multiplex_index * MODULES_PER_MULTIPLEXED_DATA_MESSAGE;
        txMessage.data[0] = multiplex_index;

        for (int index = 1; index <= 4; index++)
        {
            status_bits |= (pack->module[ base_module_num + (index - 1) ].status.bits.fault_over_temperature);
            status_bits |= (pack->module[ base_module_num + (index - 1) ].status.bits.fault_under_voltage)<< 1;
            status_bits |= (pack->module[ base_module_num + (index - 1) ].status.bits.fault_over_voltage)<< 2;
            status_bits |= (pack->module[ base_module_num + (index - 1) ].status.bits.charge_over_temperature_limit)<< 3;
            status_bits |= (pack->module[ base_module_num + (index - 1) ].status.bits.warning_low_voltage)<< 4;
            status_bits |= (pack->module[ base_module_num + (index - 1) ].status.bits.warning_high_voltage)<< 5;
            status_bits |= (pack->module[ base_module_num + (index - 1) ].status.bits.warning_high_temperature)<< 6;

            txMessage.data[index] = status_bits;
            status_bits = 0;
        }
        queueCanMessage(&txMessage);
    }
}

/**
 * @brief Get data for set of messages for ID 629, construct CAN messages and send them
 *
 * @param pack pack data structure that data will be read from
 */
void CAN_SendMessages629(Pack_t * pack)
{
    CAN_TxMessage_t txMessage = {0};
    txMessage.tx_header.StdId = 0x629U;
    txMessage.tx_header.DLC = 4;
    uint32_t balancing_status_raw = 0;

    for(int module_num = 0; module_num < PACK_NUM_BATTERY_MODULES; module_num++)
    {
        balancing_status_raw |= pack->module[module_num].status.bits.balancing_active << module_num;
    }

    txMessage.data[0] = (uint8_t)balancing_status_raw >> 0;
    txMessage.data[1] = (uint8_t)balancing_status_raw >> 8;
    txMessage.data[2] = (uint8_t)balancing_status_raw >> 16;
    txMessage.data[3] = (uint8_t)balancing_status_raw >> 24;

    queueCanMessage(&txMessage);
}

/**
 * @brief Getter for data contained in the last received ECU current data CAN message
 * 
 * @param batt_current Signed pack current scaled, to get in current in A divide by 65.535
 * @param supp_batt_volt Unsigned supplemental battery voltage scaled, divide by 1000 to get voltage in V
 * @param status Refer to Solar CAN ID Excel Sheet for specifics on each bit, last 3 bits are reserved
 * @param[out] rx_timestamp Time since board power on in ms at which last ECU CAN message was received
 * @returns Whether a CAN message has been received (and there is new data) since the last time this function was called
*/
bool CAN_GetMessage0x450Data(uint32_t *rx_timestamp, ECU_Data_t *ecu_data)
{

    HAL_NVIC_DisableIRQ(USB_LP_CAN1_RX0_IRQn); // Start critical section - do not want a CAN RX complete interrupt to be serviced during this function call
    bool new_rx_message = CAN_data.rx_message_0x450.new_rx_message;

    if(new_rx_message == true){

        CAN_data.rx_message_0x450.new_rx_message = false;

        ecu_data->adc_data.batt_current = ((int16_t)(CAN_data.rx_message_0x450.data[1])) << 8  | (int16_t)(CAN_data.rx_message_0x450.data[0]);
        ecu_data->adc_data.supp_batt_volt = ((uint8_t)(CAN_data.rx_message_0x450.data[3])) << 8 | (uint16_t)(CAN_data.rx_message_0x450.data[2]);
        ecu_data->adc_data.lvs_current = (int8_t) CAN_data.rx_message_0x450.data[4];        
        ecu_data->status.raw = CAN_data.rx_message_0x450.data[5];
        *rx_timestamp = CAN_data.rx_message_0x450.timestamp;

    }
    
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
    if (HAL_CAN_GetRxMessage(CAN_data.can_handle, 0, (CAN_RxHeaderTypeDef *) &CAN_data.rx_message_0x450.rx_header, (uint8_t *) CAN_data.rx_message_0x450.data) != HAL_OK) // retrieve message
    {
        Error_Handler();
    }

    if (CAN_data.rx_message_0x450.rx_header.StdId != ECU_CURRENT_MESSAGE_ID)
    {
        // There is likely a problem with the filter configuration
        Error_Handler();
    }

    CAN_data.rx_message_0x450.timestamp = HAL_GetTick();
    CAN_data.rx_message_0x450.new_rx_message = true;
}

/**
 * @brief Handle a CAN TX complete interrupt for any of the 3 CAN TX mailboxes
 * 
 * This function needs to be called from all of:
 * HAL_CAN_TxMailbox0CompleteCallback(),
 * HAL_CAN_TxMailbox1CompleteCallback(), and
 * HAL_CAN_TxMailbox2CompleteCallback()
 */
void CAN_TxCompleteCallback(void)
{
    if (CAN_data.tx_queue_pop_index != CAN_data.tx_queue_push_index)
    {
        // Initiate next message transmission if possible
        populateCanTxMailbox();
    }
    else
    {
        // No more messages in the queue to send
    }
}

/**
 * @brief Handle a CAN TX complete interrupt with error for any of the 3 CAN TX mailboxes
 * 
 * This function needs to be called from HAL_CAN_ErrorCallback()
 * 
 * @note This function has not been written to handle the CAN error interrupt,
 * just a TX complete interrupt for TX completion with errors
 */
void CAN_ErrorCallback(void)
{
    // This callback is needed if automatic retransmission is DISABLED in the CAN peripheral's configuration
    // If board is disconnected from CAN bus, TX requests will complete with error
    // due to lack of CAN message acknowledgement from another device.
    // Continue as if this was a clean transmission completion; note that multiple
    // TX mailboxes may need an error serviced in one interrupt

    // With automatic retransmission ENABLED, and board is disconnected from CAN bus or no other devices on bus are active),
    // messages go unacknowledged, do not complete and are repeatedly sent indefinitely.
    // The CAN_data.tx_queue will overflow and the firmware will fault

    if (CAN_data.can_handle->ErrorCode & (HAL_CAN_ERROR_TX_TERR0))
    {
        CAN_TxCompleteCallback();
    }
    if (CAN_data.can_handle->ErrorCode & (HAL_CAN_ERROR_TX_TERR1))
    {
        CAN_TxCompleteCallback();
    }
    if (CAN_data.can_handle->ErrorCode & (HAL_CAN_ERROR_TX_TERR2))
    {
        CAN_TxCompleteCallback();
    }
}
