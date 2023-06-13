/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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
#define REGEN_EN_Pin GPIO_PIN_0
#define REGEN_EN_GPIO_Port GPIOC
#define PARK_EN_Pin GPIO_PIN_1
#define PARK_EN_GPIO_Port GPIOC
#define FWRD_EN_Pin GPIO_PIN_2
#define FWRD_EN_GPIO_Port GPIOC
#define RVRS_EN_Pin GPIO_PIN_3
#define RVRS_EN_GPIO_Port GPIOC
#define CRUISE_STAT_Pin GPIO_PIN_0
#define CRUISE_STAT_GPIO_Port GPIOA
#define TMR_OUT_Pin GPIO_PIN_1
#define TMR_OUT_GPIO_Port GPIOA
#define LED_OUT2_Pin GPIO_PIN_2
#define LED_OUT2_GPIO_Port GPIOA
#define LED_OUT1_Pin GPIO_PIN_3
#define LED_OUT1_GPIO_Port GPIOA
#define CRUISE_DWN_Pin GPIO_PIN_4
#define CRUISE_DWN_GPIO_Port GPIOA
#define CRUISE_UP_Pin GPIO_PIN_5
#define CRUISE_UP_GPIO_Port GPIOA
#define NEXT_SCREEN_Pin GPIO_PIN_6
#define NEXT_SCREEN_GPIO_Port GPIOA
#define CRUISE_TOGGLE_Pin GPIO_PIN_7
#define CRUISE_TOGGLE_GPIO_Port GPIOA
#define REGEN_VAL_Pin GPIO_PIN_5
#define REGEN_VAL_GPIO_Port GPIOC
#define BRK_IN_Pin GPIO_PIN_0
#define BRK_IN_GPIO_Port GPIOB
#define THROTTLE_VAL_Pin GPIO_PIN_1
#define THROTTLE_VAL_GPIO_Port GPIOB
#define TMS_Pin GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
#define SWO_Pin GPIO_PIN_3
#define SWO_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
