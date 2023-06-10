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

/*============================================================================*/
/* DEFINITIONS */

#define MESSAGE_623_MODULE_VOLTAGE_MAX_VALUE 5U
#define MESSAGE_623_PACK_VOLTAGE_MAX_VALUE 140U

#define MESSAGE_623_MODULE_VOLTAGE_SCALE_FACTOR (0xFFU / (MESSAGE_623_MODULE_VOLTAGE_MAX_VALUE))
#define MESSAGE_623_PACK_VOLTAGE_SCALE_FACTOR (0xFFFFU / (MESSAGE_623_PACK_VOLTAGE_MAX_VALUE))

/*============================================================================*/
/* PRIVATE FUNCTION PROTOTYPES */

void CAN_InitTxMessages(CAN_Tx_Message_t txMessages[NUM_CAN_MESSAGES_TRANSMIT]);
void CAN_Init_0x450_Filter(CAN_HandleTypeDef *hcan);

/*============================================================================*/
/* PUBLIC FUNCTION IMPLEMENTATIONS */

/**
 * @brief Initialize CAN headers and filters.
 * Call this function once before sending any CAN messages.
 *
 * @param message623 message 623 structure to populate
 * @param pack pack data structure that data will be read from
 */
void CAN_Init(CAN_HandleTypeDef *hcan, CAN_Tx_Message_t txMessages[NUM_CAN_MESSAGES_TRANSMIT])
{
    CAN_InitTxMessages(txMessages);
    CAN_Init_0x450_Filter(hcan);
    // any additional filter configuration functions should go here
}

/**
 * @brief Get data for message 622, populate appropriate message in message array.
 *
 * @param txMessages array of messages to transmit
 * @param pack pack data structure that data will be read from
 */
void CAN_CompileMessage622(CAN_Tx_Message_t txMessages[NUM_CAN_MESSAGES_TRANSMIT], Pack_t *pack)
{
    uint32_t status = pack->status.raw;
    CAN_Tx_Message_t *message622_p = &(txMessages[INDEX_622]);
    message622_p->data[0] = (uint8_t)status;         // bits 0-7
    message622_p->data[1] = (uint8_t)(status >> 8);  // 8-15
    message622_p->data[2] = (uint8_t)(status >> 16); // 16, 17 (garbage beyond this)
}

/**
 * @brief Get data for message 623, populate appropriate message in message array.
 *
 * @param txMessages array of messages to transmit
 * @param pack pack data structure that data will be read from
 */
void CAN_CompileMessage623(CAN_Tx_Message_t txMessages[NUM_CAN_MESSAGES_TRANSMIT], Pack_t *pack)
{
    uint16_t min_module_voltage = 0xFFFFU;
    uint16_t max_module_voltage = 0;
    uint16_t module_voltage;
    // locations of min and max voltage
    uint8_t min_module = 0;
    uint8_t max_module = 0;

    uint16_t total_pack_voltage = (uint16_t) ((Pack_GetPackVoltage(pack) * MESSAGE_623_PACK_VOLTAGE_SCALE_FACTOR) / PACK_MODULE_VOLTAGE_LSB_PER_V);

    CAN_Tx_Message_t *message623_p = &(txMessages[INDEX_623]);

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
    message623_p->data[0] = (uint8_t) (total_pack_voltage >> 8); // casting shifted 16 bit integer into 8 bit integer get rids of upper 8 bits, leaves lower 8 bits
    message623_p->data[1] = (uint8_t) total_pack_voltage;        // casting 16 bit integer into 8 bit integer gets rid of upper 8 bits, leaves lower 8 bits
    message623_p->data[2] = min_volt_rescaled;
    message623_p->data[3] = min_module; // add one because of indexing from zero
    message623_p->data[4] = max_volt_rescaled;
    message623_p->data[5] = max_module;
}

/**
 * @brief Get data for message 624, populate appropriate message in message array.
 *
 * @param txMessages array of messages to transmit
 * @param pack pack data structure that data will be read from
 */
void CAN_CompileMessage624(CAN_Tx_Message_t txMessages[NUM_CAN_MESSAGES_TRANSMIT], Pack_t *pack)
{
    CAN_Tx_Message_t *message624_p = &(txMessages[INDEX_624]);

    float soc = Pack_GetPackStateOfCharge(pack);
    float capacity = SOC_getCapacity(soc);
    float dod = SOC_getDOD(capacity);

    uint16_t capacity_encoded = (uint16_t) capacity;
    uint16_t dod_encoded = (uint16_t) dod;

    message624_p->data[0] = (uint8_t) soc;
    message624_p->data[1] = (uint8_t) (dod_encoded >> 8);
    message624_p->data[2] = (uint8_t) dod_encoded;
    message624_p->data[3] = (uint8_t) (capacity_encoded >> 8);
    message624_p->data[4] = (uint8_t) capacity_encoded;
}

/**
 * @brief Get data for message 625, populate message struct.
 *
 * @param message623 message structure that will be populated
 * @param pack pack data structure that data will be read from
 */
void CAN_CompileMessage625(CAN_Tx_Message_t txMessages[NUM_CAN_MESSAGES_TRANSMIT], Pack_t *pack)
{
    // Temperature measured in degrees celcius
    float sum = 0.0;
    float average_pack_temperature;
    float min_module_temperature = FLT_MIN;
    float max_module_temperature = FLT_MAX;
    float module_temperature;
    // indices of modules with min and max temperatures
    uint8_t min_module = 0;
    uint8_t max_module = 0;

    CAN_Tx_Message_t *message625_p = &(txMessages[INDEX_625]);

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
    message625_p->data[0] = (int8_t) average_pack_temperature;
    message625_p->data[1] = (int8_t) min_module_temperature;
    message625_p->data[2] = min_module;
    message625_p->data[3] = (int8_t) max_module_temperature;
    message625_p->data[4] = max_module;
}

