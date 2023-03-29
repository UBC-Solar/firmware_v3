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

#define DRIVER_CONTROLS_BASE_ADDRESS 0x400
#define MOTOR_CTRL_BASE 0x500
#define BATTERY_BASE 0x620

#define CAN_DATA_LENGTH 8 			// incoming data is 64 bytes total
#define CAN_HALF_DATA_LENGTH 4 		// some IDs split the data into 2 values of 32 byte (e.g., 0x50B)
#define CAN_CONTROL_DATA_LENGTH 2

extern CAN_TxHeaderTypeDef drive_command_header;
extern CAN_TxHeaderTypeDef screen_cruise_control_header;
extern CAN_RxHeaderTypeDef can_rx_header;
extern CAN_FilterTypeDef mcb_filter;
extern uint32_t can_mailbox;

/* USER CODE END Private defines */

void MX_CAN_Init(void);

/* USER CODE BEGIN Prototypes */

void CAN_Filter_Init(void);

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __CAN_H__ */

