/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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
#define ST_1_Pin GPIO_PIN_0
#define ST_1_GPIO_Port GPIOA
#define ST_2_Pin GPIO_PIN_1
#define ST_2_GPIO_Port GPIOA
#define ST_3_Pin GPIO_PIN_2
#define ST_3_GPIO_Port GPIOA
#define ST_4_Pin GPIO_PIN_3
#define ST_4_GPIO_Port GPIOA
#define BP_1_Pin GPIO_PIN_4
#define BP_1_GPIO_Port GPIOA
#define BP_2_Pin GPIO_PIN_5
#define BP_2_GPIO_Port GPIOA
#define BP_3_Pin GPIO_PIN_6
#define BP_3_GPIO_Port GPIOA
#define SA_1_Pin GPIO_PIN_7
#define SA_1_GPIO_Port GPIOA

/* USER CODE BEGIN Private defines */

/* Number of ADC Channels used */
#define NUM_ADC_CHANNELS_USED 8

/*Number of ADC Channels Total */
#define NUM_ADC_CHANNELS_TOTAL 16

/* Sensor ADC Channel Assignments (0-7 IN USE)*/
#define ST_1 0
#define ST_2 1
#define ST_3 2
#define ST_4 3
#define BP_1 4
#define BP_2 5
#define BP_3 6
#define SA_1 7


/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
