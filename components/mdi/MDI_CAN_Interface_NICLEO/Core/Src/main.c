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
#include "math.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
CAN_HandleTypeDef hcan;

I2C_HandleTypeDef hi2c1;

/* USER CODE BEGIN PV */
static const uint8_t DAC_ADDR = 0b0001100 << 1;
static const uint8_t DAC_REGEN_ADDR = 0b0001101 << 1;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_CAN_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

CAN_TxHeaderTypeDef TxHeader; //struct with outgoing message information (type and length, etc.)
CAN_RxHeaderTypeDef RxHeader; //struct with incoming message information
uint32_t TxMailbox[3]; //4 transmit mailboxes
//CAN_FilterTypeDef fltr;

uint8_t TxData[8]; //buffer for transmit message
uint8_t RxData[8]; //buffer for receive message

CAN_msg_t msg0; //where all the info and data for the message will be put


bool send_data_flag = 0; 
bool Parse_Data_Flag = 0; 

uint16_t parsed_voltage = 0; 

int32_t velocity; //FOR TESTING
uint32_t acceleration; //FOR TESTING 
uint64_t t_start; //FOR TESTING
uint64_t t_end; //FOR TESTING
uint64_t t_elapsed[256]; //FOR TESTING
uint8_t count_t = 0; //FOR TESTING
float count = 0; //FOR TESTING


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
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
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
  HAL_CAN_Start(&hcan); //starts CAN
  HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING); //FIFO0 message pending interrupt (interrupt callback func will be called when this interrupt is triggered)

  TxHeader.DLC = 8;
  TxHeader.ExtId = 0;
  TxHeader.IDE = CAN_ID_STD;
  TxHeader.RTR = CAN_RTR_DATA; 
  TxHeader.StdId = 0x401; 
  TxHeader.TransmitGlobalTime = DISABLE;

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  while (1)
  {
    
    //////////////////TEST MESSAGE GENERATION//////////////////////
      msg0.power_or_eco = POWER_ON; //this would come from switch
     
      t_end = HAL_GetTick();

      count_t++; 

      if(count_t == 3) {
        count = count + 0.1;
        count_t = 0; 
      } 

     if(count > 99999.0)
      count = 0; 

     velocity = -100; 
     acceleration = 0xFFFFFFFF;
     acceleration = acceleration/2.0*sin(count)  + acceleration/2.0 ; 
     send_test_message(TxData, velocity, acceleration); 

     HAL_CAN_AddTxMessage(&hcan, &TxHeader, TxData, TxMailbox);
    //////////////////////////////////////////////////////////////////
    
    if(Parse_Data_Flag == 1 ){ //Parse_Data_Flag set in interrupt

      decode_CAN_velocity_message(RxData, &msg0); 

      Parse_Data_Flag = 0; 
      send_data_flag = 1; 
    }
    
    if( (send_data_flag == 1 ) && (Parse_Data_Flag == 0) ){

      if(msg0.regen == REGEN_TRUE)
        Send_Regen(msg0.acceleration, DAC_REGEN_ADDR, &hi2c1); 
      else{
        parsed_voltage = Parse_ACC(msg0.acceleration);
    	  Send_Voltage(parsed_voltage, DAC_ADDR, &hi2c1); 
      }
      //send direction
      if(msg0.direction == REVERSE_FALSE)
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_SET); 
      else 
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_RESET);
      //send power/eco
      if(msg0.power_or_eco == ECO_ON)
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_SET); 
      else 
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_RESET);

	    send_data_flag = 0; 
      

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
  hcan.Init.Prescaler = 16;
  hcan.Init.Mode = CAN_MODE_LOOPBACK;   //Loopback for testing, Normal for actual
  //Time settings need to be adjusted for the can bus frequency and clock frequency
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

  //Code for setting up filters
   CAN_FilterTypeDef canfilterconfig; //struct that contains all the info for filters

   canfilterconfig.FilterActivation = CAN_FILTER_ENABLE;  //ENABLE
   canfilterconfig.FilterBank = 10;                       //Current filter bank to use
   canfilterconfig.FilterFIFOAssignment = CAN_RX_FIFO0;   //Using Fifo0 interrupt
   canfilterconfig.FilterIdHigh = 0x401<<5;               //Assuming we are only filtering for 0x401 message
   canfilterconfig.FilterIdLow = 0x0000;
   canfilterconfig.FilterMaskIdHigh = 0; //changed from 0x1<<13
   canfilterconfig.FilterMaskIdLow = 0x0000;
   canfilterconfig.FilterMode = CAN_FILTERMODE_IDMASK;    //Masking mode
   canfilterconfig.FilterScale = CAN_FILTERSCALE_32BIT;   
   canfilterconfig.SlaveStartFilterBank = 13; // tells where to start giving the slave filters

   HAL_CAN_ConfigFilter(&hcan, &canfilterconfig);         //sets up the filter according the info in canfilterconfig

  /* USER CODE END CAN_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

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
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, Direction_Pin|Pwr_Eco_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : Direction_Pin */
  GPIO_InitStruct.Pin = Direction_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(Direction_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : Pwr_Eco_Pin */
  GPIO_InitStruct.Pin = Pwr_Eco_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(Pwr_Eco_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

//Interrupt where code will go when it recieves a CAN message.
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) //recieve data in this function
{
  //gets the CAN message with info in RxHeader and data in RxData
	if(HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, RxData) != HAL_OK)
	{
		Error_Handler();
	}

  //sets a flag to let the main loop know a message has been recieved 
	Parse_Data_Flag = 1; 

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
