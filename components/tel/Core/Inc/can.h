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
#include "radio.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern CAN_HandleTypeDef hcan;

/* USER CODE BEGIN Private defines */
typedef struct {
    RADIO_CANMsg_TypeDef can_radio_msg;
    bool is_sent;
} CAN_QueueMsg_TypeDef;

#define RTC_TIMESTAMP_MSG_ID 0x300
#define TEL_DIAGNOSTICS_ID 0x750

#define CAN_DATA_LENGTH 8
#define CAN_TIMESTAMP_LENGTH 8

/* INITIAL CONSTANTS */
#define INITIAL_FLAGS                       0x00
#define FLAG_HIGH                           1
#define FIRST_DATA_BYTE                     0
#define MAX_RX_QUEUE_SIZE                   60
#define START_OF_ARRAY                      0
#define CIRCULAR_INCREMENT_SET(index, max_size) (((index) + 1) % (max_size))

extern uint8_t g_rx_queue_index;
extern CAN_QueueMsg_TypeDef g_rx_queue[MAX_RX_QUEUE_SIZE];


/* USER CODE END Private defines */

void MX_CAN_Init(void);

/* USER CODE BEGIN Prototypes */
void CanFilterSetup(void);
void CAN_Init(void);


/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __CAN_H__ */

