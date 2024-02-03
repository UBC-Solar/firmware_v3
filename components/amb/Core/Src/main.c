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
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

// Struct initialization for lookup table
typedef struct
{
    float resistanceValue;
    int16_t temperature; // temperature in degrees Celsius
} LookupEntry;



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
#define TABLE_SIZE 166


/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint16_t adc_data[ADC_BUF_SIZE]; // array to hold the temperature readings, and voltage and current readings
uint8_t conv_flag = 0; // flag to indicate when to convert the raw data from ADC to temperature
uint8_t idx = 0; // index for converting values in array

LookupEntry lookupTable[TABLE_SIZE] = {
    {195.652, -40.0},
    {184.9171, -39.0},
    {174.8452, -38.0},
    {165.391, -37.0},
    {156.5125, -36.0},
    {148.171, -35.0},
    {140.3304, -34.0},
    {132.9576, -33.0},
    {126.0215, -32.0},
    {119.4936, -31.0},
    {113.3471, -30.0},
    {107.5649, -29.0},
    {102.1155, -28.0},
    {96.9776, -27.0},
    {92.1315, -26.0},
    {87.5588, -25.0},
    {83.2424, -24.0},
    {79.1663, -23.0},
    {75.3157, -22.0},
    {71.6768, -21.0},
    {68.2367, -20.0},
    {64.9907, -19.0},
    {61.919, -18.0},
    {59.0113, -17.0},
    {56.2579, -16.0},
    {53.6496, -15.0},
    {51.1779, -14.0},
    {48.8349, -13.0},
    {46.6132, -12.0},
    {44.5058, -11.0},
    {42.5062, -10.0},
    {40.5997, -9.0},
    {38.7905, -8.0},
    {37.0729, -7.0},
    {35.4417, -6.0},
    {33.8922, -5.0},
    {32.4197, -4.0},
    {31.02, -3.0},
    {29.689, -2.0},
    {28.4231, -1.0},
    {27.2186, 0.0},
    {26.076, 1.0},
    {24.9877, 2.0},
    {23.9509, 3.0},
    {22.9629, 4.0},
    {22.0211, 5.0},
    {21.123, 6.0},
    {20.2666, 7.0},
    {19.4495, 8.0},
    {18.6698, 9.0},
    {17.9255, 10.0},
    {17.2139, 11.0},
    {16.5344, 12.0},
    {15.8856, 13.0},
    {15.2658, 14.0},
    {14.6735, 15.0},
    {14.1075, 16.0},
    {13.5664, 17.0},
    {13.0489, 18.0},
    {12.554, 19.0},
    {12.0805, 20.0},
    {11.6281, 21.0},
    {11.1947, 22.0},
    {10.7795, 23.0},
    {10.3815, 24.0},
    {10.0, 25.0},
    {9.6342, 26.0},
    {9.2835, 27.0},
    {8.947, 28.0},
    {8.6242, 29.0},
    {8.3145, 30.0},
    {8.0181, 31.0},
    {7.7337, 32.0},
    {7.4609, 33.0},
    {7.1991, 34.0},
    {6.9479, 35.0},
    {6.7067, 36.0},
    {6.4751, 37.0},
    {6.2526, 38.0},
    {6.039, 39.0},
    {5.8336, 40.0},
    {5.6357, 41.0},
    {5.4454, 42.0},
    {5.2623, 43.0},
    {5.0863, 44.0},
    {4.9169, 45.0},
    {4.7539, 46.0},
    {4.5971, 47.0},
    {4.4461, 48.0},
    {4.3008, 49.0},
    {4.1609, 50.0},
    {4.0262, 51.0},
    {3.8964, 52.0},
    {3.7714, 53.0},
    {3.651, 54.0},
    {3.535, 55.0},
    {3.4231, 56.0},
    {3.3152, 57.0},
    {3.2113, 58.0},
    {3.111, 59.0},
    {3.0143, 60.0},
    {2.9224, 61.0},
    {2.8337, 62.0},
    {2.7482, 63.0},
    {2.6657, 64.0},
    {2.5861, 65.0},
    {2.5093, 66.0},
    {2.4351, 67.0},
    {2.3635, 68.0},
    {2.2943, 69.0},
    {2.2275, 70.0},
    {2.1627, 71.0},
    {2.1001, 72.0},
    {2.0396, 73.0},
    {1.9811, 74.0},
    {1.9245, 75.0},
    {1.8698, 76.0},
    {1.817, 77.0},
    {1.7658, 78.0},
    {1.7164, 79.0},
    {1.6685, 80.0},
    {1.6224, 81.0},
    {1.5777, 82.0},
    {1.5345, 83.0},
    {1.4927, 84.0},
    {1.4521, 85.0},
    {1.4129, 86.0},
    {1.3749, 87.0},
    {1.3381, 88.0},
    {1.3025, 89.0},
    {1.268, 90.0},
    {1.2343, 91.0},
    {1.2016, 92.0},
    {1.17, 93.0},
    {1.1393, 94.0},
    {1.1096, 95.0},
    {1.0807, 96.0},
    {1.0528, 97.0},
    {1.0256, 98.0},
    {0.9993, 99.0},
    {0.9738, 100.0},
    {0.9492, 101.0},
    {0.9254, 102.0},
    {0.9022, 103.0},
    {0.8798, 104.0},
    {0.858, 105.0},
    {0.8368, 106.0},
    {0.8162, 107.0},
    {0.7963, 108.0},
    {0.7769, 109.0},
    {0.758, 110.0},
    {0.7397, 111.0},
    {0.7219, 112.0},
    {0.7046, 113.0},
    {0.6878, 114.0},
    {0.6715, 115.0},
    {0.6556, 116.0},
    {0.6402, 117.0},
    {0.6252, 118.0},
    {0.6106, 119.0},
    {0.5964, 120.0},
    {0.5826, 121.0},
    {0.5692, 122.0},
    {0.5562, 123.0},
    {0.5435, 124.0},
    {0.5311, 125.0}};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
