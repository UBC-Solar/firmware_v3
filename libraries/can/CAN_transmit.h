/**
 *  @file CAN_transmit.h
 *  @brief header file for CAN_transmit.c. 
 *
 *  @date 2024/18/06
 *  @author Aarjav Jain
 */

#ifndef __CAN_TRANSMIT_H__
#define __CAN_TRANSMIT_H__

#include <stdint.h>
#include "stm32f1xx_hal.h"

void CANTX_send(CAN_TxHeaderTypeDef* tx_header, uint8_t tx_data[], CAN_HandleTypeDef *can_handle, uint32_t* mailbox, void (*Error_Handler)());

#endif // CAN_TRANSMIT_H
