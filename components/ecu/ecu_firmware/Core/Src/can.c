/**
 * @file can.c
 * @brief CAN communication functions for ECU
 * @date 2021/01/30
 * @author Blake Shular (blake-shular)
 * @author Tigran Hakobyan (Tik-Hakobyan)
 */

#include "can.h"
#include "main.h"

#define CAN_ID 0x450
#define MESSAGE_LENGTH 3

CAN_TxHeaderTypeDef current_message_header = {
    CAN_ID, 0, CAN_ID_STD, CAN_RTR_DATA, MESSAGE_LENGTH, DISABLE
}; 

/**
 * @brief Transmit ECU current reading CAN message
 * @param currentVal    The net current charging or discharging the battery pack on the high voltage bus, in 100ths of Amps.
                        The range is ±75A.                       
 * @param doc_coc       Discharge and Charge overcurrent status.
 */
void CAN_send_current(int currentVal, int doc_coc) {
    uint32_t begin_tick = HAL_GetTick();
    uint32_t * pTxMailbox = 0;
    HAL_StatusTypeDef status;
    currentVal = currentVal * 127 / 7500;     //Scaling factor (1A/100)*(127/75A) = 127/7500     

    uint8_t data[] = {(uint8_t)currentVal, 0, (uint8_t) doc_coc};
    do {
        status = HAL_CAN_AddTxMessage(CAN_hcan, &current_message_header, data, pTxMailbox);
    } while (status != HAL_OK && HAL_GetTick() - begin_tick <= CAN_TIMEOUT);
}

