/**
 * @file can.c
 * @brief Header file for CAN communication functions for ECU
 * 
 * @date 2021/01/30
 * @author Blake Shular (blake-shular)
 */

#ifndef __CAN_H
#define __CAN_H

#include "main.h"
#include "common.h"

#define CAN_TIMEOUT 50 // (ms)

/*============================================================================*/
/* FUNCTION PROTOTYPES */

void CAN_Init(CAN_HandleTypeDef *hcan);
void CAN_SendMessage450();
void CAN_SendMessage3F4();

//not a complete function just there as a template
bool CAN_GetMessage0x3E5DataData(int8_t *pack_current, uint8_t *low_voltage_current, bool *overcurrent_status, uint32_t *rx_timestamp);


#endif /* __CAN_H */
