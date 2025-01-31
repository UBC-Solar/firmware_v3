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

/* PRIVATE INCLUDES */
#include <stdint.h>
#include <stdbool.h>
#include "bitops.h"
#include "radio.h"
#include "canload.h"

#define CANLOAD_MSG_ID                      0x760
#define CANLOAD_DATA_LENGTH                 1

/**
 * @brief CAN message header for sending out bus load
 */

CAN_TxHeaderTypeDef CANLOAD_busload = {
    .StdId = CANLOAD_MSG_ID,
    .ExtId = 0x0000,
    .IDE = CAN_ID_STD,
    .RTR = CAN_RTR_DATA,
    .DLC = CANLOAD_DATA_LENGTH};


#include "CAN_comms.h"
#include "rtc.h"
#include "cpu_load.h"
#include "bitops.h"

#define CPU_LOAD_CAN_MESSAGE_ID 0x759
#define CPU_LOAD_CAN_DATA_LENGTH 4

CAN_TxHeaderTypeDef cpu_load_can_header = {
    .StdId = CPU_LOAD_CAN_MESSAGE_ID,
    .ExtId = 0x0000,
    .IDE = CAN_ID_STD,
    .RTR = CAN_RTR_DATA,
    .DLC = CPU_LOAD_CAN_DATA_LENGTH};


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
 * @brief Callback from CAN comms to send a message over UART
 * 
 * @param CAN_comms_Rx_msg The CAN message received
 */
void CAN_comms_Rx_callback(CAN_comms_Rx_msg_t* CAN_comms_Rx_msg)
{
    HAL_GPIO_TogglePin(USER_LED_GPIO_Port, USER_LED_Pin);	    // Visual Confirmation of CAN working

    CANLOAD_calculate_message_bits(CAN_comms_Rx_msg->header.DLC, CAN_comms_Rx_msg->header.IDE); // Calculate CAN bus load

    RTC_check_and_sync_rtc(CAN_comms_Rx_msg->header.StdId, CAN_comms_Rx_msg->data);     // Sync timestamps

    RADIO_filter_and_queue_msg(CAN_comms_Rx_msg);
}


/**
 * @brief Allows all messages to be received
 */
void CAN_filter_init(CAN_FilterTypeDef* can_filter)
{
    /* TODO: Review Filter Implementation */
    // Use mask and list mode to filter IDs from the CAN ID BOM

    can_filter->FilterIdHigh = 0x0000;
    can_filter->FilterMaskIdHigh = 0x0000;

    can_filter->FilterIdLow = 0x0000;
    can_filter->FilterMaskIdLow = 0x0000;

    can_filter->FilterFIFOAssignment = CAN_FILTER_FIFO0;
    can_filter->FilterBank = (uint32_t) 0;
    can_filter->FilterMode = CAN_FILTERMODE_IDMASK;
    can_filter->FilterScale = CAN_FILTERSCALE_16BIT;
    can_filter->FilterActivation = CAN_FILTER_ENABLE;
}


/**
 * @brief Initializes the CAN filter and CAN Rx callback function as CAN_comms_Rx_callback().
 * 
 * Note: This uses the CAN_comms abstraction layer which will initialize two freeRTOS tasks. As a result it is recommend to 
 * Call this function inside the MX_FREERTOS_Init() function in freertos.c
 */
void CAN_tasks_init()
{
    CAN_comms_config_t CAN_comms_config_tel = {0}; 

    CAN_FilterTypeDef CAN_filter = {0};
    CAN_filter_init(&CAN_filter);

    CAN_comms_config_tel.hcan = &hcan;
    CAN_comms_config_tel.CAN_Filter = CAN_filter;
    CAN_comms_config_tel.CAN_comms_Rx_callback = CAN_comms_Rx_callback;   

    CAN_comms_init(&CAN_comms_config_tel);
}


/**
 * @brief Callback function for when a CAN error occurs
 * 
 * Currently only toggles the user LED upon a RX FIFO overrun
 */
void HAL_CAN_ErrorCallback(CAN_HandleTypeDef *hcan)
{
    // TODO: Add diagnostics. Dont toggle LED if you already do in Rx
    if (hcan->ErrorCode & HAL_CAN_ERROR_RX_FOV1)
    {
        HAL_GPIO_TogglePin(USER_LED_GPIO_Port, USER_LED_Pin);   // TODO: add diagnostics
    }
}

void CAN_tx_canload_msg() {
    CAN_comms_Tx_msg_t CAN_comms_Tx_msg = {
        .data[0] = (uint8_t) CANLOAD_get_bus_load(),
        .header = CANLOAD_busload
    };  

  CANLOAD_calculate_message_bits(CAN_comms_Tx_msg.header.DLC, CAN_comms_Tx_msg.header.IDE);
  CAN_comms_Add_Tx_message(&CAN_comms_Tx_msg);
}

void CAN_cpu_load_can_tx(){
   CAN_comms_Tx_msg_t CPU_LOAD_can_tx = {0};
   FloatToBytes CPU_LOAD;

   CPU_LOAD.f = CPU_LOAD_average();

   CPU_LOAD_can_tx.header = cpu_load_can_header;
   CPU_LOAD_can_tx.data[0] = CPU_LOAD.bytes[0];
   CPU_LOAD_can_tx.data[1] = CPU_LOAD.bytes[1];
   CPU_LOAD_can_tx.data[2] = CPU_LOAD.bytes[2];
   CPU_LOAD_can_tx.data[3] = CPU_LOAD.bytes[3];
  
  CAN_comms_Add_Tx_message(&CPU_LOAD_can_tx);
}



/* USER CODE END 1 */
