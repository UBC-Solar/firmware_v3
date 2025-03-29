/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    iwdg.h
  * @brief   This file contains all the function prototypes for
  *          the iwdg.c file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#ifndef __IWDG_H__
#define __IWDG_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <stdbool.h>

/* USER CODE BEGIN Includes */
/* USER CODE END Includes */

extern IWDG_HandleTypeDef hiwdg;

/* USER CODE BEGIN Private defines */
#define RESET_SEQUENCE_DELAY_MS      200

/* USER CODE END Private defines */

void MX_IWDG_Init(void);

/* USER CODE BEGIN Prototypes */
void IWDG_Refresh(IWDG_HandleTypeDef* hiwdg);
bool IWDG_is_reset(); 
void IWDG_perform_reset_sequence();

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __IWDG_H__ */

