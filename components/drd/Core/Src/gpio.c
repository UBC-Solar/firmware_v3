/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   This file provides code for the configuration
  *          of all used GPIO pins.
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
#include "gpio.h"

/* USER CODE BEGIN 0 */
#include "drive_state.h"
/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/** Configure pins as
        * Analog
        * Input
        * Output
        * EVENT_OUT
        * EXTI
*/
void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, BATT_UV_Pin|BATT_HI_Pin|BATT_FLT_Pin|BATT_OT_Pin
                          |DCH_OC_Pin|CH_OC_Pin|MTR_FLT_Pin|MTR_OC_Pin
                          |MTR_OT_Pin|ESTOP_Pin|DEBUG_LED_1_Pin|DEBUG_LED_2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, BATT_LO_Pin|BATT_OV_Pin|DISPLAY_A0_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, BMS_COMM_FLT_Pin|DISPLAY_RESET_Pin|DTR_OUT_Pin|RTS_OUT_Pin
                          |LTS_OUT_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : DTR_IN_Pin */
  GPIO_InitStruct.Pin = DTR_IN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(DTR_IN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : BATT_UV_Pin BATT_HI_Pin BATT_FLT_Pin BATT_OT_Pin
                           DCH_OC_Pin CH_OC_Pin MTR_FLT_Pin MTR_OC_Pin
                           MTR_OT_Pin ESTOP_Pin DEBUG_LED_1_Pin DEBUG_LED_2_Pin */
  GPIO_InitStruct.Pin = BATT_UV_Pin|BATT_HI_Pin|BATT_FLT_Pin|BATT_OT_Pin
                          |DCH_OC_Pin|CH_OC_Pin|MTR_FLT_Pin|MTR_OC_Pin
                          |MTR_OT_Pin|ESTOP_Pin|DEBUG_LED_1_Pin|DEBUG_LED_2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : BATT_LO_Pin DISPLAY_A0_Pin */
  GPIO_InitStruct.Pin = BATT_LO_Pin|DISPLAY_A0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : BATT_OV_Pin */
  GPIO_InitStruct.Pin = BATT_OV_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(BATT_OV_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : BMS_COMM_FLT_Pin DTR_OUT_Pin RTS_OUT_Pin LTS_OUT_Pin */
  GPIO_InitStruct.Pin = BMS_COMM_FLT_Pin|DTR_OUT_Pin|RTS_OUT_Pin|LTS_OUT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : REGEN_EN_Pin HAZARD_Pin */
  GPIO_InitStruct.Pin = REGEN_EN_Pin|HAZARD_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : DISPLAY_RESET_Pin */
  GPIO_InitStruct.Pin = DISPLAY_RESET_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(DISPLAY_RESET_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : BRK_IN_Pin */
  GPIO_InitStruct.Pin = BRK_IN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(BRK_IN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PARK_EN_Pin FORWARD_EN_Pin REVERSE_EN_Pin */
  GPIO_InitStruct.Pin = PARK_EN_Pin|FORWARD_EN_Pin|REVERSE_EN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

}

/* USER CODE BEGIN 2 */


/**
 * @brief GPIO Interrupt Handler
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	Drive_State_interrupt_handler(GPIO_Pin);
}
/* USER CODE END 2 */
