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
#include "ecu.h"

#define CAN_TIMEOUT 50 // (ms)

/*============================================================================*/
/* FUNCTION PROTOTYPES */

CAN_HandleTypeDef *CAN_hcan;

HAL_StatusTypeDef HAL_CAN_AddTxMessage(CAN_HandleTypeDef *hcan, CAN_TxHeaderTypeDef *pHeader, uint8_t aData[], uint32_t *pTxMailbox);

void CAN_Init(CAN_HandleTypeDef *hcan);

void CAN_SendMessage450(ECU_t *ECU);
void CAN_SendMessage3F4(ECU_t *ECU);

//not a complete function just there as a template
bool CAN_GetMessage0x3E5DataData(int8_t *pack_current, uint8_t *low_voltage_current, bool *overcurrent_status, uint32_t *rx_timestamp);


#endif /* __CAN_H */
