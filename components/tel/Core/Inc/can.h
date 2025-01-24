/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    can.h
  * @brief   This file contains all the function prototypes for
  *          the can.c file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CAN_H__
#define __CAN_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */
#include <stdint.h>
#include "cmsis_os.h"
#include "FreeRTOS.h"

#include "task.h"
#include "tel_freertos.h"
/* USER CODE END Includes */

extern CAN_HandleTypeDef hcan;

/* USER CODE BEGIN Private defines */
#define MAX_CAN_DATA_LENGTH                 8


/* USER CODE END Private defines */

void MX_CAN_Init(void);

/* USER CODE BEGIN Prototypes */

void CAN_filter_init();

typedef struct {
   uint32_t queue_dropped_rx_msg;
   uint32_t queue_dropped_tx_msg;
   uint8_t rx_queue_count;
   uint8_t tx_queue_count;
   uint32_t success_rx;
   uint32_t success_tx;
   uint32_t hal_failure_tx;
   uint32_t hal_failure_rx;

} CAN_diagnostics_t;

typedef struct
{
  CAN_RxHeaderTypeDef header;
  uint8_t data[MAX_CAN_DATA_LENGTH];
} CAN_Rx_msg_t;

typedef struct
{
	CAN_TxHeaderTypeDef header;
	uint8_t data[MAX_CAN_DATA_LENGTH];
} CAN_Tx_msg_t;

void CAN_Rx_callback(CAN_Rx_msg_t* CAN_Rx_msg);
void CAN_Init();


extern CAN_diagnostics_t can_diagnostic;
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __CAN_H__ */