//float volt2temp(uint32_t raw_value);
float interpolate(float x, float x0, float x1, float y0, float y1);
float getTemperature(float resistanceValue);

//uint32_t avgReading();
//float log_float(float valueToConvert);
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
			/* V = 3.3 * ADC_VAL / 2^12 */
			CONVERTED_VALUES[index].float_value = 3.3 * ADC_VALUES[index] / pow(2, 12) * 1000;	// slightly overcompensates
			break;
		case VSENSE2:
			/* Relationship for converting ADC for VSENSE1 and modify CONVERTED_VALUES[index] */
			CONVERTED_VALUES[index].float_value = 3.3 * ADC_VALUES[index] / pow(2, 12) * 1000;	// slightly undercompensates
			break;

		case ISENSE1:
			/* ISENSE [V] = 0.11 * I [A] + 1.65 */
//			uint32_t dummyValue = (ADC_VALUES[index] - 1.65) / 0.11;
			CONVERTED_VALUES[index].float_value = (ADC_VALUES[index] - 1.65) / 0.11;
			break;
		case ISENSE2:
			/* ISENSE [V] = 0.11 * I [A] + 1.65 */
			CONVERTED_VALUES[index].float_value = (ADC_VALUES[index] - 1.65) / 0.11;
			break;
		case TEMP_1:
			/* TEMP_1 use Lookup Table using values obtained from the NXFT15XH103FA2B090 Datasheet */
			CONVERTED_VALUES[index].float_value = getTemperature((float)ADC_VALUES[index]);
			break;
		case TEMP_2:
			/* TEMP_2 use Lookup Table using values obtained from the NXFT15XH103FA2B090 Datasheet */
			CONVERTED_VALUES[index].float_value = getTemperature((float)ADC_VALUES[index]);
			break;
		case TEMP_3:
			/* TEMP_3 use Lookup Table using values obtained from the NXFT15XH103FA2B090 Datasheet */
			CONVERTED_VALUES[index].float_value = getTemperature((float)ADC_VALUES[index]);
			break;
		case TEMP_4:
			/* TEMP_4 use Lookup Table using values obtained from the NXFT15XH103FA2B090 Datasheet */
			CONVERTED_VALUES[index].float_value = getTemperature((float)ADC_VALUES[index]);
			break;
		case TEMP_5:
			/* TEMP_5 use Lookup Table using values obtained from the NXFT15XH103FA2B090 Datasheet */
			CONVERTED_VALUES[index].float_value = getTemperature((float)ADC_VALUES[index]);
			break;
		case TEMP_6:
			/* TEMP_6 use Lookup Table using values obtained from the NXFT15XH103FA2B090 Datasheet */
			CONVERTED_VALUES[index].float_value = getTemperature((float)ADC_VALUES[index]);
			break;
		case TEMP_7:
			/* TEMP_7 use Lookup Table using values obtained from the NXFT15XH103FA2B090 Datasheet */
			CONVERTED_VALUES[index].float_value = getTemperature((float)ADC_VALUES[index]);
			break;
		case TEMP_8:
			/* TEMP_8 use Lookup Table using values obtained from the NXFT15XH103FA2B090 Datasheet */
			CONVERTED_VALUES[index].float_value = getTemperature((float)ADC_VALUES[index]);
			break;
	}
}
//
///**
// * Method to convert a given ADC Channel value into a Temperature value
// *
// * @param raw_value the raw thermistor value from the ADC
// * @param REF the reference voltage for the board typically 3.3V
// *
// * @return the float value for the converted temperature in Celsius
// */
//
//float volt2temp(uint32_t raw_value){
//		const float A = 3380; //from NXFT15XH103FA2B090 data sheet B25/50 Value
//		const float B = 3428; //from NXFT15XH103FA2B090 data sheet B25/80 Value
//		const float C = 3455; //from NXFT15XH103FA2B090 data sheet B25/100 Value
//
//	    const float R_balance = 3300; // from AMB PCB Schematic component R4.1 to R4.8
//	    float V_therm = 0.0;
//	    float R_therm = 0.0;
//	    float Vout = 3.3; //Reference Voltage from Board
//	    double temp_celsius = 0.0;
//
//	    V_therm = 3.3 * raw_value / pow(2, 12);
//	    R_therm = (R_balance * ((Vout/V_therm) - 1))/1000; //convert to kOhm
//
//	    //SteinHart-Hart Equation: https://www.ametherm.com/thermistor/ntc-thermistors-steinhart-and-hart-equation
////	    temp_kelvin = 1/(A + B*log(R_therm) + C* pow(log(R_therm),3) );
//	    temp_celsius = - log((5*R_therm)/158)/0.042;
////	    temp_celsius = (double) 31.6 * exp(-0.042*R_therm);
//	    //return a Celsius value
//	    return temp_celsius;
////	    return temp_kelvin - 273.15;
//}
//
////Helper method to get average 5 readings from ADC Channel
//uint32_t avgReading(){
//	uint32_t dummyValue =0;
//	uint32_t average=0;
//
//	for(int index=0; index < 5; index++){
//			  HAL_ADC_PollForConversion(&hadc1, 1000);
//			  dummyValue = HAL_ADC_GetValue(&hadc1);
//			  average+= dummyValue;
//		}
//	return average/5;
//}

