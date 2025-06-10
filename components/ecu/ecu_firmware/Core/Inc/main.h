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
#define GPIO_BMS_Pin GPIO_PIN_13
#define GPIO_BMS_GPIO_Port GPIOC
#define SUPP_LOW_Pin GPIO_PIN_0
#define SUPP_LOW_GPIO_Port GPIOC
#define PACK_FANS_CTRL_Pin GPIO_PIN_1
#define PACK_FANS_CTRL_GPIO_Port GPIOC
#define MDU_FAN_CTRL_Pin GPIO_PIN_2
#define MDU_FAN_CTRL_GPIO_Port GPIOC
#define OC_REF_SENSE_Pin GPIO_PIN_3
#define OC_REF_SENSE_GPIO_Port GPIOC
#define OC_LATCH_SET_Pin GPIO_PIN_1
#define OC_LATCH_SET_GPIO_Port GPIOA
#define LED_OUT_Pin GPIO_PIN_2
#define LED_OUT_GPIO_Port GPIOA
#define FLT_OUT_Pin GPIO_PIN_3
#define FLT_OUT_GPIO_Port GPIOA
#define ESTOP_STATUS_Pin GPIO_PIN_4
#define ESTOP_STATUS_GPIO_Port GPIOA
#define ESTOP_STATUS_EXTI_IRQn EXTI4_IRQn
#define OD_REF_SENSE_Pin GPIO_PIN_5
#define OD_REF_SENSE_GPIO_Port GPIOA
#define SUPP_SENSE_Pin GPIO_PIN_6
#define SUPP_SENSE_GPIO_Port GPIOA
#define PACK_CURRENT_OFFSET_SENSE_Pin GPIO_PIN_7
#define PACK_CURRENT_OFFSET_SENSE_GPIO_Port GPIOA
#define PACK_CURRENT_SENSE_Pin GPIO_PIN_4
#define PACK_CURRENT_SENSE_GPIO_Port GPIOC
#define T_AMBIENT_SENSE_Pin GPIO_PIN_5
#define T_AMBIENT_SENSE_GPIO_Port GPIOC
#define LVS_CURRENT_OFFSET_SENSE_Pin GPIO_PIN_0
#define LVS_CURRENT_OFFSET_SENSE_GPIO_Port GPIOB
#define LVS_CURRENT_SENSE_Pin GPIO_PIN_1
#define LVS_CURRENT_SENSE_GPIO_Port GPIOB
#define BOOT1_Pin GPIO_PIN_2
#define BOOT1_GPIO_Port GPIOB
#define SPAR1_CTRL_Pin GPIO_PIN_10
#define SPAR1_CTRL_GPIO_Port GPIOB
#define TEL_CTRL_Pin GPIO_PIN_11
#define TEL_CTRL_GPIO_Port GPIOB
#define Unused1_CTRL_Pin GPIO_PIN_12
#define Unused1_CTRL_GPIO_Port GPIOB
#define MPPT_PC_CTRL_Pin GPIO_PIN_13
#define MPPT_PC_CTRL_GPIO_Port GPIOB
#define DRD_CTRL_Pin GPIO_PIN_14
#define DRD_CTRL_GPIO_Port GPIOB
#define MDU_CTRL_Pin GPIO_PIN_15
#define MDU_CTRL_GPIO_Port GPIOB
#define DCH_RST_Pin GPIO_PIN_6
#define DCH_RST_GPIO_Port GPIOC
#define DOC_COC_LED_Pin GPIO_PIN_7
#define DOC_COC_LED_GPIO_Port GPIOC
#define LLIM_CTRL_Pin GPIO_PIN_8
#define LLIM_CTRL_GPIO_Port GPIOC
#define PC_CTRL_Pin GPIO_PIN_9
#define PC_CTRL_GPIO_Port GPIOC
#define HLIM_CTRL_Pin GPIO_PIN_8
#define HLIM_CTRL_GPIO_Port GPIOA
#define NEG_CTRL_Pin GPIO_PIN_9
#define NEG_CTRL_GPIO_Port GPIOA
#define SWAP_CTRL_Pin GPIO_PIN_10
#define SWAP_CTRL_GPIO_Port GPIOA
#define ESTOP_LED_Pin GPIO_PIN_11
#define ESTOP_LED_GPIO_Port GPIOA
#define POS_CTRL_Pin GPIO_PIN_12
#define POS_CTRL_GPIO_Port GPIOA
#define DOC_COC_Pin GPIO_PIN_15
#define DOC_COC_GPIO_Port GPIOA
#define HLIM_BMS_Pin GPIO_PIN_11
#define HLIM_BMS_GPIO_Port GPIOC
#define LLIM_BMS_Pin GPIO_PIN_3
#define LLIM_BMS_GPIO_Port GPIOB
#define FLT_BMS_Pin GPIO_PIN_4
#define FLT_BMS_GPIO_Port GPIOB
#define COM_BMS_Pin GPIO_PIN_5
#define COM_BMS_GPIO_Port GPIOB
#define BAL_BMS_Pin GPIO_PIN_6
#define BAL_BMS_GPIO_Port GPIOB
#define OT_BMS_Pin GPIO_PIN_7
#define OT_BMS_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

#define DEBUG_LED_BLINK_INTERVAL 500 // ms

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
