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

/* USER CODE END Includes */

extern CAN_HandleTypeDef hcan;

/* USER CODE BEGIN Private defines */

#define DRIVER_INTERFACE_ADDRESS     0x400
#define MOTOR_DRIVE_COMMAND_ADDRESS  0x401
#define GET_VELOCITY_HEADER			 0x503
#define BATTERY_MESSAGE_HEADER		 0x622

#define CAN_ID_VELOCITY				 0x503
#define CAN_ID_BATTERY_TEMP			 0x622
#define CAN_ID_BATTERY_SOC			 0x626

#define CAN_DATA_LENGTH 8

extern CAN_TxHeaderTypeDef drive_command_header;
extern CAN_TxHeaderTypeDef DID_next_page_header;
extern uint32_t can_mailbox;
extern CAN_RxHeaderTypeDef can_rx_header;
/* USER CODE END Private defines */

void MX_CAN_Init(void);

/* USER CODE BEGIN Prototypes */

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __CAN_H__ */

