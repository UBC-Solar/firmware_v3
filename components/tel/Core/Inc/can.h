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
#define RTC_TIMESTAMP_MSG_ID 0x751
#define TEL_DIAGNOSTICS_ID 0x750
#define IMU_X_AXIS 0x752
#define IMU_Y_AXIS 0x753
#define IMU_Z_AXIS 0x754
#define GPS_latitude_ID 0x755
#define GPS_longitude_ID 0x756
#define GPS_altitude_hdop_ID 0x757
#define GPS_side_count_ID 0x758
#define GET_BYTE_FROM_WORD(i, word) ((word >> (i * 8)) & 0xFF);

#define CAN_DATA_LENGTH 8

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


typedef union DoubleBytes {
	double double_value;			/**< Double value member of the union. */
	uint64_t double_as_int;			/**< 64 bit in member of union. */
} DoubleBytes;

typedef struct {
  CAN_RxHeaderTypeDef header;
  uint8_t data[8];
  union DoubleBytes timestamp;
} CAN_msg_t;

typedef struct {
  CAN_TxHeaderTypeDef header;
  uint8_t data[8];
  union DoubleBytes timestamp;
} CAN_Radio_msg_t;

void CanFilterSetup(void);
void Can_Init(void);

extern osThreadId readCANTaskHandle;

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __CAN_H__ */

