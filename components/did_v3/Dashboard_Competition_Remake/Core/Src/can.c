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

/**
 * @brief Initialize CAN node for sending and receiving
 * @param: CAN filter structure
 * @retval: nothing
 */
void CanFilterSetup()
{
	// Messages received by the DID (0x622,0x623,0x624,0x625,0x501,0x503,0x50B,0x400, 0x401, 0x403, 0x750,).

	// Filter for 0x500 and 401 IDs in list mode
    CAN_filter0.FilterIdHigh = (uint16_t) (0x501 << 5);
    CAN_filter0.FilterMaskIdHigh = (uint16_t) (0x503 << 5);

    CAN_filter0.FilterIdLow = (uint16_t) (0x50B << 5);
    CAN_filter0.FilterMaskIdLow = (uint16_t) (0x750 << 5);

    CAN_filter0.FilterFIFOAssignment = CAN_FILTER_FIFO0;
    CAN_filter0.FilterBank = (uint32_t) 0;
    CAN_filter0.FilterMode = CAN_FILTERMODE_IDLIST;
    CAN_filter0.FilterScale = CAN_FILTERSCALE_16BIT;
    CAN_filter0.FilterActivation = CAN_FILTER_ENABLE;

    // Filter for 0x600 IDs in filter mode
    CAN_filter1.FilterIdHigh = (uint16_t) (0x622 << 5);
    CAN_filter1.FilterMaskIdHigh = (uint16_t) (0x623 << 5);

    CAN_filter1.FilterIdLow = (uint16_t) (0x624 << 5);
    CAN_filter1.FilterMaskIdLow = (uint16_t) (0x625 << 5);

    CAN_filter1.FilterFIFOAssignment = CAN_FILTER_FIFO0;
    CAN_filter1.FilterBank = (uint32_t) 1;
    CAN_filter1.FilterMode = CAN_FILTERMODE_IDLIST;
    CAN_filter1.FilterScale = CAN_FILTERSCALE_16BIT;
    CAN_filter1.FilterActivation = CAN_FILTER_ENABLE;

    // Filter for 0x400
    CAN_filter2.FilterIdHigh = (uint16_t) (0x400 << 5);
    CAN_filter2.FilterMaskIdHigh = (uint16_t) (0x401<< 5);

    CAN_filter2.FilterIdLow = (uint16_t) (0x403 << 5);
    CAN_filter2.FilterMaskIdLow = (uint16_t) (0x403 << 5);

    CAN_filter2.FilterFIFOAssignment = CAN_FILTER_FIFO0;
    CAN_filter2.FilterBank = (uint32_t) 2;
    CAN_filter2.FilterMode = CAN_FILTERMODE_IDLIST;
    CAN_filter2.FilterScale = CAN_FILTERSCALE_16BIT;
    CAN_filter2.FilterActivation = CAN_FILTER_ENABLE;



	// Configure reception filters
    HAL_CAN_ConfigFilter(&hcan, &CAN_filter0);
    HAL_CAN_ConfigFilter(&hcan, &CAN_filter1);
    HAL_CAN_ConfigFilter(&hcan, &CAN_filter2);
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
  hcan.Init.TimeSeg2 = CAN_BS2_2TQ;
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

/* USER CODE END 1 */
