/**
 * @file can.c
 * @brief Header file for CAN communication functions for ECU
 * 
 * @date 2021/01/30
 * @author Blake Shular (blake-shular)
 */

#ifndef __CAN_H
#define __CAN_H

#include "stm32f1xx_hal.h"

#define CAN_TIMEOUT 50 // (ms)

CAN_HandleTypeDef *CAN_hcan;

void CAN_send_current(int currentVal);
HAL_StatusTypeDef HAL_CAN_AddTxMessage(CAN_HandleTypeDef *hcan, CAN_TxHeaderTypeDef *pHeader, uint8_t aData[], uint32_t *pTxMailbox);

#endif /* __CAN_H */
