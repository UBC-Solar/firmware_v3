/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    can.c
  * @brief   This file provides code for the configuration
  *          of the CAN instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
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

/**
 * @brief Initialize CAN node for sending and receiving
 * @param: CAN filter structure
 * @retval: nothing
 */
void CanFilterSetup(void)
{
	// Use mask and list mode to filter IDs from the CAN ID BOM

	// Filter for 0x500 and 0x600 IDs
    CAN_filter0.FilterIdHigh = (uint16_t) (0x501 << 5);
    CAN_filter0.FilterMaskIdHigh = (uint16_t) (0x7F5 << 5);

    CAN_filter0.FilterIdLow = (uint16_t) (0x620 << 5);
    CAN_filter0.FilterMaskIdLow = (uint16_t) (0x7F8 << 5);

    CAN_filter0.FilterFIFOAssignment = CAN_FILTER_FIFO0;
    CAN_filter0.FilterBank = (uint32_t) 0;
    CAN_filter0.FilterMode = CAN_FILTERMODE_IDMASK;
    CAN_filter0.FilterScale = CAN_FILTERSCALE_16BIT;
    CAN_filter0.FilterActivation = CAN_FILTER_ENABLE;

    // Remaining IDs filtered with list mode
    CAN_filter1.FilterIdHigh = (uint16_t) (0x502 << 5);
    CAN_filter1.FilterMaskIdHigh = (uint16_t) (0x401 << 5);

    CAN_filter1.FilterIdLow = (uint16_t) (0x401 << 5);
    CAN_filter1.FilterMaskIdLow = (uint16_t) (0x401 << 5);

    CAN_filter1.FilterFIFOAssignment = CAN_FILTER_FIFO0;
    CAN_filter1.FilterBank = (uint32_t) 1;
    CAN_filter1.FilterMode = CAN_FILTERMODE_IDLIST;
    CAN_filter1.FilterScale = CAN_FILTERSCALE_16BIT;
    CAN_filter1.FilterActivation = CAN_FILTER_ENABLE;

	// Configure reception filters
    HAL_CAN_ConfigFilter(&hcan, &CAN_filter0);
    HAL_CAN_ConfigFilter(&hcan, &CAN_filter1);

}

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
  hcan.Init.Prescaler = 4;
  hcan.Init.Mode = CAN_MODE_NORMAL;
  hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan.Init.TimeSeg1 = CAN_BS1_13TQ;
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
    HAL_NVIC_SetPriority(USB_LP_CAN1_RX0_IRQn, 5, 0);
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

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
    HAL_StatusTypeDef status = HAL_CAN_DeactivateNotification(hcan,
    CAN_IT_RX_FIFO0_MSG_PENDING);
    assert_param(status == HAL_OK);

    // osThreadFlagsClear(canReadMessagesTaskHandle);
    osThreadFlagsSet(readCANTaskHandle, CAN_READY);
}

/* USER CODE END 1 */
