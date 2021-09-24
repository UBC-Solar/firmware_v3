/**
  ******************************************************************************
  * @file    can.h
  * @brief   This file contains all the function prototypes for
  *          the can.c file
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
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

#define CAN_DATA_LENGTH 8
#define CAN_CONTROL_DATA_LENGTH 2

extern CAN_TxHeaderTypeDef drive_command_header;
extern CAN_TxHeaderTypeDef screen_cruise_control_header;
extern CAN_RxHeaderTypeDef can_rx_header;
extern CAN_FilterTypeDef battery_soc_filter;
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
