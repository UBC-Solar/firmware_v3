/**
  ******************************************************************************
  * @file    can.c
  * @brief   This file provides code for the configuration
  *          of the CAN instances and CAN filters.
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "can.h"

/* USER CODE BEGIN 0 */

/**
 * 	CAN message header for a drive command. This command header is used in normal, regen, and
 * 	cruise control mode to send appropriate drive command to the motor controller.
 */
CAN_TxHeaderTypeDef drive_command_header = {
    .StdId = DRIVER_CONTROLS_BASE_ADDRESS + 1,
    .ExtId = 0x0000,
    .IDE = CAN_ID_STD,
    .RTR = CAN_RTR_DATA,
    .DLC = CAN_DATA_LENGTH};

/**
 * 	CAN message header for a next screen command. This command header is used to indicate to the
 * 	driver information display (DID) board that the next screen button has been pressed.
 */
CAN_TxHeaderTypeDef screen_cruise_control_header = {
    .StdId = DRIVER_CONTROLS_BASE_ADDRESS + 3,
    .ExtId = 0x0000,
    .IDE = CAN_ID_STD,
    .RTR = CAN_RTR_DATA,
    .DLC = CAN_CONTROL_DATA_LENGTH};

CAN_RxHeaderTypeDef can_rx_header;                  /**< Stores the header information for CAN messages read from
													     the RX (receive) buffer */

CAN_FilterTypeDef battery_soc_filter;               /**< Stores the intialisation information for the hardware CAN
													     that filters for battery (ID 0x626) messages */

uint32_t can_mailbox;                               /**< Stores the TX (transmit) mailbox that a CAN message has
													     been placed into.*/

/* USER CODE END 0 */

CAN_HandleTypeDef hcan; /**< CAN peripheral handle. */

/**
 *  @brief 	CAN initialization function.
 *  */
void MX_CAN_Init(void)
{
    hcan.Instance = CAN1;
    hcan.Init.Prescaler = 16;
    hcan.Init.Mode = CAN_MODE_NORMAL;
    hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
    hcan.Init.TimeSeg1 = CAN_BS1_13TQ;
    hcan.Init.TimeSeg2 = CAN_BS2_4TQ;
    hcan.Init.TimeTriggeredMode = ENABLE;
    hcan.Init.AutoBusOff = ENABLE;
    hcan.Init.AutoWakeUp = ENABLE;
    hcan.Init.AutoRetransmission = DISABLE;
    hcan.Init.ReceiveFifoLocked = DISABLE;
    hcan.Init.TransmitFifoPriority = DISABLE;
    if (HAL_CAN_Init(&hcan) != HAL_OK)
    {
        Error_Handler();
    }
}

void HAL_CAN_MspInit(CAN_HandleTypeDef *canHandle)
{

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if (canHandle->Instance == CAN1)
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
    }
}

void HAL_CAN_MspDeInit(CAN_HandleTypeDef *canHandle)
{

    if (canHandle->Instance == CAN1)
    {
        /* USER CODE BEGIN CAN1_MspDeInit 0 */

        /* USER CODE END CAN1_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_CAN1_CLK_DISABLE();

        /**CAN GPIO Configuration
    PB8     ------> CAN_RX
    PB9     ------> CAN_TX
    */
        HAL_GPIO_DeInit(GPIOB, GPIO_PIN_8 | GPIO_PIN_9);

        /* USER CODE BEGIN CAN1_MspDeInit 1 */

        /* USER CODE END CAN1_MspDeInit 1 */
    }
}

/**
 * 	@brief 	Initializes a hardware CAN filter that filters for CAN messages that have the standard ID 0x626.
 * 			This corresponds to a message from the BMS (battery management system) that contains the battery SOC.
 */
void CAN_Filter_Init(void)
{
    // in this case we are using two 16-bit filters in identifer mask mode
    // therefore, the high and low values for the FilterID and FilterMask are going to be the same since
    // we are currently only filtering for one ID (0x626)
    battery_soc_filter.FilterIdHigh = (uint32_t)((BATTERY_BASE + 6) << 5);
    battery_soc_filter.FilterIdLow = (uint32_t)((BATTERY_BASE + 6) << 5);

    // masks away the last 5 bits of CAN message - the only relevant bits are [15:5] (11-bit standard identifier)
    battery_soc_filter.FilterMaskIdHigh = (uint32_t)(0x7FF << 5);
    battery_soc_filter.FilterMaskIdLow = (uint32_t)(0x7FF << 5);

    battery_soc_filter.FilterFIFOAssignment = CAN_FILTER_FIFO0;
    battery_soc_filter.FilterBank = (uint32_t)0;
    battery_soc_filter.FilterMode = CAN_FILTERMODE_IDMASK;
    battery_soc_filter.FilterScale = CAN_FILTERSCALE_16BIT;
    battery_soc_filter.FilterActivation = CAN_FILTER_ENABLE;
}

/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
