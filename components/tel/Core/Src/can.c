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
#include <stdint.h>
#include <stdbool.h>
#include "bitops.h"
#include "radio.h"


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

/**
 * @brief Allows all messages to be received
 */
void CanFilterSetup(void)
{
  /* TODO: Review Filter Implementation */
  // Use mask and list mode to filter IDs from the CAN ID BOM

  // Filter for 0x500 and 0x600 IDs
  CAN_FilterTypeDef CAN_filter0;

  CAN_filter0.FilterIdHigh = 0x0000;
  CAN_filter0.FilterMaskIdHigh = 0x0000;

  CAN_filter0.FilterIdLow = 0x0000;
  CAN_filter0.FilterMaskIdLow = 0x0000;

  CAN_filter0.FilterFIFOAssignment = CAN_FILTER_FIFO0;
  CAN_filter0.FilterBank = (uint32_t) 0;
  CAN_filter0.FilterMode = CAN_FILTERMODE_IDMASK;
  CAN_filter0.FilterScale = CAN_FILTERSCALE_16BIT;
  CAN_filter0.FilterActivation = CAN_FILTER_ENABLE;

  // Configure reception filters
  HAL_CAN_ConfigFilter(&hcan, &CAN_filter0);
}


/**
 * @brief CAN set-up: Sets up the filters, Starts CAN with HAL, and Activates notifications for interrupts.
 */
void CAN_Init(void)
{
   HAL_StatusTypeDef can_start;

   RADIO_queue_init();                  // Set constant fields like delimeters and new lines

   CanFilterSetup();                    // Allows all msgs

   can_start = HAL_CAN_Start(&hcan);
   assert_param(can_start == HAL_OK);

   HAL_StatusTypeDef can_notification_status = HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING);
   assert_param(can_notification_status == HAL_OK);
   can_notification_status = HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO0_OVERRUN);
   assert_param(can_notification_status == HAL_OK);

   /* To avoid warning of unused variable */
   (void) can_notification_status;
   (void) can_start;
}


/**
 * @brief Getter for the CAN ID from the CAN Rx header
 * 
 * @param can_rx_header The CAN Rx header
 * 
 * @return The CAN ID as a 32-bit unsigned integer to account for both standard and extended IDs
 */
uint32_t get_can_id(CAN_RxHeaderTypeDef* can_rx_header)
{
    return (can_rx_header->IDE == CAN_ID_STD) ? can_rx_header->StdId : can_rx_header->ExtId;
}


void HAL_CAN_ErrorCallback(CAN_HandleTypeDef *hcan)
{
    if (hcan->ErrorCode & HAL_CAN_ERROR_RX_FOV1)
    {
        HAL_GPIO_TogglePin(USER_LED_GPIO_Port, USER_LED_Pin);
    }
}


static uint64_t can_msg_counter = 0;

/**
 * @brief Callback function for when a CAN message is received in the FIFO0
 */
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    ++can_msg_counter;
    // CAN_RxHeaderTypeDef can_rx_header;                        
    // uint8_t can_data[MAX_CAN_DATA_LENGTH];

    // HAL_GPIO_TogglePin(USER_LED_GPIO_Port, USER_LED_Pin);	    // Visual Confirmation of CAN working

    // HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &can_rx_header, &can_data[START_OF_ARRAY]);       
    // uint32_t can_id = get_can_id(&can_rx_header);
    // RADIO_set_rx_msg(can_id, can_data, can_rx_header.DLC);      // Queues Rxed CAN message for radio Tx   
}

/* USER CODE END 1 */
