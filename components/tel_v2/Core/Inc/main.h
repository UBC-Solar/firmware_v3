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
#include <stdbool.h>

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

typedef struct tel_diagnostics{
  volatile bool rtc_reset;
  volatile bool gps_sync_fail;
  volatile bool imu_fail;
  volatile bool gps_fail;
  volatile bool watchdog_reset;
} tel_diagnostics;

extern tel_diagnostics g_tel_diagnostics;

extern CAN_FilterTypeDef CAN_filter0;
extern CAN_FilterTypeDef CAN_filter1;

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
#define RADIO_GPIO_DIO7_Pin GPIO_PIN_3
#define RADIO_GPIO_DIO7_GPIO_Port GPIOC
#define USER_LED_Pin GPIO_PIN_1
#define USER_LED_GPIO_Port GPIOA
#define RTC_SYNC_Pin GPIO_PIN_2
#define RTC_SYNC_GPIO_Port GPIOA
#define SDL_CD_Pin GPIO_PIN_3
#define SDL_CD_GPIO_Port GPIOA
#define SDL_CS_Pin GPIO_PIN_4
#define SDL_CS_GPIO_Port GPIOA
#define SDL_CLK_Pin GPIO_PIN_5
#define SDL_CLK_GPIO_Port GPIOA
#define SDL_MISO_Pin GPIO_PIN_6
#define SDL_MISO_GPIO_Port GPIOA
#define SDL_MOSI_Pin GPIO_PIN_7
#define SDL_MOSI_GPIO_Port GPIOA
#define RADIO_GPIO_DIO9_SLEEP_Pin GPIO_PIN_4
#define RADIO_GPIO_DIO9_SLEEP_GPIO_Port GPIOC
#define RADIO_GPIO_DIO11_Pin GPIO_PIN_5
#define RADIO_GPIO_DIO11_GPIO_Port GPIOC
#define RADIO_GPIO_DIO4_Pin GPIO_PIN_0
#define RADIO_GPIO_DIO4_GPIO_Port GPIOB
#define RADIO_GPIO_DIO12_Pin GPIO_PIN_1
#define RADIO_GPIO_DIO12_GPIO_Port GPIOB
#define BOOT1_Pin GPIO_PIN_2
#define BOOT1_GPIO_Port GPIOB
#define IMU_SCL_Pin GPIO_PIN_10
#define IMU_SCL_GPIO_Port GPIOB
#define IMU_SDA_Pin GPIO_PIN_11
#define IMU_SDA_GPIO_Port GPIOB
#define RADIO_GPIO_DIO6_Pin GPIO_PIN_12
#define RADIO_GPIO_DIO6_GPIO_Port GPIOB
#define RADIO_GPIO_DIO3_Pin GPIO_PIN_13
#define RADIO_GPIO_DIO3_GPIO_Port GPIOB
#define RADIO_DIO8_SLEEP_CONTROL_Pin GPIO_PIN_14
#define RADIO_DIO8_SLEEP_CONTROL_GPIO_Port GPIOB
#define RADIO_GPIO_DIO2_Pin GPIO_PIN_15
#define RADIO_GPIO_DIO2_GPIO_Port GPIOB
#define RADIO_GPIO_DIO10_Pin GPIO_PIN_6
#define RADIO_GPIO_DIO10_GPIO_Port GPIOC
#define RADIO_GPIO_DIO1_Pin GPIO_PIN_7
#define RADIO_GPIO_DIO1_GPIO_Port GPIOC
#define RADIO_GPIO_DIO0_Pin GPIO_PIN_8
#define RADIO_GPIO_DIO0_GPIO_Port GPIOC
#define RADIO_TX_Pin GPIO_PIN_9
#define RADIO_TX_GPIO_Port GPIOA
#define RADIO_RX_Pin GPIO_PIN_10
#define RADIO_RX_GPIO_Port GPIOA
#define GPS_SCL_Pin GPIO_PIN_6
#define GPS_SCL_GPIO_Port GPIOB
#define GPS_SDA_Pin GPIO_PIN_7
#define GPS_SDA_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
