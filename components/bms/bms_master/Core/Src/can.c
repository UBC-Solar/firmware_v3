/**
 *  @file
 *  @brief
 *
 *  @date
 *  @author
 */

#include "can.h"

/*============================================================================*/
/* PRIVATE FUNCTION PROTOTYPES */

void CAN_InitTxMessages(CAN_Tx_Messages_t txMessages[NUM_CAN_MESSAGES]);
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

void CAN_Init(CAN_HandleTypeDef *hcan, CAN_Tx_Message_t txMessages[NUM_CAN_MESSAGES])
{
    CAN_InitTxMessages(txMessages);
    CAN_Init_0x450_Filter(hcan);
    // any additional filter configuration functions should go here
}

/**
 * @brief Get data for message 623, populate message struct.
 *
 * @param message623 message 623 structure to populate
 * @param pack pack data structure that data will be read from
 */
void CAN_CompileMessage623(CAN_Tx_Message_t message623, BTM_PackData_t *pack)
{
    uint16_t total_pack_voltage = pack->pack_voltage;
    uint16_t min_module_voltage = 65535;
    uint16_t max_module_voltage = 0;
    uint16_t local_module_voltage;

    // locations of min and max voltage
    uint8_t min_module = 0;
    uint8_t max_module = 0;
    uint8_t min_stack = 0;
    uint8_t max_stack = 0;

    uint8_t rescaled_factor = 51; // (uint8_t bit size in decimal: 255)/(Max physical voltage per module: 5)

    for (int ic_num = 0; ic_num < BTM_NUM_DEVICES; ic_num++)
    {
        for (int module_num = 0; module_num < BTM_NUM_MODULES; module_num++)
        {
            if (pack->stack[ic_num].module[module_num].enable)
            {
                local_module_voltage = (pack->stack[ic_num].module[module_num].voltage);
                // check and store minimum voltage
                if (local_module_voltage < min_module_voltage)
                {
                    min_module_voltage = local_module_voltage; // store minimum voltage
                    min_stack = ic_num;                        // store stack with minimum voltage
                    min_module = module_num;                   // store module with minimum voltage
                }
                // check and store maximum voltage
                if (local_module_voltage > max_module_voltage)
                {
                    max_module_voltage = local_module_voltage;
                    max_module = module_num;
                    max_stack = ic_num;
                }
            }
        }
    }

    uint32_t min_volt_rescaled = (uint8_t)(min_module_voltage * rescaled_factor / 10000); // rescale max voltage of fit 8 bits of data
    uint32_t max_volt_rescaled = (uint8_t)(max_module_voltage * rescaled_factor / 10000); // 10000: conversion from 10^4mV to V

    message623.data[0] = (uint8_t)total_pack_voltage;        // casting 16 bit integer into 8 bit integer get rids of upper 8 bits, leaves lower 8 bits:[0]
    message623.data[1] = (uint8_t)(total_pack_voltage >> 8); // casting shifted 16 bit integer into 8 bit integer get rids of upper 8 bits, leaves lower 8 bits
    // TODO: need to deal with stack and module indices (LUT)
    // Store in message 623 data array
    message623.data[2] = min_volt_rescaled;
    message623.data[3] = min_module;
    message623.data[4] = max_volt_rescaled;
    message623.data[5] = max_module;
}

/**
 * @brief Get data for message 627, populate message struct.
 *
 * @param message623 message 627 structure to populate
 * @param pack pack data structure that data will be read from
 */
void CAN_CompileMessage627(CAN_Tx_Message_t message627, BTM_PackData_t *pack);
{
    float avg_pack_temp = 0; // Measured in Celsius
    float min_module_temp = 200;
    float max_module_temp = -200;
    float sum = 0;

    // locations of min and max temperature
    uint8_t min_module = 0;
    uint8_t max_module = 0;
    uint8_t min_stack = 0;
    uint8_t max_stack = 0;

    for (int ic_num = 0; ic_num < BTM_NUM_DEVICES; ic_num++)
    {
        for (int module_num = 0; module_num < BTM_NUM_MODULES; module_num++)
        {
            if (pack->stack[ic_num].module[module_num].enable)
            {
                local_module_temp = (pack->stack[ic_num].module[module_num].temperature);

                sum += local_module_temp; // Get sum of temperature

                // check and store minimum temperature
                if (local_module_temp < min_module_temp)
                {
                    min_module_temp = local_module_temp; // store minimum temperature
                    min_stack = ic_num;                  // store stack with minimum temperature
                    min_module = module_num;             // store module with minimum temperature
                }
                // check and store maximum temperature
                if (local_module_temp > max_module_temp)
                {
                    max_module_temp = local_module_temp;
                    max_module = module_num;
                    max_stack = ic_num;
                }
            }
        }
    }

    avg_pack_temp = sum / TOTAL_MODULES_ENABLED;

    // Populate data array
    // TODO: need to deal with stack and module indices (LUT)
    // message627.data[0] = avg_pack_temp;
    message627.data[1] = 0;
    // message627.data[2] = min_module_temp;
    message627.data[3] = min_module; // which stack?
    // message627.data[4] = max_module_temp;
    message627.data[5] = max_module; // which stack?
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
    CAN_Rx_Message_t *rx_msg;
    unsigned int fifo_fill_level;
    uint8_t rx_msg_index = 0;

    for (int fifo_num = 0; fifo_num < NUM_RX_FIFOS; fifo_num++)
    {
        fifo_fill_level = HAL_CAN_GetRxFifoFillLevel(hcan, fifo_num); // number of messages pending in fifo
        for (int message_num = 0; message_num < fifo_fill_level; message_num++)
        {
            rx_msg = &rxMessages[rx_msg_index]; // get address of local rx message struct
            rx_msg->fifo = fifo_num;
            if (HAL_CAN_GetRxMessage(hcan, fifo_num, rx_msg->rx_header, rx_msg->data) != HAL_OK) // retrieve message
            {
                Error_Handler();
            }
            rx_msg->message_status = MSG_RECIEVED; // sucessfully recieved message
            rx_msg_index++;
        }
    }

    // if rx_msg_index < MAX_MESSAGES_PER_FIFO, not all FIFOs were full
    for (int i = rx_msg_index; i < MAX_MESSAGES_PER_FIFO; i++)
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
void CAN_InitTxMessages(CAN_Tx_Message_t txMessages[NUM_CAN_MESSAGES]);
{
    for (int i = 0; i < NUM_CAN_MESSAGES; i++)
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
    filter_config.FilterMaskIdHigh = ECU_CURRENT_MSG_ID_MASK << 5; // ID upper 16 bits (not using mask), bit shift per bit order (see large comment above)
    filter_config.FilterMaskIdLow = 0;                             // ID lower 16 bits (not using mask)
    filter_config.FilterIdHigh = ECU_CURRENT_MSG_ID_MASK << 5;     // filter ID upper 16 bits (list mode, mask = ID)
    filter_config.FilterIdLow = 0;                                 // filter ID lower 16 bits

    if (HAL_CAN_ConfigFilter(hcan, &filter_config) != HAL_OK) // configure filter registers
    {
        Error_Handler();
    }
}