/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    can.h
  * @brief   This file contains all the function prototypes for
  *          the can.c file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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

#include "cmsis_os.h"

/* USER CODE END Includes */

extern CAN_HandleTypeDef hcan;

/* USER CODE BEGIN Private defines */

#define CAN_READY (uint32_t) 0x0001

#define RTC_TIMESTAMP 0x751

#define CAN_DATA_LENGTH 8

extern CAN_TxHeaderTypeDef rtc_timestamp_header;
extern CAN_TxHeaderTypeDef tel_diagnostics_header;

extern uint32_t can_mailbox;

/* USER CODE END Private defines */

void MX_CAN_Init(void);

/* USER CODE BEGIN Prototypes */

typedef struct {
  CAN_RxHeaderTypeDef header;
  uint8_t data[8];
  double timestamp;
} CAN_msg_t;

void CanFilterSetup(void);
void Can_Init(void);

extern osThreadId readCANTaskHandle;

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __CAN_H__ */

