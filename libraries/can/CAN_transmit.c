/**
 *  @file CAN_transmit.c
 *  @brief Defines functions to send CAN messages
 *
 *  @date 2024/18/06
 *  @author Aarjav Jain
 */


#include "CAN_transmit.h"


/**
 * @brief Sends a CAN message and uses provided Error handler if an error occurs
 * @param tx_header: CAN_TxHeaderTypeDef pointer to the CAN message header
 * @param tx_data: uint8_t array to the CAN message data
 * @param can_handle: CAN_HandleTypeDef pointer to the CAN handle
 * @param mailbox: uint32_t pointer to the mailbox that will be set to the mailbox used to send the message
 * @param Error_Handler: void function pointer to the error handler of your choice
 * @return void
 */
void CANTX_send(CAN_TxHeaderTypeDef* tx_header, uint8_t tx_data[], CAN_HandleTypeDef *can_handle, uint32_t* mailbox, void (*Error_Handler)()) {
    if (HAL_CAN_GetTxMailboxesFreeLevel(can_handle) == 0)
    {
        return;
    }

    if (HAL_OK != HAL_CAN_AddTxMessage(can_handle, tx_header, tx_data, mailbox))
    {
        Error_Handler();
    }
}
