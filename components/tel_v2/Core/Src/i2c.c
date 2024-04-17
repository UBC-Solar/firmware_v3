/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    i2c.c
  * @brief   This file provides code for the configuration
  *          of the I2C instances.
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
/* Includes ------------------------------------------------------------------*/
#include "i2c.h"

/* USER CODE BEGIN 0 */


/* USER CODE END 0 */

I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef hi2c2;

/* I2C1 init function */
void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}
/* I2C2 init function */
void MX_I2C2_Init(void)
{

  /* USER CODE BEGIN I2C2_Init 0 */

  /* USER CODE END I2C2_Init 0 */

  /* USER CODE BEGIN I2C2_Init 1 */

  /* USER CODE END I2C2_Init 1 */
  hi2c2.Instance = I2C2;
  hi2c2.Init.ClockSpeed = 100000;
  hi2c2.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C2_Init 2 */

  /* USER CODE END I2C2_Init 2 */

}

void HAL_I2C_MspInit(I2C_HandleTypeDef* i2cHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(i2cHandle->Instance==I2C1)
  {
  /* USER CODE BEGIN I2C1_MspInit 0 */

  /* USER CODE END I2C1_MspInit 0 */

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**I2C1 GPIO Configuration
    PB6     ------> I2C1_SCL
    PB7     ------> I2C1_SDA
    */
    GPIO_InitStruct.Pin = GPS_SCL_Pin|GPS_SDA_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* I2C1 clock enable */
    __HAL_RCC_I2C1_CLK_ENABLE();
  /* USER CODE BEGIN I2C1_MspInit 1 */

  /* USER CODE END I2C1_MspInit 1 */
  }
  else if(i2cHandle->Instance==I2C2)
  {
  /* USER CODE BEGIN I2C2_MspInit 0 */

  /* USER CODE END I2C2_MspInit 0 */

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**I2C2 GPIO Configuration
    PB10     ------> I2C2_SCL
    PB11     ------> I2C2_SDA
    */
    GPIO_InitStruct.Pin = IMU_SCL_Pin|IMU_SDA_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* I2C2 clock enable */
    __HAL_RCC_I2C2_CLK_ENABLE();
  /* USER CODE BEGIN I2C2_MspInit 1 */

  /* USER CODE END I2C2_MspInit 1 */
  }
}

void HAL_I2C_MspDeInit(I2C_HandleTypeDef* i2cHandle)
{

  if(i2cHandle->Instance==I2C1)
  {
  /* USER CODE BEGIN I2C1_MspDeInit 0 */

  /* USER CODE END I2C1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_I2C1_CLK_DISABLE();

    /**I2C1 GPIO Configuration
    PB6     ------> I2C1_SCL
    PB7     ------> I2C1_SDA
    */
    HAL_GPIO_DeInit(GPS_SCL_GPIO_Port, GPS_SCL_Pin);

    HAL_GPIO_DeInit(GPS_SDA_GPIO_Port, GPS_SDA_Pin);

  /* USER CODE BEGIN I2C1_MspDeInit 1 */

  /* USER CODE END I2C1_MspDeInit 1 */
  }
  else if(i2cHandle->Instance==I2C2)
  {
  /* USER CODE BEGIN I2C2_MspDeInit 0 */

  /* USER CODE END I2C2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_I2C2_CLK_DISABLE();

    /**I2C2 GPIO Configuration
    PB10     ------> I2C2_SCL
    PB11     ------> I2C2_SDA
    */
    HAL_GPIO_DeInit(IMU_SCL_GPIO_Port, IMU_SCL_Pin);

    HAL_GPIO_DeInit(IMU_SDA_GPIO_Port, IMU_SDA_Pin);

  /* USER CODE BEGIN I2C2_MspDeInit 1 */

  /* USER CODE END I2C2_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */


/* IMU -----------------------------------------------------------------------*/



/*
 * Documentation: https://controllerstech.com/how-to-interface-mpu6050-gy-521-with-stm32/
 */
void initIMU(void)
{
  uint8_t data;

  printf("Initializing IMU...\n\r");

  /*
   * We need to check if the sensor is responding by reading the “WHO_AM_I (0x75)�? Register.
   * If the sensor responds with 0x68, this means it’s available and good to go.
   */
  for(int i = 0; i < 5; i++) {
    if(HAL_I2C_IsDeviceReady(&hi2c2, IMU_DEVICE_ADDRESS, 1, HAL_MAX_DELAY) == HAL_OK) {
	    HAL_I2C_Mem_Read (&hi2c2, IMU_DEVICE_ADDRESS, WHO_AM_I_REG, 1, &data, 1, 1000);
	    printf("Read a value from WHOAMI register: %x\n\r", data);
	    break; // Break when initialized
    }
  }

  /*
   * Next we will wake the sensor up and in order to do that we will write to the
   * “PWR_MGMT_1 (0x6B)�? Register. See below the register content.
   * On writing (0x00) to the PWR_MGMT_1 Register, sensor wakes up and the Clock sets up to 8 MHz.
   */
  data = 0;
  HAL_I2C_Mem_Write(&hi2c2, IMU_DEVICE_ADDRESS, PWR_MGMT_1_REG, 1, &data, 1, 1000);

  /*
   * Now we have to set the Data output Rate or Sample Rate. This can be done by writing into
   * “SMPLRT_DIV (0x19)�? Register. This register specifies the divider from the gyroscope output
   * rate used to generate the Sample Rate for the MPU6050.
   * As the formula says Sample Rate = Gyroscope Output Rate / (1 + SMPLRT_DIV).
   * Where Gyroscope Output Rate is 8KHz, To get the sample rate of 1KHz,
   * we need to use the SMPLRT_DIV as ‘7’.
   */
  data = 0x07;
  HAL_I2C_Mem_Write(&hi2c2, IMU_DEVICE_ADDRESS, SMPLRT_DIV_REG, 1, &data, 1, 1000);

  /*
   * Now configure the Accelerometer and Gyroscope registers and to do so, we need to modify
   * “GYRO_CONFIG (0x1B)�? and “ACCEL_CONFIG (0x1C)�?Registers.
   * Writing (0x00) to both of these registers would set the Full scale range of ± 2g in ACCEL_CONFIG
   * Register and a Full scale range of ± 250 °/s in GYRO_CONFIG Register along with Self-test disabled.
   */
  data = 0x00;
  HAL_I2C_Mem_Write(&hi2c2, IMU_DEVICE_ADDRESS, GYRO_CONFIG_REG, 1, &data, 1, 1000);
  data = 0x00;
  HAL_I2C_Mem_Write(&hi2c2, IMU_DEVICE_ADDRESS, ACCEL_CONFIG_REG, 1, &data, 1, 1000);

  /*
   * This completes the initialization of the MPU6050 and Now we will see How to Read the Data from the
   * sensor and how to convert it in the respective formats. Let us start with the Acceleration values first.
   */



  printf("IMU Initialized\n\r");


}


/* USER CODE END 1 */
