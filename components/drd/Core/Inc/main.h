/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "CAN_comms.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define DTR_IN_Pin GPIO_PIN_15
#define DTR_IN_GPIO_Port GPIOC
#define BATT_UV_Pin GPIO_PIN_0
#define BATT_UV_GPIO_Port GPIOC
#define BATT_HI_Pin GPIO_PIN_1
#define BATT_HI_GPIO_Port GPIOC
#define BATT_FLT_Pin GPIO_PIN_2
#define BATT_FLT_GPIO_Port GPIOC
#define BATT_OT_Pin GPIO_PIN_3
#define BATT_OT_GPIO_Port GPIOC
#define BATT_OV_Pin GPIO_PIN_1
#define BATT_OV_GPIO_Port GPIOA
#define ACCEL_2_Pin GPIO_PIN_2
#define ACCEL_2_GPIO_Port GPIOA
#define ACCEL_1_Pin GPIO_PIN_3
#define ACCEL_1_GPIO_Port GPIOA
#define SPI1_CS_Pin GPIO_PIN_4
#define SPI1_CS_GPIO_Port GPIOA
#define DISPLAY_A0_Pin GPIO_PIN_6
#define DISPLAY_A0_GPIO_Port GPIOA
#define DCH_OC_Pin GPIO_PIN_4
#define DCH_OC_GPIO_Port GPIOC
#define CH_OC_Pin GPIO_PIN_5
#define CH_OC_GPIO_Port GPIOC
#define BMS_COMM_FLT_Pin GPIO_PIN_0
#define BMS_COMM_FLT_GPIO_Port GPIOB
#define REGEN_EN_Pin GPIO_PIN_1
#define REGEN_EN_GPIO_Port GPIOB
#define DISPLAY_RESET_Pin GPIO_PIN_10
#define DISPLAY_RESET_GPIO_Port GPIOB
#define HAZARD_Pin GPIO_PIN_11
#define HAZARD_GPIO_Port GPIOB
#define BRK_IN_Pin GPIO_PIN_12
#define BRK_IN_GPIO_Port GPIOB
#define DTR_OUT_Pin GPIO_PIN_13
#define DTR_OUT_GPIO_Port GPIOB
#define RTS_OUT_Pin GPIO_PIN_14
#define RTS_OUT_GPIO_Port GPIOB
#define LTS_OUT_Pin GPIO_PIN_15
#define LTS_OUT_GPIO_Port GPIOB
#define MTR_FLT_Pin GPIO_PIN_6
#define MTR_FLT_GPIO_Port GPIOC
#define MTR_OC_Pin GPIO_PIN_7
#define MTR_OC_GPIO_Port GPIOC
#define MTR_OT_Pin GPIO_PIN_8
#define MTR_OT_GPIO_Port GPIOC
#define ESTOP_Pin GPIO_PIN_9
#define ESTOP_GPIO_Port GPIOC
#define PARK_EN_Pin GPIO_PIN_8
#define PARK_EN_GPIO_Port GPIOA
#define FWRD_EN_Pin GPIO_PIN_9
#define FWRD_EN_GPIO_Port GPIOA
#define RVRS_EN_Pin GPIO_PIN_10
#define RVRS_EN_GPIO_Port GPIOA
#define DEBUG_LED_1_Pin GPIO_PIN_10
#define DEBUG_LED_1_GPIO_Port GPIOC
#define DEBUG_LED_2_Pin GPIO_PIN_11
#define DEBUG_LED_2_GPIO_Port GPIOC

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
