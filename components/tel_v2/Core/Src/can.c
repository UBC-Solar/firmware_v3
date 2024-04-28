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

HAL_StatusTypeDef can_start;

CAN_TxHeaderTypeDef rtc_timestamp_header = {
    .StdId = RTC_TIMESTAMP,
    .ExtId = 0x0000,
    .IDE = CAN_ID_STD,
    .RTR = CAN_RTR_DATA,
    .DLC = CAN_DATA_LENGTH};

CAN_TxHeaderTypeDef tel_diagnostics_header = {
    .StdId = RTC_TIMESTAMP,
    .ExtId = 0x0000,
    .IDE = CAN_ID_STD,
    .RTR = CAN_RTR_DATA,
    .DLC = CAN_DATA_LENGTH};

uint32_t can_mailbox;

extern CAN_MSG_Rx_Queue;
extern CAN_MSG_memory_pool;

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
  hcan.Init.Prescaler = 2;
  hcan.Init.Mode = CAN_MODE_NORMAL;
  hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan.Init.TimeSeg1 = CAN_BS1_7TQ;
  hcan.Init.TimeSeg2 = CAN_BS2_8TQ;
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

void CanFilterSetup(void)
{
  /* TODO: Review Filter Implementation */
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


/*
 * CAN set-up: Sets up the filters, Starts CAN with HAL, and Activates notifications for interrupts.
 */
void Can_Init(void)
{
  CanFilterSetup();
  can_start = HAL_CAN_Start(&hcan);
  assert_param(can_start == HAL_OK);

  HAL_StatusTypeDef can_notification_status = HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING);
  assert_param(can_notification_status == HAL_OK);

  /* To avoid warning of unused variable */
  (void) can_notification_status;
}


void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
  //HAL_StatusTypeDef status = HAL_CAN_DeactivateNotification(hcan, CAN_IT_RX_FIFO0_MSG_PENDING);

  /* Assert the status */
  //assert_param(status == HAL_OK);
  CAN_RxHeaderTypeDef can_rx_header;
  uint8_t can_data[8];


  /* Get CAN message */
//  while(HAL_CAN_GetRxFifoFillLevel(hcan, CAN_RX_FIFO0) != 0) {
    HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &can_rx_header, can_data);  // TODO: Put can_rx_header and can_data into a data structure able to be accessed in the freertos task
  //  printf("%d\n\r", HAL_CAN_GetRxFifoFillLevel(hcan, CAN_RX_FIFO0));
    /* Put CAN message in the Queue */
    CAN_msg_t *new_CAN_msg;
    new_CAN_msg = osPoolAlloc(CAN_MSG_memory_pool);
    new_CAN_msg->header = can_rx_header;
    for(int i = 0; i < 8; i++) {
      new_CAN_msg->data[i] = can_data[i];
    }
    osMessagePut(CAN_MSG_Rx_Queue, new_CAN_msg, osWaitForever);
//  }

  /* Set the Flag to CAN_READY */
  osSignalSet(readCANTaskHandle, CAN_READY);

  /* To avoid warning of unused variable */
  //(void) status;
}

/* USER CODE END 1 */
