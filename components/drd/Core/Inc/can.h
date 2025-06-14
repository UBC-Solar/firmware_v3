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
extern const CAN_TxHeaderTypeDef drive_command_header;
extern const CAN_TxHeaderTypeDef mdu_request_header;
extern const CAN_TxHeaderTypeDef drd_diagnostic_header;
extern const CAN_TxHeaderTypeDef time_since_bootup_can_header;

/*
 *	CAN IDs
 */

#define MOTOR_DRIVE_COMMAND_ADDRESS 		0x401
#define DRD_DIAGNOSTIC_MESSAGE				0x403
#define CAN_ID_PACK_CURRENT		    		0x450
#define CAN_ID_BATT_FAULTS		 			0x622
#define CAN_ID_PACK_VOLTAGE		    		0x623
#define CAN_ID_PACK_HEALTH          		0x624
#define CAN_ID_MTR_FAULTS         			0x08A50225
#define CAN_ID_MTR_DATA_REQUEST     		0x08F89540 //DRD sends data request to the MCU
#define STR_CAN_MSG_ID						0x580
#define MDU_REQUEST_COMMAND_ID 				0x08F89540
#define FRAME0 								0x08850225
#define TIME_SINCE_BOOTUP_CAN_ID			0x404

#define MDU_REQUEST_FRAME					0b111 //request frames 0, 1 and 2 (0th, 1st and 2nd bit)
#define MDU_REQUEST_SIZE 					1
#define DRD_DIAGNOSTIC_SIZE					8
#define TIME_SINCE_BOOTUP_CAN_DATA_LENGTH 	4

/* USER CODE END Private defines */

void MX_CAN_Init(void);

/* USER CODE BEGIN Prototypes */
void CAN_comms_Rx_callback(CAN_comms_Rx_msg_t* CAN_comms_Rx_msg);
void CAN_tasks_init();
void CAN_filter_init(CAN_FilterTypeDef* can_filter);
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __CAN_H__ */

