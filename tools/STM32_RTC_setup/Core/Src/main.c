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
#include <time.h>
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "nmea_parse.h"
//#include "nmea_parse.h"
//#include "debug_io.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

	char RTC_time[30];
	char RTC_date[30];

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

RTC_HandleTypeDef hrtc;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

#define DEVICE_ADDRESS (0x42<<1)

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_RTC_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */

#ifdef __GNUC__
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif

PUTCHAR_PROTOTYPE
{
  HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
  return ch;
}

time_t convertToEpochTime(RTC_TimeTypeDef *sTime, RTC_DateTypeDef *sDate);
int lastDayOfMonth(int month, int year);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

	RTC_TimeTypeDef sTime;
	RTC_DateTypeDef sDate;

	int isGPSDateTimeSet = 0;

void readNMEA(uint8_t *buffer){
		HAL_I2C_Master_Receive(&hi2c1, DEVICE_ADDRESS, buffer, sizeof(buffer), HAL_MAX_DELAY);
}

void getGPSDateTime(uint8_t *buffer, char *GPSTime, char *GPSDate){
    GPS myData;
    nmea_parse(&myData, buffer);

    // lastMeasure is a null-terminated string and has the format hhmmss.sss
    if(myData.fix == 1 && myData.RMC_Flag == 1) { // Make sure there's a valid fix and that there is an RMC message

        strncpy(GPSTime, myData.lastMeasure, 10); // Copy the GPS time to GPSTime
        GPSTime[10] = '\0'; // Ensure null termination

        strncpy(GPSDate, myData.date, 6);
        GPSDate[6] = '\0';

        // Check if we've already set the RTC time with GPS time

        if (isGPSDateTimeSet == 0) {
            RTC_TimeTypeDef sTime = {0};

            // Manually parsing the hours, minutes, and seconds
            sTime.Hours   = (GPSTime[0] - '0') * 10 + (GPSTime[1] - '0');
            sTime.Minutes = (GPSTime[2] - '0') * 10 + (GPSTime[3] - '0');
            sTime.Seconds = (GPSTime[4] - '0') * 10 + (GPSTime[5] - '0');

            HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);

            RTC_DateTypeDef sDate = {0};
            sDate.Date  = (GPSDate[0] - '0') * 10 + (GPSDate[1] - '0');
			sDate.Month = (GPSDate[2] - '0') * 10 + (GPSDate[3] - '0');
			sDate.Year  = (GPSDate[4] - '0') * 10 + (GPSDate[5] - '0');

			HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

            isGPSDateTimeSet = 1;
        }
        else{
        	HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
        	HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

			time_t epochTime = convertToEpochTime(&sTime, &sDate);

	    	printf("Epoch Time: %ld\r\n", (long)epochTime);
        }
    }
}

time_t convertToEpochTime(RTC_TimeTypeDef *sTime, RTC_DateTypeDef *sDate) {
    struct tm t;

    // Populate the tm structure fields from the RTC time and date.
    t.tm_year = sDate->Year + 100;  // Year since 1900, so add 100 to the year from the RTC (which starts at 2000).
    t.tm_mon = sDate->Month - 1;    // tm_mon is 0-11, so subtract 1 from the RTC month (which starts at 1).
    t.tm_mday = sDate->Date;
    t.tm_hour = sTime->Hours;
    t.tm_min = sTime->Minutes;
    t.tm_sec = sTime->Seconds;
    t.tm_isdst = -1;                // Disable daylight saving time adjustments.

    // Subtract 8 hours with rollback feature.
    t.tm_hour -= 8;
    if (t.tm_hour < 0) {
        t.tm_hour += 24;  // Adjust the hour to ensure it's not less than 0.
        t.tm_mday--;      // Decrement the day to reflect the day change.

        // Adjust the month and year if needed when day rolls below 1.
        if (t.tm_mday < 1) {
            t.tm_mon--;  // Decrement the month.
            if (t.tm_mon < 0) { // If month rolls below January
                t.tm_mon = 11; // Set month to December
                t.tm_year--;   // Decrement the year
            }
            // Set day to last day of the new month.
            t.tm_mday = lastDayOfMonth(t.tm_mon, t.tm_year + 1900);
        }
    }

    // Convert to epoch time.
    return mktime(&t);
}

// Function to return the last day of a month
int lastDayOfMonth(int month, int year) {
    // Array to hold the number of days in each month
    int daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    // Check for leap year in February
    if (month == 1) { // February
        if ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0) {
            return 29;
        }
    }
    return daysInMonth[month];
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
  MX_RTC_Init();
  MX_I2C1_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
//  DebugIO_Init(&huart1);

  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  if(HAL_I2C_IsDeviceReady(&hi2c1, DEVICE_ADDRESS, 1, HAL_MAX_DELAY) == HAL_OK){
		  uint8_t receiveBuffer[256]; // The size of all of these buffers is tentative for testing sake, will change later
		  char GPSTime[256];
		  char GPSDate[256];

		  HAL_I2C_Master_Receive(&hi2c1, DEVICE_ADDRESS, receiveBuffer, sizeof(receiveBuffer), HAL_MAX_DELAY);
		  getGPSDateTime(receiveBuffer, GPSTime, GPSDate);

	  }
	  else{
		  printf("I2C not working buddy... do better...\r\n");
	  }
	  HAL_Delay(500);

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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
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
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef DateToUpdate = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.AsynchPrediv = RTC_AUTO_1_SECOND;
  hrtc.Init.OutPut = RTC_OUTPUTSOURCE_ALARM;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */

  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date
  */
//  sTime.Hours = 0x16;
//  sTime.Minutes = 0x39;
//  sTime.Seconds = 0x10;

  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
//  DateToUpdate.WeekDay = RTC_WEEKDAY_SATURDAY;
//  DateToUpdate.Month = RTC_MONTH_JANUARY;
//  DateToUpdate.Date = 0x13;
//  DateToUpdate.Year = 0x24;

  if (HAL_RTC_SetDate(&hrtc, &DateToUpdate, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

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
