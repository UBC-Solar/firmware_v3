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

/*============================================================================*/
/* DEFINITIONS */

#define CAN_TIMEOUT 50 // (ms)

#define MAX_CHARGING_CURRENT 137U  // Max outlet current is 15A, power bar breaker tripped at ~15A. 
#define MAX_CHARGING_VOLTAGE 1344U // 134.4 V 0.1V/bit offset

#define CAN_MAX_DATAFRAME_BYTES 8U
#define CHARGER_ENABLE_MESSAGE_ID 0x1806E5F4U
#define CHARGER_STATUS_MESSAGE_ID 0x18FF50E5U

#define MESSAGE_450_LVS_CURRENT_MAX_VALUE 30U   // 30A
#define MESSAGE_450_BATT_CURRENT_MAX_VALUE 100U // 100A

#define LVS_CURRENT_LSB_PER_V 1000U   // LVS current reading from ADC in mA
#define BATT_CURRENT_LSB_PER_V 10000U // BATT current reading from ADC in mA but must divide by extra 10 to fit into int16_t because it is signed

#define MESSAGE_450_LVS_CURRENT_SCALE_FACTOR (0xFFU / (MESSAGE_450_LVS_CURRENT_MAX_VALUE))
#define MESSAGE_450_BATT_CURRENT_SCALE_FACTOR (0xFFFFU / (MESSAGE_450_BATT_CURRENT_MAX_VALUE))

/*============================================================================*/
/* FUNCTION PROTOTYPES */

void CAN_Init(CAN_HandleTypeDef *hcan);
void CAN_SendMessage450();
void CAN_SendMessage1806E5F4();
void CAN_CheckRxMessages(uint32_t rx_fifo);
bool CAN_CheckRxChargerMessage();

#endif /* __CAN_H */
