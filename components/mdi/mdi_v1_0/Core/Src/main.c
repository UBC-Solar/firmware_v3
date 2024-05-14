/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "can.h"
#include "Cruise_Control.h"
#include "MDI_Helper_Functions.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define DAC_ACC_ADDR    (0b0001110 << 1) //I2C address for the acceleration DAC
#define DAC_REGEN_ADDR  (0b0001101 << 1) //I2C address for the regen DAC
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
CAN_HandleTypeDef hcan;

I2C_HandleTypeDef hi2c2;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim6;
TIM_HandleTypeDef htim7;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_CAN_Init(void);
static void MX_I2C2_Init(void);
static void MX_TIM6_Init(void);
static void MX_TIM1_Init(void);
static void MX_TIM7_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

CAN_TxHeaderTypeDef TxHeader; //struct with outgoing message information (type and length, etc.)
CAN_RxHeaderTypeDef RxHeader; //struct with incoming message information
uint32_t TxMailbox[3]; //3 transmit mailboxes

uint8_t TxData[8]; //buffer for transmit message
uint8_t RxData[8]; //buffer for receive message
uint8_t getTxData[8];

uint8_t localRxDataFrame0[8];
uint8_t localRxDataFrame1[8];
uint8_t localRxDataFrame2[8];
uint8_t localRxDataMessage401[8];

CAN_message_t msg0; //where all the info and data for the message will be put

int txIDList[] = {0x501, 0x502, 0x503, 0x50B}; //array for the messages to be sent 

//Flags Initialization
bool SendMessageTimerInterrupt = 0;
bool SendSlowMessage = 0;
bool UpdateSpeedInterrupt = 0;
bool send_data_flag = 0; 
bool Parse_Data_Flag = 0; 
bool parse_frame0 = 0;
bool parse_frame1 = 0;
bool parse_frame2 = 0;
bool TriggerBlinky = 0;


