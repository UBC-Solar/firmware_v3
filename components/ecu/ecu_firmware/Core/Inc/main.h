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

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define OFFSET_REF_BATT_Pin GPIO_PIN_0
#define OFFSET_REF_BATT_GPIO_Port GPIOC
#define SPAR1_CTRL_Pin GPIO_PIN_1
#define SPAR1_CTRL_GPIO_Port GPIOC
#define AMB_CTRL_Pin GPIO_PIN_2
#define AMB_CTRL_GPIO_Port GPIOC
#define SPAR2_CTRL_Pin GPIO_PIN_3
#define SPAR2_CTRL_GPIO_Port GPIOC
#define OFFSET_REF_AM_Pin GPIO_PIN_0
#define OFFSET_REF_AM_GPIO_Port GPIOA
<<<<<<< HEAD
=======
#define Bsense_Pin GPIO_PIN_1
#define Bsense_GPIO_Port GPIOA
>>>>>>> user/jaynith/ecu/ecu-update
#define Msense_Pin GPIO_PIN_2
#define Msense_GPIO_Port GPIOA
#define Asense_Pin GPIO_PIN_3
#define Asense_GPIO_Port GPIOA
#define LVS_CURR_SENSE_Pin GPIO_PIN_4
#define LVS_CURR_SENSE_GPIO_Port GPIOA
<<<<<<< HEAD
#define SUPP_SENS_Pin GPIO_PIN_5
#define SUPP_SENS_GPIO_Port GPIOA
=======
#define FAN2_CTRL_Pin GPIO_PIN_5
#define FAN2_CTRL_GPIO_Port GPIOA
>>>>>>> user/jaynith/ecu/ecu-update
#define ESTOP_3_3V_IN_Pin GPIO_PIN_6
#define ESTOP_3_3V_IN_GPIO_Port GPIOA
#define OC_LATCH_SET_Pin GPIO_PIN_7
#define OC_LATCH_SET_GPIO_Port GPIOA
#define ESTOP_CTRL_Pin GPIO_PIN_4
#define ESTOP_CTRL_GPIO_Port GPIOC
#define TELEM_CTRL_Pin GPIO_PIN_5
#define TELEM_CTRL_GPIO_Port GPIOC
#define FAN3_CTRL_Pin GPIO_PIN_0
#define FAN3_CTRL_GPIO_Port GPIOB
#define GPIO_BMS_Pin GPIO_PIN_1
#define GPIO_BMS_GPIO_Port GPIOB
<<<<<<< HEAD
#define FAN2_CTRL_Pin GPIO_PIN_10
#define FAN2_CTRL_GPIO_Port GPIOB
=======
>>>>>>> user/jaynith/ecu/ecu-update
#define FAN1_CTRL_Pin GPIO_PIN_11
#define FAN1_CTRL_GPIO_Port GPIOB
#define FLT_OUT_Pin GPIO_PIN_12
#define FLT_OUT_GPIO_Port GPIOB
#define SUPP_LOW_Pin GPIO_PIN_13
#define SUPP_LOW_GPIO_Port GPIOB
#define FAN4_CTRL_Pin GPIO_PIN_14
#define FAN4_CTRL_GPIO_Port GPIOB
#define MDUFAN_CTRL_Pin GPIO_PIN_15
#define MDUFAN_CTRL_GPIO_Port GPIOB
#define OT_OUT_Pin GPIO_PIN_6
#define OT_OUT_GPIO_Port GPIOC
#define SWAP_CTRL_Pin GPIO_PIN_7
#define SWAP_CTRL_GPIO_Port GPIOC
#define HLIM_IN_Pin GPIO_PIN_8
#define HLIM_IN_GPIO_Port GPIOC
#define PC_CTRL_Pin GPIO_PIN_9
#define PC_CTRL_GPIO_Port GPIOC
#define HLIM_CTRL_Pin GPIO_PIN_8
#define HLIM_CTRL_GPIO_Port GPIOA
#define NEG_CTRL_Pin GPIO_PIN_9
#define NEG_CTRL_GPIO_Port GPIOA
#define DCDC_NEG_CTRL_Pin GPIO_PIN_11
#define DCDC_NEG_CTRL_GPIO_Port GPIOA
#define DCDC_POS_CTRL_Pin GPIO_PIN_12
#define DCDC_POS_CTRL_GPIO_Port GPIOA
#define SWDIO_Pin GPIO_PIN_13
#define SWDIO_GPIO_Port GPIOA
#define SWCLK_Pin GPIO_PIN_14
#define SWCLK_GPIO_Port GPIOA
#define DIST_RST_Pin GPIO_PIN_10
#define DIST_RST_GPIO_Port GPIOC
#define LLIM_CTRL_Pin GPIO_PIN_11
#define LLIM_CTRL_GPIO_Port GPIOC
#define LLIM_IN_Pin GPIO_PIN_3
#define LLIM_IN_GPIO_Port GPIOB
#define FLT_BMS_Pin GPIO_PIN_4
#define FLT_BMS_GPIO_Port GPIOB
#define COM_BMS_Pin GPIO_PIN_5
#define COM_BMS_GPIO_Port GPIOB
#define BAL_BMS_Pin GPIO_PIN_6
#define BAL_BMS_GPIO_Port GPIOB
#define DASH_CTRL_Pin GPIO_PIN_7
#define DASH_CTRL_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
