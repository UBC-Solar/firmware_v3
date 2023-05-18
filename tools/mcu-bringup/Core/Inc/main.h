/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
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

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define GPIO_OUT_Pin GPIO_PIN_4
#define GPIO_OUT_GPIO_Port GPIOA
#define OT_OUT_Pin GPIO_PIN_5
#define OT_OUT_GPIO_Port GPIOA
#define BAL_OUT_Pin GPIO_PIN_6
#define BAL_OUT_GPIO_Port GPIOA
#define FLT_OUT_Pin GPIO_PIN_7
#define FLT_OUT_GPIO_Port GPIOA
#define LLIM_OUT_Pin GPIO_PIN_4
#define LLIM_OUT_GPIO_Port GPIOC
#define HLIM_OUT_Pin GPIO_PIN_5
#define HLIM_OUT_GPIO_Port GPIOC
#define FAN_PWM_Pin GPIO_PIN_0
#define FAN_PWM_GPIO_Port GPIOB
#define COM_OUT_Pin GPIO_PIN_1
#define COM_OUT_GPIO_Port GPIOB
#define SPI2_CS_Pin GPIO_PIN_12
#define SPI2_CS_GPIO_Port GPIOB
#define LED_OUT_Pin GPIO_PIN_6
#define LED_OUT_GPIO_Port GPIOC
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
