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
#include "rtc.h"
#include "main.h"

HAL_StatusTypeDef can_start;

CAN_TxHeaderTypeDef tel_diagnostics_header = {
    .StdId = TEL_DIAGNOSTICS_ID,
    .ExtId = 0x0000,
    .IDE = CAN_ID_STD,
    .RTR = CAN_RTR_DATA,
    .DLC = 1};

CAN_TxHeaderTypeDef IMU_x_axis_header = {
    .StdId = IMU_X_AXIS,
    .ExtId = 0x0000,
    .IDE = CAN_ID_STD,
    .RTR = CAN_RTR_DATA,
    .DLC = CAN_DATA_LENGTH};

CAN_TxHeaderTypeDef IMU_y_axis_header = {
    .StdId = IMU_Y_AXIS,
    .ExtId = 0x0000,
    .IDE = CAN_ID_STD,
    .RTR = CAN_RTR_DATA,
    .DLC = CAN_DATA_LENGTH};

CAN_TxHeaderTypeDef IMU_z_axis_header = {
    .StdId = IMU_Z_AXIS,
    .ExtId = 0x0000,
    .IDE = CAN_ID_STD,
    .RTR = CAN_RTR_DATA,
    .DLC = CAN_DATA_LENGTH};

CAN_TxHeaderTypeDef GPS_latitude_header = {
    .StdId = GPS_latitude_ID,
    .ExtId = 0x0000,
    .IDE = CAN_ID_STD,
    .RTR = CAN_RTR_DATA,
    .DLC = CAN_DATA_LENGTH};

CAN_TxHeaderTypeDef GPS_longitude_header = {
    .StdId = GPS_longitude_ID,
    .ExtId = 0x0000,
    .IDE = CAN_ID_STD,
    .RTR = CAN_RTR_DATA,
    .DLC = CAN_DATA_LENGTH};

CAN_TxHeaderTypeDef GPS_altitude_hdop_header = {
    .StdId = GPS_altitude_hdop_ID,
    .ExtId = 0x0000,
    .IDE = CAN_ID_STD,
    .RTR = CAN_RTR_DATA,
    .DLC = CAN_DATA_LENGTH};

CAN_TxHeaderTypeDef GPS_side_count_header = {
    .StdId = GPS_side_count_ID,
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

void CanFilterSetup(void)
{
  /* TODO: Review Filter Implementation */
  // Use mask and list mode to filter IDs from the CAN ID BOM

  // Filter for 0x500 and 0x600 IDs
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


/*
 * CAN set-up: Sets up the filters, Starts CAN with HAL, and Activates notifications for interrupts.
 */
void CAN_Init(void)
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
  CAN_msg_t *new_CAN_msg;


  /* Get CAN message */
    HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &can_rx_header, can_data);  // TODO: Put can_rx_header and can_data into a data structure able to be accessed in the freertos task
    /* Put CAN message in the Queue */
    new_CAN_msg = osPoolAlloc(CAN_MSG_memory_pool);
    new_CAN_msg->header = can_rx_header;
    for(int i = 0; i < CAN_DATA_LENGTH; i++) {
      new_CAN_msg->data[i] = can_data[i];
    }

    /* Perform rtc syncing check if the message is 0x300 and if RTC is reset to 2000-01-01 */
    if (new_CAN_msg->header.StdId == RTC_TIMESTAMP_MSG_ID)
    {
      start_of_second = HAL_GetTick();
    }

    new_CAN_msg->timestamp.double_value = get_current_timestamp();
    osMessagePut(CAN_MSG_Rx_Queue, new_CAN_msg, osWaitForever);

  /* Set the Flag to CAN_READY */
  osSignalSet(readCANTaskHandle, CAN_READY);

  /* To avoid warning of unused variable */
}


/**
 * @brief Function to transmit the TEL diagnostic message containing rtc reset, gps fix, imu fail, and watchdog reset flags
 * @return void
 */
void CAN_diagnostic_msg_tx_radio_bus() {
  uint8_t data_send = INITIAL_FLAGS;
  CAN_Radio_msg_t diagnostics_msg;
  diagnostics_msg.header = tel_diagnostics_header;
  union Utils_DoubleBytes_t current_timestamp;

  current_timestamp.double_value = get_current_timestamp();
  diagnostics_msg.timestamp = current_timestamp;

  if(g_tel_diagnostics.rtc_reset) 
    SET_BIT(data_send, FLAG_HIGH << 0);
  if(g_tel_diagnostics.gps_fix)
    SET_BIT(data_send, FLAG_HIGH << 1);
  if(g_tel_diagnostics.imu_fail)
    SET_BIT(data_send, FLAG_HIGH << 2);
  if(g_tel_diagnostics.watchdog_reset)
    SET_BIT(data_send, FLAG_HIGH << 3);
  
  diagnostics_msg.data[FIRST_DATA_BYTE] = data_send;

  CAN_radio_and_bus_transmit(&hcan, &diagnostics_msg, &can_mailbox);
}



/**
 * @brief Function to transmit CAN message on the CAN bus as well as over radio
 * @param hcan The CAN handle
 * @param tx_CAN_msg The CAN message to be transmitted
 * @param can_mailbox The CAN mailbox
 * @return void
*/
void CAN_radio_and_bus_transmit(CAN_HandleTypeDef* hcan, CAN_Radio_msg_t* tx_CAN_msg, uint32_t* can_mailbox) {
    HAL_CAN_AddTxMessage(hcan, &(tx_CAN_msg->header), tx_CAN_msg->data, can_mailbox);
    RADIO_tx_CAN_msg(tx_CAN_msg);
}


/**
 * @brief Function to convert a CAN_msg_t to a CAN_Radio_msg_t
 * @param rx_CAN_msg The received CAN message
 * @param tx_CAN_msg The CAN message to be transmitted
 */
void CAN_rx_to_radio(CAN_msg_t* rx_CAN_msg, CAN_Radio_msg_t* tx_CAN_msg) {
  // Do the header fields
  tx_CAN_msg->header.StdId = rx_CAN_msg->header.StdId;
  tx_CAN_msg->header.ExtId = rx_CAN_msg->header.ExtId;
  tx_CAN_msg->header.IDE = rx_CAN_msg->header.IDE;
  tx_CAN_msg->header.RTR = rx_CAN_msg->header.RTR;
  tx_CAN_msg->header.DLC = rx_CAN_msg->header.DLC;

  // Do the data field
  for (int i = 0; i < 8; i++) {
    tx_CAN_msg->data[i] = rx_CAN_msg->data[i];
  }

  // Do the timestamp field
  tx_CAN_msg->timestamp = rx_CAN_msg->timestamp;  
}


/**
 * @brief Function to handle received CAN message on queue
 * @param rx_CAN_msg The received CAN message
 * @return void
 */
void CAN_handle_rx_msg(CAN_msg_t* rx_CAN_msg) {
  HAL_GPIO_TogglePin(USER_LED_GPIO_Port, USER_LED_Pin);   // Blink LED to indicate CAN message received

  // RTC_check_and_sync_rtc(rx_CAN_msg);                     // Sync RTC with memorator message. Also sets rtc reset

  CAN_Radio_msg_t tx_CAN_msg;
  CAN_rx_to_radio(rx_CAN_msg, &tx_CAN_msg);               // Convert CAN message to radio message
  RADIO_tx_CAN_msg(&tx_CAN_msg);                          // Send CAN on radio

  osPoolFree(CAN_MSG_memory_pool, rx_CAN_msg);            // Free can msg from pool
}

/* USER CODE END 1 */