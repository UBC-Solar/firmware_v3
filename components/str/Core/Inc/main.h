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
#define LTS_IN_Pin GPIO_PIN_0
#define LTS_IN_GPIO_Port GPIOA
#define RTS_IN_Pin GPIO_PIN_1
#define RTS_IN_GPIO_Port GPIOA
#define DEBUG_LED_Pin GPIO_PIN_6
#define DEBUG_LED_GPIO_Port GPIOA
#define MODE_PW_Pin GPIO_PIN_9
#define MODE_PW_GPIO_Port GPIOC

#define TICK_DELAY 1000

/* USER CODE BEGIN Private defines */
typedef enum {
  TS_OFF = 0b00,
  TS_RIGHT = 0b01,
  TS_LEFT = 0b10,
} turn_signal_status_t;

typedef enum {
  POWER_MODE = 0b00,
  ECO_MODE = 0b01,
} mode_status_t;

uint32_t last_time = 0;
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
