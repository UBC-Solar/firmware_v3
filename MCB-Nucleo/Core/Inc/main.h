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
#define ADC_Throttle_Pin GPIO_PIN_0
#define ADC_Throttle_GPIO_Port GPIOA
#define ADC_Regen_Pin GPIO_PIN_1
#define ADC_Regen_GPIO_Port GPIOA
#define USART_TX_Pin GPIO_PIN_2
#define USART_TX_GPIO_Port GPIOA
#define USART_RX_Pin GPIO_PIN_3
#define USART_RX_GPIO_Port GPIOA
#define LD2_Pin GPIO_PIN_5
#define LD2_GPIO_Port GPIOA
#define MECH_BRAKE_Pin GPIO_PIN_1
#define MECH_BRAKE_GPIO_Port GPIOB
#define SWITCH_REVERSE_Pin GPIO_PIN_2
#define SWITCH_REVERSE_GPIO_Port GPIOB
#define SWITCH_PARK_Pin GPIO_PIN_11
#define SWITCH_PARK_GPIO_Port GPIOB
#define BTN_CRUISE_TOGGLE_Pin GPIO_PIN_13
#define BTN_CRUISE_TOGGLE_GPIO_Port GPIOB
#define BTN_CRUISE_TOGGLE_EXTI_IRQn EXTI15_10_IRQn
#define BTN_CRUISE_UP_Pin GPIO_PIN_14
#define BTN_CRUISE_UP_GPIO_Port GPIOB
#define BTN_CRUISE_UP_EXTI_IRQn EXTI15_10_IRQn
#define BTN_CRUISE_DOWN_Pin GPIO_PIN_15
#define BTN_CRUISE_DOWN_GPIO_Port GPIOB
#define BTN_CRUISE_DOWN_EXTI_IRQn EXTI15_10_IRQn
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
