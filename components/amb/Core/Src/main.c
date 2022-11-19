/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
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
#include "adc.h"
#include "can.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */



/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/*
 *
 * Constant Definitions
 *
 *
 */
#define DATA_LENGTH 8


/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint16_t adc_data[ADC_BUF_SIZE]; // array to hold the temperature readings, and voltage and current readings
uint8_t conv_flag = 0; // flag to indicate when to convert the raw data from ADC to temperature
uint8_t idx = 0; // index for converting values in array

CAN_TxHeaderTypeDef txHeaderVoltage = {
		.StdId = 0x701,
		.ExtId = 0x0000,
		.IDE = CAN_ID_STD,
		.RTR = CAN_RTR_DATA,
		.DLC = DATA_LENGTH};

CAN_TxHeaderTypeDef txHeaderCurrent = {
		.StdId = 0x702,
		.ExtId = 0x0000,
		.IDE = CAN_ID_STD,
		.RTR = CAN_RTR_DATA,
		.DLC = DATA_LENGTH};

CAN_TxHeaderTypeDef txHeaderTemp = {
		.StdId = 0x703,
		.ExtId = 0x0000,
		.IDE = CAN_ID_STD,
		.RTR = CAN_RTR_DATA,
		.DLC = DATA_LENGTH};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

uint32_t ADC_VALUES[ADC_CHANNELS] = {0};	// unprocessed ADC Values
union FloatBytes CONVERTED_VALUES[ADC_CHANNELS] = {0};	// processed values

void Convert_Values(uint8_t index) {
	/* Convert value with appropriate relationship given the index */

	switch(index) {
		case VSENSE1:
			/* Relationship for converting ADC for VSENSE1 and modify CONVERTED_VALUES[index] */
			/* TODO: Figure out actual Voltage Equations */
			/* V = 3.3 * ADC_VAL / 2^12 */
			CONVERTED_VALUES[index].float_value = (ADC_VALUES[index] + 81.822) / 40.441;	// slightly overcompensates
			break;
		case VSENSE2:
			/* Relationship for converting ADC for VSENSE1 and modify CONVERTED_VALUES[index] */
			CONVERTED_VALUES[index].float_value = (ADC_VALUES[index] + 82.621) / 40.271;	// slightly undercompensates
			break;

		case ISENSE1:
			/* ISENSE [V] = 0.11 * I [A] + 1.65 */
			CONVERTED_VALUES[index].float_value = (ADC_VALUES[index] - 1.65) / 0.11;
			break;
		case ISENSE2:
			/* ISENSE [V] = 0.11 * I [A] + 1.65 */
			CONVERTED_VALUES[index].float_value = (ADC_VALUES[index] - 1.65) / 0.11;
			break;
		case TEMP_1:
			/* TODO */
			break;
		case TEMP_2:
			/* TODO */
			break;
		case TEMP_3:
			/* TODO */
			break;
		case TEMP_4:
			/* TODO */
			break;
		case TEMP_5:
			/* TODO */
			break;
		case TEMP_6:
			/* TODO */
			break;
		case TEMP_7:
			/* TODO */
			break;
		case TEMP_8:
			/* Relationship for converting to temperature and modify CONVERTED_VALUES[index] */
			/* TODO */
			break;
	}
}

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
  MX_ADC1_Init();
  MX_TIM1_Init();
  MX_CAN_Init();
  /* USER CODE BEGIN 2 */
  HAL_CAN_Start(&hcan);

  HAL_ADC_Start(&hadc1);


  /*
   *
   * Manually written code starts here
   *
   *
   */

  // Values from ADC; index 0 = voltage, 1 = current; 2 = raw temperature