//Variables for HALL speed calculations
uint32_t oldTime = 0;
uint32_t currentTime = 0;
float rpm = 0;
float parsed_acceleration = 0;
uint8_t countSlowTimerInterrupt = 0; //counter to send the slower messages

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  uint8_t RxDataLocal[8];
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
  MX_CAN_Init();
  MX_I2C2_Init();
  MX_TIM6_Init();
  MX_TIM1_Init();
  MX_TIM7_Init();
  /* USER CODE BEGIN 2 */
  HAL_CAN_Start(&hcan); //starts CAN
  HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING); //FIFO0 message pending interrupt (interrupt callback func will be called when this interrupt is triggered)

  HAL_TIM_Base_Start_IT(&htim6); //timer for sending messages
  HAL_TIM_Base_Start_IT(&htim1); //timer for updating speed coming from HALL sensor
  HAL_TIM_Base_Start_IT(&htim7); //WATCHDOG timer to reset rpm to 0 when motor stops spinning

  TxHeader.DLC = 8;
  TxHeader.IDE = CAN_ID_EXT; //type of id being sent ext or simple
  TxHeader.RTR = CAN_RTR_DATA; 
  TxHeader.ExtId = 0x08F89540; //request frame ExtId
  TxHeader.TransmitGlobalTime = DISABLE;

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  //BOOT LED Initial Value
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);

  while (1)
  {
	////////////////
	// BLINKY LED //
	////////////////
	if(TriggerBlinky == 1){
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);
	}
	else {
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);
	}
    /////////////////////////////////////////////////////////
    // PARSE MESSAGES COMING FROM MOTOR CONTROLLER AND MCB //
    /////////////////////////////////////////////////////////
    if(Parse_Data_Flag == 1 ) //Parse_Data_Flag set in interrupt
    {
      CopyRxData(localRxDataMessage401,RxDataLocal);
      CAN_Decode_Velocity_Message(RxDataLocal, &msg0); //Decode the Incoming 0x401 Messages to get data for driving the car

      Parse_Data_Flag = 0; //reset parse flag
      send_data_flag = 1; //send updated data to motor flag

    }
    
    if(parse_frame0 == 1) //Parse the frame based on info we want
    {
    	CopyRxData(localRxDataFrame0,RxDataLocal);
    	Decode_Frame0(RxDataLocal,&msg0);
    	parse_frame0 = 0; //reset flag
    }
    
    if(parse_frame1 == 1) //Parse the frame based on info we want (currently this frame doesn't have info we care about)
    {
    	CopyRxData(localRxDataFrame1,RxDataLocal);
    	parse_frame1 = 0; //reset flag
    }

    if(parse_frame2 == 1) //Parse the frame based on info we want
    {
    	CopyRxData(localRxDataFrame2,RxDataLocal);
    	Decode_Frame2(RxData,&msg0);
    	parse_frame2 = 0; //reset flag
    }

    //////////////////////////////////////
    // SEND SIGNALS TO MOTOR CONTROLLER //
    //////////////////////////////////////
    if( (send_data_flag == 1 ) && (Parse_Data_Flag == 0) )
    {
    	//send direction
    	//According to data sheet: OPEN Switch = Forward / CLOSED Switch = Reverse
    	//GPIO Must Be Open Drain
    	if(msg0.FWD_direction == FORWARD_FALSE)
    		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET);
    	else
    		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET);

    	 //send power/eco
    	 //According to data sheet: OPEN Switch = ECO_MODE / CLOSED Switch = POWER MODE
    	 //GPIO Must Be Open Drain
    	 if(msg0.PWR_mode_on == POWER_MODE_ON)
    		 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
    	 else
    		 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);

      //send acceleration current to motor
      if(msg0.regen == REGEN_TRUE) {
        Send_Voltage(msg0.acceleration, DAC_REGEN_ADDR, &hi2c2);
      }
      else{
        parsed_acceleration = msg0.acceleration;
        Send_Voltage(parsed_acceleration, DAC_ACC_ADDR, &hi2c2);
      }

      //reset flag
      send_data_flag = 0;
    }

    /////////////////////////////
    //// SEND MESSAGE TO MCB ////
    /////////////////////////////

    if(SendMessageTimerInterrupt == 1)
    {
    	//send request for data from MC
    	TxHeader.IDE = CAN_ID_EXT; //type of id being sent ext or simple
    	TxHeader.ExtId = 0x08F89540; //request frame ExtId
    	//Send_Test_Message(TxData, 7, 7); //request all frames
    	Send_Test_Message(TxData, 5, 5); //request frame 0 and 2
    	HAL_CAN_AddTxMessage(&hcan, &TxHeader, TxData, TxMailbox);

    	//txData 0x501
    	TxHeader.StdId = txIDList[0];
    	TxHeader.IDE = CAN_ID_STD; //type of id being sent ext or simple
    	get501(getTxData, msg0);
    	for(int i = 0; i < 8; i++){
    		TxData[i] = getTxData[i];
    	}
    	HAL_CAN_AddTxMessage(&hcan, &TxHeader, TxData, TxMailbox);
      
    	//txData 0x502
    	TxHeader.StdId = txIDList[1];
    	TxHeader.IDE = CAN_ID_STD; //type of id being sent ext or simple
    	get502(getTxData, msg0);
    	for(int i = 0; i < 8; i++){
    		TxData[i] = getTxData[i];
    	}
    	HAL_CAN_AddTxMessage(&hcan, &TxHeader, TxData, TxMailbox);
      
    	//need to wait for mailbox to clear before sending msg
    	while(HAL_CAN_GetTxMailboxesFreeLevel(&hcan) < 1 ){};

    	//txData 0x503
    	TxHeader.StdId = txIDList[2];
    	TxHeader.IDE = CAN_ID_STD; //type of id being sent ext or simple
    	get503(getTxData, msg0);
    	for(int i = 0; i < 8; i++){
    		TxData[i] = getTxData[i];
    	}
    	HAL_CAN_AddTxMessage(&hcan, &TxHeader, TxData, TxMailbox);
      
    	//need to wait for mailbox to clear before sending msg
    	while(HAL_CAN_GetTxMailboxesFreeLevel(&hcan) < 1 ){}; //very sketch this was added by BEN :) - P.S Mischa dont kill me

    	if(SendSlowMessage == 1)
    	{
    		//txData 0x50B
    		TxHeader.StdId = txIDList[3];
    		TxHeader.IDE = CAN_ID_STD; //type of id being sent ext or simple
    		get50B(getTxData, msg0);
    		for(int i = 0; i < 8; i++){
    			TxData[i] = getTxData[i];
    		}
    		HAL_CAN_AddTxMessage(&hcan, &TxHeader, TxData, TxMailbox);
    		SendSlowMessage = 0;
      }
    	SendMessageTimerInterrupt = 0;
    }
    ///////////////////////////////
    //// HALL SPEED CONVERSION ////
    ///////////////////////////////

    if(UpdateSpeedInterrupt == 1)
    {
      UpdateSpeedInterrupt = 0; //reset flag

      rpm = 60.0 / ((currentTime - oldTime) * 0.001); //get RPM

      msg0.motorVelocity = rpm;
      msg0.vehicleVelocity = (WHEEL_RADIUS * 2.0 * M_PI * rpm) / 60.0 ;  //linear speed = radius x angular speed = r*2*π*(RPM)/60
    }
  }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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
  hcan.Init.Prescaler = 2;
  hcan.Init.Mode = CAN_MODE_NORMAL;
  hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan.Init.TimeSeg1 = CAN_BS1_4TQ;
  hcan.Init.TimeSeg2 = CAN_BS2_3TQ;
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

  //Code for setting up filters
  CAN_FilterTypeDef canfilterconfig; //struct that contains all the info for filters
  canfilterconfig.FilterActivation = CAN_FILTER_ENABLE;  //ENABLE
  canfilterconfig.FilterBank = 0;                       //Current filter bank to use
  canfilterconfig.FilterFIFOAssignment = CAN_RX_FIFO0;   //Using Fifo0 interrupt
  canfilterconfig.FilterIdHigh = 0x401<<5;               //Assuming we are only filtering for 0x401 message
  canfilterconfig.FilterIdLow = 0x0000;
  canfilterconfig.FilterMaskIdHigh = 0; //changed from 0x1<<13
  canfilterconfig.FilterMaskIdLow = 0x0000;
  canfilterconfig.FilterMode = CAN_FILTERMODE_IDMASK;    //Masking mode
  canfilterconfig.FilterScale = CAN_FILTERSCALE_32BIT;   
  canfilterconfig.SlaveStartFilterBank = 13; // tells where to start giving the slave filters
  HAL_CAN_ConfigFilter(&hcan, &canfilterconfig);         //sets up the filter according the info in canfilterconfig
  
  //frame0
  canfilterconfig.FilterBank = 1;                       //Current filter bank to use
  canfilterconfig.FilterIdHigh = 0x08850225;
  canfilterconfig.FilterIdLow = 0x0000;
  HAL_CAN_ConfigFilter(&hcan, &canfilterconfig);         //sets up the filter according the info in canfilterconfig
  
  //frame1
  canfilterconfig.FilterBank = 2;                       //Current filter bank to use
  canfilterconfig.FilterIdHigh = 0x08950225;
  canfilterconfig.FilterIdLow = 0x0000;
  HAL_CAN_ConfigFilter(&hcan, &canfilterconfig);         //sets up the filter according the info in canfilterconfig
  
  //frame2
  canfilterconfig.FilterBank = 3;                       //Current filter bank to use
  canfilterconfig.FilterIdHigh = 0x08A50225;
  canfilterconfig.FilterIdLow = 0x0000;
  HAL_CAN_ConfigFilter(&hcan, &canfilterconfig);         //sets up the filter according the info in canfilterconfig


  /* USER CODE END CAN_Init 2 */
}

