/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
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

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/** Helps write float values into the CAN message data array.
 * 	Instead of converting 32-bit floating point values into 4 byte arrays manually,
 * 	the union automatically takes care of that conversion. This conversion is necessary
 * 	since you cannot write float values directly into the CAN message data field. The float
 * 	value must first be converted to an array of 4 bytes (uint8_t).
 *
 */
typedef union FloatBytes {
	float float_value;			/**< Float value member of the union. */
	uint8_t bytes[4];			/**< Array of 4 bytes member of union. */
} FloatBytes;

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
void Convert_Values(uint8_t index);

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define TEMP_1_Pin GPIO_PIN_0
#define TEMP_1_GPIO_Port GPIOA
#define TEMP_2_Pin GPIO_PIN_1
#define TEMP_2_GPIO_Port GPIOA
#define TEMP_3_Pin GPIO_PIN_2
#define TEMP_3_GPIO_Port GPIOA
#define TEMP_4_Pin GPIO_PIN_3
#define TEMP_4_GPIO_Port GPIOA
#define ISENSE2_Pin GPIO_PIN_4
#define ISENSE2_GPIO_Port GPIOA
#define VSENSE2_Pin GPIO_PIN_5
#define VSENSE2_GPIO_Port GPIOA
#define VSENSE1_Pin GPIO_PIN_6
#define VSENSE1_GPIO_Port GPIOA
#define ISENSE1_Pin GPIO_PIN_7
#define ISENSE1_GPIO_Port GPIOA
#define TEMP_5_Pin GPIO_PIN_4
#define TEMP_5_GPIO_Port GPIOC
#define TEMP_6_Pin GPIO_PIN_5
#define TEMP_6_GPIO_Port GPIOC
#define TEMP_7_Pin GPIO_PIN_0
#define TEMP_7_GPIO_Port GPIOB
#define TEMP_8_Pin GPIO_PIN_1
#define TEMP_8_GPIO_Port GPIOB
#define SWDIO_Pin GPIO_PIN_13
#define SWDIO_GPIO_Port GPIOA
#define SWCLK_Pin GPIO_PIN_14
#define SWCLK_GPIO_Port GPIOA

/* USER CODE BEGIN Private defines */

/* Number of ADC Channels Used */
#define ADC_BUF_SIZE	12

/* Number of ADC Channels Total */
#define ADC_CHANNELS 16

/* 12 ADC Channel Assignments (10-13 NOT IN USE) */
#define VSENSE1			6
#define VSENSE2			5
#define ISENSE1			7
#define ISENSE2			4
#define TEMP_1			0
#define TEMP_2			1
#define TEMP_3			2
#define TEMP_4			3
#define TEMP_5			14
#define TEMP_6			15
#define TEMP_7			8
#define TEMP_8			9

/* Conversion Constants */
#define ISENSE_OFFSET_CURRENT		0.0639
#define ISENSE_OFFSET_VOLTAGE		1.65
#define VSENSE_OFFSET_VOLTS			0.645

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
