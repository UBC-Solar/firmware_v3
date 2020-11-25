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
#define FAN3_CTRL_Pin GPIO_PIN_1
#define FAN3_CTRL_GPIO_Port GPIOC
#define MDUFAN_CTRL_Pin GPIO_PIN_2
#define MDUFAN_CTRL_GPIO_Port GPIOC
#define DIST_RST_Pin GPIO_PIN_3
#define DIST_RST_GPIO_Port GPIOC
#define Bsense_Pin GPIO_PIN_1
#define Bsense_GPIO_Port GPIOA
#define Msense_Pin GPIO_PIN_2
#define Msense_GPIO_Port GPIOA
#define Asense_Pin GPIO_PIN_3
#define Asense_GPIO_Port GPIOA
#define LV_SENSE_Pin GPIO_PIN_4
#define LV_SENSE_GPIO_Port GPIOA
#define DCDC_P_CTRL_Pin GPIO_PIN_5
#define DCDC_P_CTRL_GPIO_Port GPIOA
#define DCDC_M_CTRL_Pin GPIO_PIN_6
#define DCDC_M_CTRL_GPIO_Port GPIOA
#define FAN1_CTRL_Pin GPIO_PIN_7
#define FAN1_CTRL_GPIO_Port GPIOA
#define SUPP_SENSE_Pin GPIO_PIN_4
#define SUPP_SENSE_GPIO_Port GPIOC
#define LOCKOUT_EN_Pin GPIO_PIN_5
#define LOCKOUT_EN_GPIO_Port GPIOC
#define OC_LATCH_SET_Pin GPIO_PIN_0
#define OC_LATCH_SET_GPIO_Port GPIOB
#define GPIO_BMS_Pin GPIO_PIN_10
#define GPIO_BMS_GPIO_Port GPIOB
#define OT_OUT_Pin GPIO_PIN_11
#define OT_OUT_GPIO_Port GPIOB
#define HLIM_Pin GPIO_PIN_12
#define HLIM_GPIO_Port GPIOB
#define ESTOP_5V_IN_Pin GPIO_PIN_13
#define ESTOP_5V_IN_GPIO_Port GPIOB
#define FLT_OUT_Pin GPIO_PIN_14
#define FLT_OUT_GPIO_Port GPIOB
#define SUPP_LOW_Pin GPIO_PIN_15
#define SUPP_LOW_GPIO_Port GPIOB
#define SWAP_EN_Pin GPIO_PIN_6
#define SWAP_EN_GPIO_Port GPIOC
#define SPAR2_CTRL_Pin GPIO_PIN_7
#define SPAR2_CTRL_GPIO_Port GPIOC
#define SPAR1_STRL_Pin GPIO_PIN_8
#define SPAR1_STRL_GPIO_Port GPIOC
#define TELEM_CTRL_Pin GPIO_PIN_9
#define TELEM_CTRL_GPIO_Port GPIOC
#define FAN2_CTRL_Pin GPIO_PIN_8
#define FAN2_CTRL_GPIO_Port GPIOA
#define LLIM_CTRL_Pin GPIO_PIN_9
#define LLIM_CTRL_GPIO_Port GPIOA
#define NEG_CTRL_Pin GPIO_PIN_10
#define NEG_CTRL_GPIO_Port GPIOA
#define PC_CTRL_Pin GPIO_PIN_11
#define PC_CTRL_GPIO_Port GPIOA
#define HLIM_CTRL_Pin GPIO_PIN_12
#define HLIM_CTRL_GPIO_Port GPIOA
#define DASH_CTRL_Pin GPIO_PIN_10
#define DASH_CTRL_GPIO_Port GPIOC
#define AMB_CTRL_Pin GPIO_PIN_11
#define AMB_CTRL_GPIO_Port GPIOC
#define FLT_BMS_Pin GPIO_PIN_4
#define FLT_BMS_GPIO_Port GPIOB
#define BAL_BMS_Pin GPIO_PIN_5
#define BAL_BMS_GPIO_Port GPIOB
#define COM_BMS_Pin GPIO_PIN_6
#define COM_BMS_GPIO_Port GPIOB
#define LLIM_Pin GPIO_PIN_7
#define LLIM_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