/**
  * @brief I2C2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C2_Init(void)
{

  /* USER CODE BEGIN I2C2_Init 0 */

  /* USER CODE END I2C2_Init 0 */

  /* USER CODE BEGIN I2C2_Init 1 */

  /* USER CODE END I2C2_Init 1 */
  hi2c2.Instance = I2C2;
  hi2c2.Init.ClockSpeed = 100000;
  hi2c2.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C2_Init 2 */

  /* USER CODE END I2C2_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 0;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 47;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_ETRMODE2;
  sClockSourceConfig.ClockPolarity = TIM_CLOCKPOLARITY_NONINVERTED;
  sClockSourceConfig.ClockPrescaler = TIM_CLOCKPRESCALER_DIV1;
  sClockSourceConfig.ClockFilter = 15;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */

}

/**
  * @brief TIM6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM6_Init(void)
{

  /* USER CODE BEGIN TIM6_Init 0 */

  /* USER CODE END TIM6_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM6_Init 1 */

  /* USER CODE END TIM6_Init 1 */
  htim6.Instance = TIM6;
  htim6.Init.Prescaler = 8000-1;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 200-1;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM6_Init 2 */

  /* USER CODE END TIM6_Init 2 */

}

/**
  * @brief TIM7 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM7_Init(void)
{

  /* USER CODE BEGIN TIM7_Init 0 */

  /* USER CODE END TIM7_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM7_Init 1 */

  /* USER CODE END TIM7_Init 1 */
  htim7.Instance = TIM7;
  htim7.Init.Prescaler = 8000-1;
  htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim7.Init.Period = 1000-1;
  htim7.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim7) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim7, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM7_Init 2 */

  /* USER CODE END TIM7_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(Boot_LED_GPIO_Port, Boot_LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, ECO_Pin|DIR_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : Boot_LED_Pin */
  GPIO_InitStruct.Pin = Boot_LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(Boot_LED_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : ECO_Pin DIR_Pin */
  GPIO_InitStruct.Pin = ECO_Pin|DIR_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

//Interrupt where code will go when it receives a CAN message.
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) //receive data in this function
{

  //gets the CAN message with info in RxHeader and data in RxData
	if(HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, RxData) != HAL_OK)
	{
		Error_Handler();
	}

  //sets a flag to let the main loop know a message has been received
	if(RxHeader.IDE == 4){ //we received and ExtId
		switch(RxHeader.ExtId){
			case (0x08850225): //frame 0
					parse_frame0 = 1;
				CopyRxData(RxData, localRxDataFrame0);
				break;
			case (0x08950225): //frame 1
					parse_frame1 = 1;
				CopyRxData(RxData, localRxDataFrame1);
				break;
			case (0x08A50225): //frame 2
				parse_frame2 = 1;
				CopyRxData(RxData, localRxDataFrame2);
				break;
			default:
				break;
		}
	}
	else{ //we received and StdId message
		if(RxHeader.StdId == 0x401){ //0x401 velocity message
			Parse_Data_Flag = 1;
			CopyRxData(RxData, localRxDataMessage401);
		}
	}


}

// Callback: timer has rolled over
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  // Check which version of the timer triggered this callback and toggle LED
  if (htim == &htim6 )
  {
	  SendMessageTimerInterrupt = 1;

	  countSlowTimerInterrupt++;
	  if(countSlowTimerInterrupt == 5) //send 0x50B message every second (Interrupt triggers every 0.2 s)
	  {
		  SendSlowMessage = 1;
		  TriggerBlinky ^= 1;
		  countSlowTimerInterrupt = 0;
	  }
  }

  if(htim == &htim1){
	  oldTime = currentTime;
	  currentTime = HAL_GetTick();
	  UpdateSpeedInterrupt = 1;

	  TIM7 -> CNT = 0; //reset WATCHDOG timer to 0
  }

  if(htim == &htim7){
	  rpm = 0; //if we enter WATCHDOG timer interrupt we must have stopped spinning the motor
	  msg0.motorVelocity = 0;
	  msg0.vehicleVelocity = 0;  
  }
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
