/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    can.h
  * @brief   This file contains all the function prototypes for
  *          the can.c file
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
#ifndef __CAN_H__
#define __CAN_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */
#include "common.h"
/* USER CODE END Includes */

extern CAN_HandleTypeDef hcan1;

extern CAN_HandleTypeDef hcan2;

/* USER CODE BEGIN Private defines */

#define CAN_ID_SHOCK_TRAVEL 		0X710
#define CAN_ID_BRAKE_AND_STEERING 	0X711
#define CAN_ID_VDS_DIAGNOSTIC 		0X712

#define CAN_DATA_LENGTH 			8

// For Brake Pressure conversion
#define BRAKE_PRESSURE_MULTIPLIER 24.55
#define BRAKE_PRESSURE_OFFSET 1.925
#define BRAKE_PRESSURE_SURFACE_AREA 0.00023716
#define PSI_CONVERSION_FACTOR 0.0001453077
#define ADC_TO_VOLTAGE_DIVISOR 4095
#define ADC_TO_VOLTAGE_MULTIPLIER 3.3
#define MV_TO_V_MULTIPLIER 1000

// For Shock Travel conversion
#define SHOCK_TRAVEL_MULTIPLIER 69.93333
#define SHOCK_TRAVEL_OFFSET 15.66667

typedef struct {
    CAN_TxHeaderTypeDef header;
    uint8_t data[8];
} CAN_Message_t;

extern CAN_TxHeaderTypeDef shock_travel_header;
extern CAN_TxHeaderTypeDef brake_and_steering_header;
extern CAN_TxHeaderTypeDef vds_diagnostic_header;


/* USER CODE END Private defines */

void MX_CAN1_Init(void);
void MX_CAN2_Init(void);

/* USER CODE BEGIN Prototypes */
void CAN_SendShockTravel(VDS_Data_t *adc_data);
void CAN_SendBrakeAndSteering(VDS_Data_t *adc_data);
void CAN_SendDiagnostics(VDS_StatusCode_t *status);
void CAN_processMessages(void);
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __CAN_H__ */

