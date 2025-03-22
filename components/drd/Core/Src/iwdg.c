/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    iwdg.c
  * @brief   This file provides code for the configuration
  *          of the IWDG instances.
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
/* Includes ------------------------------------------------------------------*/
#include "iwdg.h"

/* USER CODE BEGIN 0 */
#include <stdbool.h>
#include "lcd.h"
static bool IWDG_is_reset();
/* USER CODE END 0 */

IWDG_HandleTypeDef hiwdg;

/* IWDG init function */
void MX_IWDG_Init(void)
{

  /* USER CODE BEGIN IWDG_Init 0 */
	#ifdef DEBUG
		return;
	#endif
  /* USER CODE END IWDG_Init 0 */

  /* USER CODE BEGIN IWDG_Init 1 */

  /* USER CODE END IWDG_Init 1 */
  hiwdg.Instance = IWDG;
  hiwdg.Init.Prescaler = IWDG_PRESCALER_4;
  hiwdg.Init.Reload = 799;
  if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN IWDG_Init 2 */

  /* USER CODE END IWDG_Init 2 */

}

/* USER CODE BEGIN 1 */

/*
 * @brief Refresh the IWDG.
 * @param hiwdg1 pointer to a IWDG_HandleTypeDef
 */
void IWDG_Refresh(IWDG_HandleTypeDef* hiwdg1)
{
	#ifndef DEBUG
		HAL_IWDG_Refresh(hiwdg1);
	#endif
}


/**
 * @brief Check if the IWDG reset occurred
 *
 * @return true if the IWDG reset occurred, and reset watchdog flags.
 */
bool IWDG_is_reset()
{
	if (__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST) != RESET)
	{
		__HAL_RCC_CLEAR_RESET_FLAGS();
		return true;
	}
	else
	{
		return false;
	}
}

/**
 * @brief Check if reset occurred, and handle accordingly.
 *
 * If the watchdog has triggered, and update the diagnostic message to indicate.
 */
int count = 0;
void IWDG_reset_handle()
{
	if (IWDG_is_reset())
	{
		for (int i = 0; i < 10; i++)
		{
		//Todo: indicate watchdog trigger in diagnostic message
			count++;
			HAL_Delay(1000);
			LCD_display_speed(count, 1);
		}
	}
}

/* USER CODE END 1 */
