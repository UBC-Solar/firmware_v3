/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
int timerInterupt = 0;

typedef struct
{

	//0x501
	uint8_t motorCurrentFlag;
	uint8_t velocityFlag;
	uint8_t busCurrentFlag;
	uint8_t busVoltageUpperLimitFlag;
	uint8_t busVoltageLowerLimitFlag;
	uint8_t heatsinkTemp;
	uint8_t hardwareOverCurrent;
	uint8_t softwareOverCurrent;
	uint8_t DCBusOverVoltage;
	//0x502
	uint8_t busVoltage;
	uint8_t busCurrent;
	//0x503
	uint8_t motorVelocity;
	uint8_t vehicleVelocity;
	//0x504
	uint8_t motorTemp;
	uint8_t controllerHeatsinkTemp;

} CAN_message_t;
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
CAN_HandleTypeDef hcan;

TIM_HandleTypeDef htim2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);
static void MX_CAN_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
CAN_TxHeaderTypeDef TxHeader; //struct with outgoing message information (type and length, etc.)
CAN_RxHeaderTypeDef RxHeader; //struct with incoming message information
uint32_t TxMailbox[3]; //3 transmit mailboxes

uint8_t TxData[8];
uint8_t RxData[8];

CAN_message_t testMessage;

void get501(uint8_t* message501);

uint8_t getBit(uint8_t lsb, uint8_t one, uint8_t two, uint8_t three,
		  uint8_t four, uint8_t five, uint8_t six, uint8_t msb);

