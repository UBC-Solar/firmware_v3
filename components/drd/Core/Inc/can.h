/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    can.h
  * @brief   This file contains all the function prototypes for
  *          the can.c file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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

/*
 *	CAN IDs
 */
#define CAN_ID_PACK_CURRENT		    0x450
#define CAN_ID_BATT_FAULTS		 	0x622
#define CAN_ID_PACK_VOLTAGE		    0x623
#define CAN_ID_MTR_FAULTS         	0x08A50225 // Todo: Change this later

#define CAN_ID_TURN_SIGNALS       	0x000 //Todo: Change this later

/* USER CODE END Private defines */

void MX_CAN_Init(void);

/* USER CODE BEGIN Prototypes */
void CAN_comms_Rx_callback(CAN_comms_Rx_msg_t* CAN_comms_Rx_msg);
void CAN_tasks_init();
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __CAN_H__ */

