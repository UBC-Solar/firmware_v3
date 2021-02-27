/**
 * @file can.c
 * @brief CAN communication functions for ECU
 * 
 * @date 2021/01/30
 * @author 
 */

#include "can.h"

CAN_TxHeaderTypeDef current_message_header = {
    0x624, 0, CAN_ID_STD, CAN_RTR_DATA, 6, DISABLE
}; 

void CAN_send_current(int currentVal) {
    uint32_t begin_tick = HAL_GetTick();
    uint32_t * pTxMailbox;
    HAL_StatusTypeDef status;
    currentVal /= 100;
    uint8_t data[] = {(uint8_t)currentVal, (uint8_t)(currentVal >> 8), 0, 0, 0, 0};
    do {
        HAL_CAN_AddTxMessage(CAN_hcan, &current_message_header, data, pTxMailbox);
    } while (status != HAL_OK && HAL_GetTick() - begin_tick <= CAN_TIMEOUT);
}
