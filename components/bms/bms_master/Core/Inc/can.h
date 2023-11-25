/**
 * @file can.h
 * @brief Header file for CAN message management
 *
 * @date October 1, 2022
 * @author Edward Ma, Jung Yi Cau, Mischa Johal
 */

#ifndef INC_CAN_H_
#define INC_CAN_H_

#include <stdint.h>
#include <math.h>
#include <stdbool.h>
#include "stm32f1xx_hal.h"
#include "pack.h"
#include "ltc6813_btm.h"
#include "analysis.h"
#include "soc.h"

/*============================================================================*/
/* FUNCTION PROTOTYPES */

void CAN_Init(CAN_HandleTypeDef *hcan);

void CAN_SendMessage622(Pack_t *pack);
void CAN_SendMessage623(Pack_t *pack);
void CAN_SendMessage624(Pack_t *pack);
void CAN_SendMessage625(Pack_t *pack);
void CAN_SendMessages626(Pack_t *pack);
void CAN_SendMessages627(Pack_t *pack);
void CAN_SendMessages628(Pack_t *pack);
void CAN_SendMessages629(Pack_t *pack);

bool CAN_GetMessage0x450Data(int8_t *pack_current, uint8_t *low_voltage_current, bool *overcurrent_status, uint32_t *rx_timestamp);

void CAN_RecievedMessageCallback(void);
void CAN_TxCompleteCallback(void);
void CAN_ErrorCallback(void);

#endif // INC_CAN_H_
