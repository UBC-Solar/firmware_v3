/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    adc.h
  * @brief   This file contains all the function prototypes for
  *          the adc.c file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#ifndef __ADC_H__
#define __ADC_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */
#include "common.h"
#include "gpio.h"
#include "can.h"
#include <stdio.h>
/* USER CODE END Includes */

extern ADC_HandleTypeDef hadc1;

/* USER CODE BEGIN Private defines */
#define DMA_BUFFER_START 0
#define DMA_BUFFER_END 7



// Macros for Brake Pressure conversion
#define BRAKE_PRESSURE_MULTIPLIER 24.55
#define BRAKE_PRESSURE_OFFSET 1.925

// Macros for Shock Travel conversion
#define SHOCK_TRAVEL_MULTIPLIER 0.1595
#define SHOCK_TRAVEL_OFFSET 2.3875

/* USER CODE END Private defines */

void MX_ADC1_Init(void);

/* USER CODE BEGIN Prototypes */

void ADC1_processRawReadings(int half, volatile uint16_t adc1_buf[], float result[]);
float getAveragedValue(volatile uint16_t array[], uint16_t length);

int ADC1_getBusyStatus();
void ADC1_setBusyStatus(int flag_value);

int ADC1_getFaultStatus(); 
void ADC1_setFaultStatus(int flag_value);

int ADC_getReading(int adc_channel);
void ADC_setReading(float adc_reading, uint8_t adc_channel);

void ADC_convertRawValues(void);
void ADC_Select(uint8_t channel);
void read_all_sensors(void);
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __ADC_H__ */