// Helper function to perform linear interpolation
float interpolate(float x, float x0, float x1, float y0, float y1)
{
    return y0 + ((x - x0) * (y1 - y0)) / (x1 - x0);
}


/**
 * Method to convert a given ADC Channel value into a Temperature value using a Lookup Table
 * Refer to the NXFT15XH103FA2B090 data sheet
 *
 * @param raw_value the raw thermistor value from the ADC
 * @return the float value for the converted temperature in Celsius
 */
float getTemperature(float resistanceValue)
{
    // Find the index of the closest entry to reading
    int closestIndex = 0;
    double closestDiff = fabs(resistanceValue - lookupTable[0].resistanceValue);
    for (int i = 0; i < TABLE_SIZE; i++)
    {
        double currentDiff = fabs(resistanceValue - lookupTable[i].resistanceValue);
        if (currentDiff < closestDiff)
        {
            closestIndex = i;
            closestDiff = currentDiff;
        }
    }

    if (closestIndex < TABLE_SIZE - 1) // If no exact match is found, perform linear interpolation
    {
        return interpolate(resistanceValue, lookupTable[closestIndex].resistanceValue, lookupTable[closestIndex + 1].resistanceValue,
                           lookupTable[closestIndex].temperature, lookupTable[closestIndex + 1].temperature);
    }
    else
    {
        return lookupTable[TABLE_SIZE - 1].temperature; // Return the last entry if we are at the end of the table
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

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

	  /* Read ADC, process, then output to CANbus */

	  /* VSENSE1 */
	  ADC_Select((uint8_t) VSENSE1);
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
	  Convert_Values((uint8_t) ISENSE1);

	  /* ISENSE2 */
	  ADC_Select((uint8_t) ISENSE2);
	  HAL_ADC_PollForConversion(&hadc1, 1000);
	  ADC_VALUES[ISENSE2] = HAL_ADC_GetValue(&hadc1);
	  Convert_Values((uint8_t) ISENSE2);


	  /* TEMP_1 */
	  ADC_Select((uint8_t) TEMP_1);
	  HAL_ADC_PollForConversion(&hadc1, 1000);
	  ADC_VALUES[TEMP_1] = HAL_ADC_GetValue(&hadc1);
	  Convert_Values((uint8_t) TEMP_1);

	  /* TEMP_2 */
	  ADC_Select((uint8_t) TEMP_2);
	  HAL_ADC_PollForConversion(&hadc1, 1000);
	  ADC_VALUES[TEMP_2] = HAL_ADC_GetValue(&hadc1);
	  Convert_Values((uint8_t) TEMP_2);

	  /* TEMP_3 */
	  ADC_Select((uint8_t) TEMP_3);
	  HAL_ADC_PollForConversion(&hadc1, 1000);
	  ADC_VALUES[TEMP_3] = HAL_ADC_GetValue(&hadc1);
	  Convert_Values((uint8_t) TEMP_3);

	  /* TEMP_4 */
	  ADC_Select((uint8_t) TEMP_4);
	  HAL_ADC_PollForConversion(&hadc1, 1000);
	  ADC_VALUES[TEMP_4] = HAL_ADC_GetValue(&hadc1);
	  Convert_Values((uint8_t) TEMP_4);

	  /* TEMP_5 */
	  ADC_Select((uint8_t) TEMP_5);
	  HAL_ADC_PollForConversion(&hadc1, 1000);
	  ADC_VALUES[TEMP_5] = HAL_ADC_GetValue(&hadc1);
	  Convert_Values((uint8_t) TEMP_5);

	  /* TEMP_6 */
	  ADC_Select((uint8_t) TEMP_6);
	  HAL_ADC_PollForConversion(&hadc1, 1000);
	  ADC_VALUES[TEMP_6] = HAL_ADC_GetValue(&hadc1);
	  Convert_Values((uint8_t) TEMP_6);

	  /* TEMP_7 */
	  ADC_Select((uint8_t) TEMP_7);
 	  HAL_ADC_PollForConversion(&hadc1, 1000);
	  ADC_VALUES[TEMP_7] = HAL_ADC_GetValue(&hadc1);
	  Convert_Values((uint8_t) TEMP_7);

	  /* TEMP_8 */
	  ADC_Select((uint8_t) TEMP_8);
	  HAL_ADC_PollForConversion(&hadc1, 1000);
	  ADC_VALUES[TEMP_8] = HAL_ADC_GetValue(&hadc1);
	  Convert_Values((uint8_t) TEMP_8);


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


	  HAL_CAN_AddTxMessage(&hcan, &txHeaderVoltage, voltage_data, &txMailbox);
	  HAL_CAN_AddTxMessage(&hcan, &txHeaderCurrent, current_data, &txMailbox);
	  HAL_CAN_AddTxMessage(&hcan, &txHeaderTemp, temp_data, &txMailbox);

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
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
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