int setCANFlag;
int txIDList[] = {0x501, 0x502, 0x503, 0x50B};
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

	testMessage.motorCurrentFlag = 1;
	testMessage.velocityFlag = 1;
	testMessage.busCurrentFlag = 1;
	testMessage.busVoltageUpperLimitFlag = 1;
	testMessage.busVoltageLowerLimitFlag = 1;
	testMessage.heatsinkTemp = 1;
	testMessage.hardwareOverCurrent = 1;
	testMessage.softwareOverCurrent = 1;
	testMessage.DCBusOverVoltage = 1;
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM2_Init();
  MX_CAN_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_Base_Start_IT(&htim2);
  HAL_CAN_Start(&hcan);
  HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING);

  TxHeader.DLC = 8;
  TxHeader.ExtId = 0;
  TxHeader.IDE = CAN_ID_STD;
  TxHeader.RTR = CAN_RTR_DATA;
  TxHeader.StdId = 0x501;
  TxHeader.TransmitGlobalTime = DISABLE;

  CAN_message_t testMessage;

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1){
	  if(timerInterupt == 1){
		  timerInterupt = 0;
		  uint8_t getTxData[8];

		  //txData 0x501
		  TxHeader.StdId = txIDList[0];
		  get501(getTxData);
		  for(int i = 0; i < 8; i++){
			  TxData[i] = getTxData[i];
		  }
		  HAL_CAN_AddTxMessage(&hcan, &TxHeader, TxData, TxMailbox);

		  //txData 0x502
		  txIDList[1] = TxHeader;
		  HAL_CAN_AddTxMessage(&hcan, TxHeader, TxData, TxMailbox);

		  //txData 0x503
		  txIDList[2] = TxHeader;
		  HAL_CAN_AddTxMessage(&hcan, TxHeader, TxData, TxMailbox);

		  //txData 0x50B
		  txIDList[3] = TxHeader;
		  HAL_CAN_AddTxMessage(&hcan, TxHeader, TxData, TxMailbox);
		  }

	  //send out message for test only
	  if(setCANFlag == 1){
		  setCANFlag = 0;
		  HAL_CAN_GetRxMessage(&hcan, CAN_RX_FIFO0, &RxHeader, RxData);
	  }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief CAN Initialization Function
  * @param None
  * @retval None
  */
static void MX_CAN_Init(void)
{

  /* USER CODE BEGIN CAN_Init 0 */

  /* USER CODE END CAN_Init 0 */

  /* USER CODE BEGIN CAN_Init 1 */

  /* USER CODE END CAN_Init 1 */
  hcan.Instance = CAN1;
  hcan.Init.Prescaler = 1;
  hcan.Init.Mode = CAN_MODE_LOOPBACK;
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

  CAN_FilterTypeDef canfilterconfig;

  canfilterconfig.FilterActivation = ENABLE;
  canfilterconfig.FilterBank = 10;
  canfilterconfig.FilterFIFOAssignment = CAN_RX_FIFO0;
  canfilterconfig.FilterIdHigh = 0x501<<5;
  canfilterconfig.FilterIdLow = 0x0000;
  canfilterconfig.FilterMaskIdHigh = 0;
  canfilterconfig.FilterMaskIdLow = 0x0000;
  canfilterconfig.FilterMode = CAN_FILTERMODE_IDMASK;
  canfilterconfig.FilterScale = CAN_FILTERSCALE_32BIT;
  canfilterconfig.SlaveStartFilterBank = 13;

  HAL_CAN_ConfigFilter(&hcan, &canfilterconfig);
  /* USER CODE END CAN_Init 2 */
}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 1000;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 1600;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);

  /*Configure GPIO pin : PA5 */
  GPIO_InitStruct.Pin = GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void get501(uint8_t* message501){

	  uint8_t message64[64];

	  message64[0] = 0;
	  message64[1] = testMessage.motorCurrentFlag;
	  message64[2] = testMessage.velocityFlag;
	  message64[3] = testMessage.busCurrentFlag;
	  message64[4] = testMessage.busVoltageUpperLimitFlag;
	  message64[5] = testMessage.busVoltageLowerLimitFlag;
	  message64[6] = testMessage.heatsinkTemp;
	  for (int i = 7; i <= 15; i++){
		  message64[i] = 0;
	  }
	  message64[16] = testMessage.hardwareOverCurrent;
	  message64[17] = testMessage.softwareOverCurrent;
	  message64[18] = testMessage.DCBusOverVoltage;
	  for (int i = 19; i <= 63; i++){
				  message64[i] = 0;
	  }

	  message501[0] = getBit(message64[0], message64[1], message64[2], message64[3],
					  message64[4], message64[5], message64[6], message64[7]);

	  message501[1] = getBit(message64[8], message64[9], message64[10], message64[11],
					  message64[12], message64[13], message64[14], message64[15]);

	  message501[2] = getBit(message64[16], message64[17], message64[18], message64[19],
					  message64[20], message64[21], message64[22], message64[23]);

	  message501[3] = getBit(message64[24], message64[25], message64[26], message64[27],
					  message64[28], message64[29], message64[30], message64[31]);

	  message501[4] = getBit(message64[32], message64[33], message64[34], message64[35],
					  message64[36], message64[37], message64[38], message64[39]);

	  message501[5] = getBit(message64[40], message64[41], message64[42], message64[43],
					  message64[44], message64[45], message64[46], message64[47]);

	  message501[6] = getBit(message64[48], message64[49], message64[50], message64[51],
					  message64[52], message64[53], message64[54], message64[55]);

	  message501[7] = getBit(message64[56], message64[57], message64[58], message64[59],
					  message64[60], message64[61], message64[62], message64[63]);

}

uint8_t getBit(uint8_t msb, uint8_t two, uint8_t three, uint8_t four,
		  uint8_t five, uint8_t six, uint8_t seven, uint8_t lsb){
	uint8_t byte = (msb * 128 + two * 64 + three * 32 + four * 16
			  + five * 8 + six * 4 + seven * 2 + lsb * 1);
	if(byte == 255 && msb == 0){
		byte = 0;
	}
	 return byte;
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	timerInterupt = 1;
}

//Interrupt where code will go when it recieves a CAN message.
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) //recieve data in this function
{
  //gets the CAN message with info in RxHeader and data in RxData
	if(HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, RxData) != HAL_OK)
	{
		Error_Handler();
	}
  //sets a flag to let the main loop know a message has been recieved
	setCANFlag = 1;
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