//  if (HAL_OK !=  HAL_ADCEx_InjectedStart(&hadc1)) {
//	  Error_Handler();
//  }

  // if (HAL_OK != HAL_ADC_Start_DMA(&hadc1, temperatures, ADC_BUF_SIZE))
	  // Error_Handler();

  if (HAL_OK != HAL_TIM_Base_Start(&htim1)) {
	  Error_Handler();
  }

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

	  /* Read ADC, process, then output to CANbus */

	  /* TODO: Output to CANbus */
	  /* TODO: Characterization for Current and Temperature */

	  /* VSENSE1 */
	  ADC_Select((uint8_t) VSENSE1);
	  HAL_ADC_PollForConversion(&hadc1, 1000);
	  ADC_VALUES[VSENSE1] = HAL_ADC_GetValue(&hadc1);
	  Convert_Values((uint8_t) VSENSE1);

	  /* VSENSE 2 */
	  ADC_Select((uint8_t) VSENSE2);
	  HAL_ADC_PollForConversion(&hadc1, 1000);
	  ADC_VALUES[VSENSE2] = HAL_ADC_GetValue(&hadc1);
	  Convert_Values((uint8_t) VSENSE2);

	  /* ISENSE1 */
	  ADC_Select((uint8_t) ISENSE1);
	  HAL_ADC_PollForConversion(&hadc1, 1000);
	  ADC_VALUES[ISENSE1] = HAL_ADC_GetValue(&hadc1);

	  /* ISENSE2 */
	  ADC_Select((uint8_t) ISENSE2);
	  HAL_ADC_PollForConversion(&hadc1, 1000);
	  ADC_VALUES[ISENSE2] = HAL_ADC_GetValue(&hadc1);

	  /* TEMP_1 */
	  ADC_Select((uint8_t) TEMP_1);
	  HAL_ADC_PollForConversion(&hadc1, 1000);
	  ADC_VALUES[TEMP_1] = HAL_ADC_GetValue(&hadc1);

	  /* TEMP_2 */
	  ADC_Select((uint8_t) TEMP_2);
	  HAL_ADC_PollForConversion(&hadc1, 1000);
	  ADC_VALUES[TEMP_2] = HAL_ADC_GetValue(&hadc1);

	  /* TEMP_3 */
	  ADC_Select((uint8_t) TEMP_3);
	  HAL_ADC_PollForConversion(&hadc1, 1000);
	  ADC_VALUES[TEMP_3] = HAL_ADC_GetValue(&hadc1);

	  /* TEMP_4 */
	  ADC_Select((uint8_t) TEMP_4);
	  HAL_ADC_PollForConversion(&hadc1, 1000);
	  ADC_VALUES[TEMP_4] = HAL_ADC_GetValue(&hadc1);

	  /* TEMP_5 */
	  ADC_Select((uint8_t) TEMP_5);
	  HAL_ADC_PollForConversion(&hadc1, 1000);
	  ADC_VALUES[TEMP_5] = HAL_ADC_GetValue(&hadc1);

	  /* TEMP_6 */
	  ADC_Select((uint8_t) TEMP_6);
	  HAL_ADC_PollForConversion(&hadc1, 1000);
	  ADC_VALUES[TEMP_6] = HAL_ADC_GetValue(&hadc1);

	  /* TEMP_7 */
	  ADC_Select((uint8_t) TEMP_7);
	  HAL_ADC_PollForConversion(&hadc1, 1000);
	  ADC_VALUES[TEMP_7] = HAL_ADC_GetValue(&hadc1);

	  /* TEMP_8 */
	  ADC_Select((uint8_t) TEMP_8);
	  HAL_ADC_PollForConversion(&hadc1, 1000);
	  ADC_VALUES[TEMP_8] = HAL_ADC_GetValue(&hadc1);


	  uint8_t voltage_data[DATA_LENGTH] = {
			  CONVERTED_VALUES[VSENSE1].bytes[0],
			  CONVERTED_VALUES[VSENSE1].bytes[1],
		      CONVERTED_VALUES[VSENSE1].bytes[2],
			  CONVERTED_VALUES[VSENSE1].bytes[3],
			  CONVERTED_VALUES[VSENSE2].bytes[0],
			  CONVERTED_VALUES[VSENSE2].bytes[1],
			  CONVERTED_VALUES[VSENSE2].bytes[2],
			  CONVERTED_VALUES[VSENSE2].bytes[3]};

	  uint8_t current_data[DATA_LENGTH] = {
			  CONVERTED_VALUES[ISENSE1].bytes[0],
	  		  CONVERTED_VALUES[ISENSE1].bytes[1],
	  		  CONVERTED_VALUES[ISENSE1].bytes[2],
	  	      CONVERTED_VALUES[ISENSE1].bytes[3],
	  		  CONVERTED_VALUES[ISENSE2].bytes[0],
	  		  CONVERTED_VALUES[ISENSE2].bytes[1],
	  		  CONVERTED_VALUES[ISENSE2].bytes[2],
	  		  CONVERTED_VALUES[ISENSE2].bytes[3]};

	  uint8_t temp_data[DATA_LENGTH] = {
			  (uint8_t)CONVERTED_VALUES[TEMP_1].float_value,
			  (uint8_t)CONVERTED_VALUES[TEMP_2].float_value,
			  (uint8_t)CONVERTED_VALUES[TEMP_3].float_value,
			  (uint8_t)CONVERTED_VALUES[TEMP_4].float_value,
			  (uint8_t)CONVERTED_VALUES[TEMP_5].float_value,
			  (uint8_t)CONVERTED_VALUES[TEMP_6].float_value,
			  (uint8_t)CONVERTED_VALUES[TEMP_7].float_value,
			  (uint8_t)CONVERTED_VALUES[TEMP_8].float_value};



	  HAL_CAN_AddTxMessage(&hcan, &txHeaderVoltage, voltage_data, NULL);
	  HAL_CAN_AddTxMessage(&hcan, &txHeaderCurrent, current_data, NULL);
	  HAL_CAN_AddTxMessage(&hcan, &txHeaderTemp, temp_data, NULL);

	  HAL_Delay (1000);


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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

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
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
	if(HAL_ADC_Stop_DMA(&hadc1) != HAL_OK)
		Error_Handler();

	conv_flag = 1; // flag set to 1 signals to the infinite while(1) loop to process the raw ADC data and convert to a temp value
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
