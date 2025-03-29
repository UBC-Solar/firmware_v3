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
#include "main.h"
#include "diagnostic.h"

void CAN_Filter_Config();

// CAN message header for MDI time_since_bootup.
const CAN_TxHeaderTypeDef CAN_Tx_header_MDI_time_since_bootup = {
  .StdId = MDI_TIME_SINCE_BOOTUP_CAN_ID,
  .ExtId = 0x0000,
  .IDE = CAN_ID_STD,
  .RTR = CAN_RTR_DATA,
  .DLC = 4
};

// CAN message header for MDI diagnostic flags
const CAN_TxHeaderTypeDef CAN_Tx_header_MDI_diagnostic_flags = {
  .StdId = MDI_DIAGNOSTIC_FLAGS_CAN_ID,
  .ExtId = 0x0000,
  .IDE = CAN_ID_STD,
  .RTR = CAN_RTR_DATA,
  .DLC = 1
};

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

  CAN_Filter_Config();

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

    /* CAN1 interrupt Init */
    HAL_NVIC_SetPriority(USB_LP_CAN1_RX0_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);
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

    /* CAN1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USB_LP_CAN1_RX0_IRQn);
  /* USER CODE BEGIN CAN1_MspDeInit 1 */

  /* USER CODE END CAN1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
  uint8_t CAN_Rx_data[8] = {0};
  CAN_RxHeaderTypeDef CAN_Rx_header;

  if(HAL_OK == HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &CAN_Rx_header, CAN_Rx_data))
  {
    if(CAN_Rx_header.StdId == DRD_MOTOR_COMMAND_CAN_ID)
    {
      // Parse motor command and update the global MDI motor command variable
      MDI_parse_motor_command(CAN_Rx_data, &g_MDI_motor_command);

      // Set motor command received flag to true
      g_motor_command_received = true;
    }
  }
  else
  {
    // TODO: Error handling
  }
}


void CAN_Filter_Config()
{
  CAN_FilterTypeDef canFilterConfig;

  canFilterConfig.FilterBank = 0;                      // Use filter bank 0 (0 to 13 for F103)
  canFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;  // Mask mode for exact matching
  canFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT; // 32-bit filter
  canFilterConfig.FilterIdHigh = (0x401 << 5) >> 16;   // Standard ID shifted
  canFilterConfig.FilterIdLow = (0x401 << 5) & 0xFFFF;
  canFilterConfig.FilterMaskIdHigh = (0x7FF << 5) >> 16; // Mask only standard ID bits
  canFilterConfig.FilterMaskIdLow = (0x7FF << 5) & 0xFFFF;
  canFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0; // Assign to FIFO 0
  canFilterConfig.FilterActivation = ENABLE;           // Enable the filter

  HAL_CAN_ConfigFilter(&hcan, &canFilterConfig);
}

/**
 * @brief Sends the MDI time_since_bootup CAN message
 */
void MDI_time_since_bootup()
{
  // Create a static variable to hold the time_since_bootup counter
  static uint32_t time_since_bootup_counter = 0;

  // Create union for converting the uint32 to bytes
  union {
    uint8_t bytes[4];
    uint32_t data;
  } CAN_data;
  CAN_data.data = time_since_bootup_counter;

  // Transmit message over CAN
  uint32_t mailbox;
  HAL_CAN_AddTxMessage(&hcan, &CAN_Tx_header_MDI_time_since_bootup, CAN_data.bytes, &mailbox);

  // Increment hearbeat counter
  time_since_bootup_counter++;
}

/**
 * @brief Sends the MDI diagnostic flags like iwdg
 */
void MDI_diagnostic_flags()
{
  uint8_t data[1] = {g_mdi_diagnostic_flags.raw};

  // Transmit message over CAN
  uint32_t mailbox;
  HAL_CAN_AddTxMessage(&hcan, &CAN_Tx_header_MDI_diagnostic_flags, data, &mailbox);
}

/* USER CODE END 1 */
