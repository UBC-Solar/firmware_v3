/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
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

#include "cmsis_os.h"

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

extern TIM_HandleTypeDef htim3;

extern union float_bytes {
	float float_value;
	uint8_t bytes[4];
} current, velocity;

// this struct is updated when an external interrupt comes in and combines boolean flags 
// that helps to decide whether to send a regen command, normal command
typedef struct input_flags {
  volatile uint8_t regen_enable;
  volatile uint8_t reverse_enable;
  volatile uint8_t cruise_status;
  volatile uint8_t brake_in;
  volatile uint8_t regen_value_is_zero;
  volatile uint8_t encoder_value_is_zero;
} input_flags;

extern input_flags event_flags;

extern uint32_t regen_value;
extern uint8_t cruise_value;
extern uint8_t battery_soc;

/* USER CODE END ET */

/* Exported functions prototypes ---------------------------------------------*/

void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/

#define REGEN_VAL_Pin GPIO_PIN_0
#define REGEN_VAL_GPIO_Port GPIOA
#define REGEN_EN_Pin GPIO_PIN_1
#define REGEN_EN_GPIO_Port GPIOA
#define REGEN_EN_EXTI_IRQn EXTI1_IRQn
#define BRK_IN_Pin GPIO_PIN_2
#define BRK_IN_GPIO_Port GPIOA
#define BRK_IN_EXTI_IRQn EXTI2_IRQn
#define RVRS_EN_Pin GPIO_PIN_3
#define RVRS_EN_GPIO_Port GPIOA
#define RVRS_EN_EXTI_IRQn EXTI3_IRQn
#define CRUISE_EN_Pin GPIO_PIN_4
#define CRUISE_EN_GPIO_Port GPIOA
#define CRUISE_EN_EXTI_IRQn EXTI4_IRQn
#define CRUISE_DIS_Pin GPIO_PIN_5
#define CRUISE_DIS_GPIO_Port GPIOA
#define CRUISE_DIS_EXTI_IRQn EXTI9_5_IRQn
#define CRUISE_UP_Pin GPIO_PIN_6
#define CRUISE_UP_GPIO_Port GPIOA
#define CRUISE_UP_EXTI_IRQn EXTI9_5_IRQn
#define CRUISE_DOWN_Pin GPIO_PIN_7
#define CRUISE_DOWN_GPIO_Port GPIOA
#define CRUISE_DOWN_EXTI_IRQn EXTI9_5_IRQn
#define ENC_AS_Pin GPIO_PIN_8
#define ENC_AS_GPIO_Port GPIOA
#define ENC_BS_Pin GPIO_PIN_9
#define ENC_BS_GPIO_Port GPIOA
#define CRUISE_STAT_Pin GPIO_PIN_10
#define CRUISE_STAT_GPIO_Port GPIOA
#define SWDIO_Pin GPIO_PIN_13
#define SWDIO_GPIO_Port GPIOA
#define SWCLK_Pin GPIO_PIN_14
#define SWCLK_GPIO_Port GPIOA
#define ENC_ZS_Pin GPIO_PIN_15
#define ENC_ZS_GPIO_Port GPIOA
#define ENC_YS_Pin GPIO_PIN_3
#define ENC_YS_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

#define ADC_MAX 0xFFF
#define ADC_MIN 0

#define ENCODER_TIMER_TICKS (uint32_t) 1

#define CRUISE_INCREMENT_VALUE 1

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
