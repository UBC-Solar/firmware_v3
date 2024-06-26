/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    can.c
  * @brief   This file provides code for the configuration
  *          of the CAN instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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

xQueueHandle CAN_rx_queue;

/**
 * 	CAN message header for a drive command. This command header is to
 * 	send an appropriate drive command to the motor controller.
 */
CAN_TxHeaderTypeDef drive_command_header = {
    .StdId = MOTOR_DRIVE_COMMAND_ADDRESS,
    .ExtId = 0x0000,
    .IDE = CAN_ID_STD,
    .RTR = CAN_RTR_DATA,
    .DLC = CAN_DATA_LENGTH};

/**
 * 	CAN message header for DID next page command.
 */

CAN_TxHeaderTypeDef DID_next_page_header = {
    .StdId = DRIVER_INTERFACE_ADDRESS,
    .ExtId = 0x0000,
    .IDE = CAN_ID_STD,
    .RTR = CAN_RTR_DATA,
    .DLC = CAN_DATA_LENGTH};

/**
 * 	CAN message header for Drive state (0x403).
 */

CAN_TxHeaderTypeDef drive_state_header = {
    .StdId = MOTOR_DRIVE_STATE,
    .ExtId = 0x0000,
    .IDE = CAN_ID_STD,
    .RTR = CAN_RTR_DATA,
    .DLC = CAN_DATA_LENGTH};

/**
 * 	CAN message header for mcb diagnostics (0x404).
 */
CAN_TxHeaderTypeDef mcb_diagnostics = {
    .StdId = MCB_DIAGNOSTICS_ADDRESS,
    .ExtId = 0x0000,
    .IDE = CAN_ID_STD,
    .RTR = CAN_RTR_DATA,
    .DLC = CAN_DATA_LENGTH};

/**
 * 	CAN message header for mcb githash (0x405).
 */
CAN_TxHeaderTypeDef mcb_githash = {
    .StdId = MCB_GITHASH_ADDRESS,
    .ExtId = 0x0000,
    .IDE = CAN_ID_STD,
    .RTR = CAN_RTR_DATA,
    .DLC = CAN_DATA_LENGTH};


uint32_t can_mailbox;

CAN_RxHeaderTypeDef can_rx_header;    /**< Stores the header information for CAN messages read from the RX (receive) buffer */


/**
 * @brief Initialize CAN node for sending and receiving
 * @param: CAN filter structure
 * @retval: nothing
 */
//The MCB board requires 3 CAN messages, i.e. 0x400, 0x401,and 0x402. The filter set up requires one filter bank with a 16 bit scale, set in list mode, covering all the messages needed while one message is duplicated.

void CanFilterSetup()
{
	// Can Filter set up for MCB

		// one filter in mask mode for 0x503, 0x622, 0x626 messages.
	    CAN_filter0.FilterIdHigh = (uint16_t) (0x503 << 5);
	    CAN_filter0.FilterMaskIdHigh = (uint16_t) (0x503 << 5);

	    CAN_filter0.FilterIdLow = (uint16_t) (0x622 << 5);
	    CAN_filter0.FilterMaskIdLow = (uint16_t) (0x626 << 5);

	    CAN_filter0.FilterFIFOAssignment = CAN_FILTER_FIFO0;
	    CAN_filter0.FilterBank = (uint32_t) 0;
	    CAN_filter0.FilterMode = CAN_FILTERMODE_IDLIST;
	    CAN_filter0.FilterScale = CAN_FILTERSCALE_16BIT;
	    CAN_filter0.FilterActivation = CAN_FILTER_ENABLE;

		// Configure reception filters
	    HAL_CAN_ConfigFilter(&hcan, &CAN_filter0);
}




/* USER CODE END 0 */

CAN_HandleTypeDef hcan;

/* CAN init function */
void MX_CAN_Init(void)
{

  /* USER CODE BEGIN CAN_Init 0 */

  CAN_rx_queue = xQueueCreate(32, sizeof(CAN_msg_t));

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

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
  // Using queue with freeRTOS: https://controllerstech.com/freertos-tutorial-5-using-queue/
  CAN_RxHeaderTypeDef can_rx_header;
  uint8_t can_data[8];

  /* Get CAN message */
  HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &can_rx_header, can_data); 

  /* Put CAN message in the Queue */
  CAN_msg_t new_CAN_msg;
  new_CAN_msg.header = can_rx_header;
  for(int i = 0; i < 8; i++) 
  {
    new_CAN_msg.data[i] = can_data[i];
  }

  // Add a message to the queue
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  xQueueSendFromISR(CAN_rx_queue, &new_CAN_msg, &xHigherPriorityTaskWoken);

  portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
  /* Set the Flag to CAN_READY */
}

/* USER CODE END 1 */
