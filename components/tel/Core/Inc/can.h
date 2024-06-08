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
#include "utils.h"

/* USER CODE END Includes */

extern CAN_HandleTypeDef hcan;

/* USER CODE BEGIN Private defines */

#define CAN_READY (uint32_t) 0x0001
#define RTC_TIMESTAMP_MSG_ID 0x300
#define TEL_DIAGNOSTICS_ID 0x750
#define IMU_X_AXIS 0x752
#define IMU_Y_AXIS 0x753
#define IMU_Z_AXIS 0x754
#define GPS_latitude_ID 0x755
#define GPS_longitude_ID 0x756
#define GPS_altitude_hdop_ID 0x757
#define GPS_side_count_ID 0x758

#define CAN_DATA_LENGTH 8
#define CAN_TIMESTAMP_LENGTH 8

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
  CAN_RxHeaderTypeDef header;
  uint8_t data[8];
  union Utils_DoubleBytes_t timestamp;
} CAN_msg_t;

typedef struct {
  CAN_TxHeaderTypeDef header;
  uint8_t data[8];
  union Utils_DoubleBytes_t timestamp;
} CAN_Radio_msg_t;

void CanFilterSetup(void);
void CAN_Init(void);
void CAN_radio_and_bus_transmit(CAN_HandleTypeDef* hcan, CAN_Radio_msg_t* tx_CAN_msg, uint32_t* can_mailbox);
void CAN_rx_to_radio(CAN_msg_t* rx_CAN_msg, CAN_Radio_msg_t* tx_CAN_msg);

extern osThreadId readCANTaskHandle;

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __CAN_H__ */

