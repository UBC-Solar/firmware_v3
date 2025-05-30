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

/* PRIVATE INCLUDES */
#include "diagnostic.h"

/* PRIVATE DEFINES */
#define RESET_SEQUENCE_DELAY_MS      200           

/* USER CODE END 0 */

IWDG_HandleTypeDef hiwdg;

/* IWDG init function */
void MX_IWDG_Init(void)
{

  /* USER CODE BEGIN IWDG_Init 0 */

  // If we're in DEBUG configuration, do not start watchdog
  #ifdef DEBUG
  return;
  #endif
  /* USER CODE END IWDG_Init 0 */

  /* USER CODE BEGIN IWDG_Init 1 */

  /* USER CODE END IWDG_Init 1 */
  hiwdg.Instance = IWDG;
  hiwdg.Init.Prescaler = IWDG_PRESCALER_4;
  hiwdg.Init.Reload = 4095;
  if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN IWDG_Init 2 */

  /* USER CODE END IWDG_Init 2 */

}

/* USER CODE BEGIN 1 */

/**
 * @brief Refresh the IWDG.
 * @param hiwdg pointer to a IWDG_HandleTypeDef
 */
void IWDG_Refresh()
{
  #ifndef DEBUG
    HAL_IWDG_Refresh(&hiwdg);
  #endif
}

/**
 * @brief Check if the IWDG reset occurred
 * 
 * @return true if the IWDG reset occurred
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
 * @brief Perform a reset LED sequence if the IWDG reset occurred.
 * 
 * This function will toggle the USER_LED of the TEL board 5 times at 200ms intervals
 */
void IWDG_perform_reset_sequence()
{
  if (IWDG_is_reset())
  {
    g_mdi_diagnostic_flags.bits.mdi_crash_iwdg = true;

    HAL_GPIO_TogglePin(BootLED_GPIO_Port, BootLED_Pin);
  }
}


/* USER CODE END 1 */
