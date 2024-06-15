/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    i2c.h
  * @brief   This file contains all the function prototypes for
  *          the i2c.c file
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
#ifndef __I2C_H__
#define __I2C_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern I2C_HandleTypeDef hi2c1;

extern I2C_HandleTypeDef hi2c2;

/* USER CODE BEGIN Private defines */

#define IMU_DEVICE_ADDRESS ((0x68)<<1)
#define GPS_DEVICE_ADDRESS ((0x42)<<1)

/* IMU REGISTERS */
#define WHO_AM_I_REG 0x75
#define PWR_MGMT_1_REG 0x6B
#define SMPLRT_DIV_REG 0x19
#define GYRO_CONFIG_REG 0x1B
#define ACCEL_CONFIG_REG 0x1C

#define ACCEL_XOUT_H_REG 0x3B
#define ACCEL_XOUT_L_REG 0x3C
#define ACCEL_YOUT_H_REG 0x3D
#define ACCEL_YOUT_L_REG 0x3E
#define ACCEL_ZOUT_H_REG 0x3F
#define ACCEL_ZOUT_L_REG 0x40

#define GYRO_XOUT_H_REG 0x43
#define GYRO_XOUT_L_REG 0x44
#define GYRO_YOUT_H_REG 0x45
#define GYRO_YOUT_L_REG 0x46
#define GYRO_ZOUT_H_REG 0x47
#define GYRO_ZOUT_L_REG 0x48

#define NUM_ACCEL_BYTES 6
#define NUM_GYRO_BYTES 6

#define READ_TIMEOUT 1000
#define GPS_TRIALS    1


/* USER CODE END Private defines */

void MX_I2C1_Init(void);
void MX_I2C2_Init(void);

/* USER CODE BEGIN Prototypes */

/* IMU -----------------------------------------------------------------------*/

enum GyroType {
    GYRO_X,
    GYRO_Y,
    GYRO_Z
};

enum AccelType {
    ACCEL_X,
    ACCEL_Y,
    ACCEL_Z
};





void initIMU(void);

/* GPS -----------------------------------------------------------------------*/
typedef struct{
  uint8_t data[150];
} GPS_msg_t;


/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __I2C_H__ */