/**
 * @brief Retrieves all messages pending in recieve FIFO0 and FIFO1.
 *
 * @param hcan CAN handle
 * @param rxMessages Messages of type CAN_Rx_Message_t recieved from FIFOs
 * @note Caller should check the value of the "message_status" field within each rx struct to see if a message was recieved or not
 */
void CAN_RecieveMessages(CAN_HandleTypeDef *hcan, CAN_Rx_Message_t rxMessages[NUM_RX_FIFOS * MAX_MESSAGES_PER_FIFO])
{
    CAN_Rx_Message_t *rx_msg_p;
    unsigned int fifo_fill_level;
    uint8_t rx_msg_index = 0;

    for (int fifo_num = 0; fifo_num < NUM_RX_FIFOS; fifo_num++)
    {
        fifo_fill_level = HAL_CAN_GetRxFifoFillLevel(hcan, fifo_num); // number of messages pending in fifo
        for (int message_num = 0; message_num < fifo_fill_level; message_num++)
        {
            rx_msg_p = &rxMessages[rx_msg_index];
            rx_msg_p->fifo = fifo_num;                                                               // indicates which fifo message was recieved in
            if (HAL_CAN_GetRxMessage(hcan, fifo_num, &(rx_msg_p->rx_header), rx_msg_p->data) != HAL_OK) // retrieve message
            {
                Error_Handler();
            }
            rx_msg_p->message_status = MSG_RECIEVED; // sucessfully recieved message
            rx_msg_index++;
        }
    }

    // if rx_msg_index < MAX_MESSAGES_PER_FIFO, not all FIFOs were full
    for (int i = rx_msg_index; i < (NUM_RX_FIFOS * MAX_MESSAGES_PER_FIFO); i++)
    {
        rxMessages[i].message_status = MSG_NOT_RECIEVED; // didn't recieve a message for struct at this index
    }
}

/*============================================================================*/
/* PRIVATE/HELPER FUNCTION IMPLEMENTATIONS */

/**
 * @brief Initilize header of each CAN message
 * Should be called once, before any CAN messages are sent
 *
 * @param txMessageArrray Array of relevant message information
 * Should be declared only once, before this function is called
 */
void CAN_InitTxMessages(CAN_Tx_Message_t txMessages[NUM_CAN_MESSAGES_TRANSMIT])
{
    for (int i = 0; i < NUM_CAN_MESSAGES_TRANSMIT; i++)
    {
        txMessages[i].tx_header.StdId = INITIAL_MESSAGE_INDEX + i; // hexadecimal
        txMessages[i].tx_header.ExtId = 0;                         // ext id is unused
        txMessages[i].tx_header.IDE = CAN_ID_STD;
        txMessages[i].tx_header.RTR = CAN_RTR_DATA; // sending data
        txMessages[i].tx_header.DLC = MAX_CAN_DATAFRAME_BYTES;
        txMessages[i].tx_header.TransmitGlobalTime = DISABLE;
    }
}

/**
 * @brief Configure recieve filters for message 0x450 (from the ECU)
 * Additional functions should be created to configure different filter banks
 * This function is SPECIFICALLY for the filter bank for message 0x450
 *
 * @param hcan CAN interface handle
 */
void CAN_Init_0x450_Filter(CAN_HandleTypeDef *hcan)
{
    /*
    Filter Information (see page 644 onward of stm32f103 reference manual)

        In ARM, CAN subsystem known as bxCAN (basic-extended)
        14 configurable filter banks (STM32F103C8 has only one CAN interface)
        Each filter bank consists of two, 32-bit registers, CAN_FxR0 and CAN_FxR1
        Depending on filter scale, filter bank provides one 32-bit filter for mask and id, or two 16 bit filters (each) for id

    Mask mode: Use mask to enable/disbale the bits of the filter you want to check the CAN ID against
    Identifier list mode: mask registers used as identifier registers (incoming ID must match exactly)

    For 32 bit filter, [31:21] map to STID [10:0], other bits we don't care about

    Good explanation on mask mode: https://www.microchip.com/forums/m456043.aspx
    What I followed for filter config: https://controllerstech.com/can-protocol-in-stm32/
    */

    CAN_FilterTypeDef filter_config;

    filter_config.FilterActivation = CAN_FILTER_ENABLE;            // enable filters
    filter_config.SlaveStartFilterBank = 14;                       // only one CAN interface, parameter meaningless (all filter banks for the one controller)
    filter_config.FilterBank = 0;                                  // settings applied for filterbank 0
    filter_config.FilterFIFOAssignment = CAN_FILTER_FIFO0;         // rx'd message will be placed into this FIFO
    filter_config.FilterMode = CAN_FILTERMODE_IDLIST;              // identifier list mode
    filter_config.FilterScale = CAN_FILTERSCALE_32BIT;             // don't need double layer of filters, not using EXTID, 32bit fine (if rx'ing many messages with diff ID's, could use double layer of filters)
    filter_config.FilterMaskIdHigh = ECU_CURRENT_MESSAGE_ID << 5; // ID upper 16 bits (not using mask), bit shift per bit order (see large comment above)
    filter_config.FilterMaskIdLow = 0;                             // ID lower 16 bits (not using mask)
    filter_config.FilterIdHigh = ECU_CURRENT_MESSAGE_ID << 5;     // filter ID upper 16 bits (list mode, mask = ID)
    filter_config.FilterIdLow = 0;                                 // filter ID lower 16 bits

    if (HAL_CAN_ConfigFilter(hcan, &filter_config) != HAL_OK) // configure filter registers
    {
        Error_Handler();
    }
}
