/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    can.c
  * @brief   This file provides code for the configuration
  *          of the CAN instances.
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
#include "can.h"

/* USER CODE BEGIN 0 */

/* PRIVATE INCLUDES */
#define TURN_SIGNAL_MODE_HORN_CAN_DATA_LENGTH 1
#define STR_BOOTUP_CAN_DATA_LENGTH 4
#define STR_DIAGNOSTIC_FLAGS_CAN_DATA_LENGTH 1

#define TURN_SIGNAL_MODE_HORN_MSG_ID  0x580
#define STR_BOOTUP_MSG_ID 0x581
#define STR_DIAGNOSTIC_FLAGS_MSG_ID 0x582

/**
 * @brief CAN message headers for STR
 */
CAN_TxHeaderTypeDef turn_signal_mode_horn_can_header = {
    .StdId = TURN_SIGNAL_MODE_HORN_MSG_ID,
    .ExtId = 0x0000,
    .IDE = CAN_ID_STD,
    .RTR = CAN_RTR_DATA,
    .DLC = TURN_SIGNAL_MODE_HORN_CAN_DATA_LENGTH};

/**
 * @brief CAN message headers for STR bootup time
 */
CAN_TxHeaderTypeDef STR_time_since_bootup_can_header = {
  .StdId = STR_BOOTUP_MSG_ID,
  .ExtId = 0x0000,
  .IDE = CAN_ID_STD,
  .RTR = CAN_RTR_DATA,
  .DLC = STR_BOOTUP_CAN_DATA_LENGTH};

/**
 * @brief CAN message headers for STR diagnostic flags
 */
CAN_TxHeaderTypeDef STR_diagnostic_flags_can_header = {
  .StdId = STR_DIAGNOSTIC_FLAGS_MSG_ID,
  .ExtId = 0x0000,
  .IDE = CAN_ID_STD,
  .RTR = CAN_RTR_DATA,
  .DLC = STR_DIAGNOSTIC_FLAGS_CAN_DATA_LENGTH};

CAN_FilterTypeDef can_filter;

/* USER CODE END 0 */

CAN_HandleTypeDef hcan;

/* CAN init function */
void MX_CAN_Init(void)
{

  /* USER CODE BEGIN CAN_Init 0 */

  /* USER CODE END CAN_Init 0 */

  /* USER CODE BEGIN CAN_Init 1 */

  /* USER CODE END CAN_Init 1 */
  hcan.Instance = CAN1;
  hcan.Init.Prescaler = 8;
  hcan.Init.Mode = CAN_MODE_NORMAL;
  hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan.Init.TimeSeg1 = CAN_BS1_4TQ;
  hcan.Init.TimeSeg2 = CAN_BS2_4TQ;
  hcan.Init.TimeTriggeredMode = DISABLE;
  hcan.Init.AutoBusOff = DISABLE;
  hcan.Init.AutoWakeUp = DISABLE;
  hcan.Init.AutoRetransmission = DISABLE;
  hcan.Init.ReceiveFifoLocked = DISABLE;
  hcan.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN_Init 2 */
  HAL_CAN_ConfigFilter(&hcan, &can_filter);
  HAL_CAN_Start(&hcan);
  /* USER CODE END CAN_Init 2 */

}

void HAL_CAN_MspInit(CAN_HandleTypeDef* canHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(canHandle->Instance==CAN1)
  {
  /* USER CODE BEGIN CAN1_MspInit 0 */

  /* USER CODE END CAN1_MspInit 0 */
    /* CAN1 clock enable */
    __HAL_RCC_CAN1_CLK_ENABLE();

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**CAN GPIO Configuration
    PB8     ------> CAN_RX
    PB9     ------> CAN_TX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_8;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    __HAL_AFIO_REMAP_CAN1_2();

  /* USER CODE BEGIN CAN1_MspInit 1 */

  /* USER CODE END CAN1_MspInit 1 */
  }
}

void HAL_CAN_MspDeInit(CAN_HandleTypeDef* canHandle)
{

  if(canHandle->Instance==CAN1)
  {
  /* USER CODE BEGIN CAN1_MspDeInit 0 */

  /* USER CODE END CAN1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_CAN1_CLK_DISABLE();

    /**CAN GPIO Configuration
    PB8     ------> CAN_RX
    PB9     ------> CAN_TX
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_8|GPIO_PIN_9);

  /* USER CODE BEGIN CAN1_MspDeInit 1 */

  /* USER CODE END CAN1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/**
 * @brief CAN message for the status of the turn signal, power/eco mode and the horn
 */
void CAN_tx_turn_signal_mode_horn_msg(turn_signal_status_t turn_signal, mode_status_t mode_status, horn_status_t horn_status) {

  uint8_t turn_signal_mode_horn_reading[1];
  turn_signal_mode_horn_reading[0] = (horn_status << 3) | (mode_status << 2) | (turn_signal);

  uint32_t mailbox;

  HAL_CAN_AddTxMessage(&hcan, &turn_signal_mode_horn_can_header, turn_signal_mode_horn_reading, &mailbox);
}

/**
 * @brief CAN message for the STR bootup time, or board heartbeat
 */
void STR_time_since_bootup(uint32_t time) {

  union {
    uint8_t bytes[4];
    uint32_t data;
  } CAN_data;
  CAN_data.data = time;

  uint32_t mailbox;

  HAL_CAN_AddTxMessage(&hcan, &STR_time_since_bootup_can_header, CAN_data.bytes, &mailbox);
}

/**
 * @brief CAN message for the iwdg and checking if the board has crashed or not
 */
void STR_diagnostic_flags()
{
  uint8_t data[1] = {g_str_diagnostic_flags.raw};

  uint32_t mailbox;
  HAL_CAN_AddTxMessage(&hcan, &STR_diagnostic_flags_can_header, data, &mailbox);
}
/* USER CODE END 1 */
