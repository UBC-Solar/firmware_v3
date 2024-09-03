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

#define CAN_READY (uint32_t) 0x0001
#define RTC_TIMESTAMP_MSG_ID 0x300
#define TEL_DIAGNOSTICS_ID 0x750

#define CAN_DATA_LENGTH 8
#define CAN_TIMESTAMP_LENGTH 8

/* INITIAL CONSTANTS */
#define INITIAL_FLAGS                       0x00
#define FLAG_HIGH                           1
#define FIRST_DATA_BYTE                     0

extern CAN_TxHeaderTypeDef tel_diagnostics_header;
extern CAN_TxHeaderTypeDef IMU_x_axis_header;
extern CAN_TxHeaderTypeDef IMU_y_axis_header;
extern CAN_TxHeaderTypeDef IMU_z_axis_header;
extern CAN_TxHeaderTypeDef GPS_latitude_header;
extern CAN_TxHeaderTypeDef GPS_longitude_header;
extern CAN_TxHeaderTypeDef GPS_altitude_hdop_header;
extern CAN_TxHeaderTypeDef GPS_side_count_header;
extern uint32_t can_mailbox;

/* USER CODE END Private defines */

void MX_CAN_Init(void);

/* USER CODE BEGIN Prototypes */

typedef struct {
    uint8_t data[MAX_CAN_DATA_LEN];
    uint32_t can_id;
    uint8_t data_len;
} CAN_FIFORxMsg_TypeDef;

void CanFilterSetup(void);
void CAN_Init(void);
void CAN_radio_and_bus_transmit(CAN_HandleTypeDef* hcan, CAN_Radio_msg_t* tx_CAN_msg, uint32_t* can_mailbox);
void CAN_rx_to_radio(CAN_msg_t* rx_CAN_msg, CAN_Radio_msg_t* tx_CAN_msg);
void CAN_diagnostic_msg_tx_radio_bus();

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __CAN_H__ */

