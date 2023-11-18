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
#include "cmsis_os.h"
#include "fatfs.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "fatfs_sd.h"
#include "string.h"
#include "stdio.h"
#include "time.h"
#include "stdlib.h"

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

SPI_HandleTypeDef hspi1;

UART_HandleTypeDef huart2;

osThreadId ReadCANHandle;
osThreadId SDwriteHandle;
/* USER CODE BEGIN PV */

//  defining struct for CAN rx header
CAN_RxHeaderTypeDef CAN_rx_header;
uint8_t CAN_rx_data[8];

char SD_message[64] = "";

// defining structs for CAN, GPS, IMU data
struct CAN_DATA{
	uint16_t ID;
	uint64_t DATA;
	uint64_t TIME;
	uint8_t LENGTH;
};

struct GPS_DATA{
	float LATITUDE;
	float LONGITUDE;
	uint16_t SAT;
	float ALTITUDE;
};

struct IMU_DATA{
	uint16_t TYPE;
	uint16_t DIM;
	uint64_t DATA;
};

// creating structs for CAN, GPS, IMU data
struct CAN_DATA CAN_DATA;
struct GPS_DATA GPS_DATA;
struct IMU_DATA IMU_DATA;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_CAN_Init(void);
void StartReadCAN(void const * argument);
void StartSDwrite(void const * argument);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

FATFS fs;				// file system
FIL fil;				// file
FRESULT fresult;		// to store the result
char buffer[1024];		// to store data

UINT br,bw;				// file read/write count

/* capacity related variables */
FATFS *pfs;
DWORD fre_clust;
uint32_t total, free_space;

/* to send the data to the uart */
void send_uart (char *string)
{
	uint8_t len = strlen(string);
	HAL_UART_Transmit(&huart2, (uint8_t *) string, len, 2000);	// transmit in blocking mode

}

int bufsize (char *buf)
{
	int i = 0;
	while(*buf++ != '\0') i++;
	return 1;
}

void bufclear(void)	// clear buffer
{
	for (int i = 0; i < 1024; i++)
	{
		buffer[i] = '\0';
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
  MX_FATFS_Init();
  MX_SPI1_Init();
  MX_USART2_UART_Init();
  MX_CAN_Init();
  /* USER CODE BEGIN 2 */

  /* mount SD card */
  fresult = f_mount(&fs, "", 0);
  if(fresult != FR_OK)
	  send_uart ("error in mounting SD CARD...\n");
  else
	  send_uart ("SD Card mounted successfully...\n");

  /* generating random file name for testing purposes - WILL BE CHANGED LATER */
  srand( 2 );												// trying to set seed
  int randomNum = rand();
  char fileName[40];
  sprintf(fileName, "file%d.txt", randomNum);				// generates random file name to be loaded onto SD card

//  /* card capacity details */
//
//	  /* check free space */
//	  f_getfree("", &fre_clust, &pfs);
//
//	  total = (uint32_t)((pfs->n_fatent - 2) * pfs->csize * 0.5);
//	  sprintf (buffer, "SD Card total size: \t%lu\r\n", free_space);
//	  send_uart(buffer);
//	  bufclear();
//	  free_space = (uint32_t)(fre_clust * pfs->csize * 0.5);
//	  sprintf (buffer, "SD Card free space: \t%lu\r\n", free_space);
//	  send_uart(buffer);


//  /* testing setting values for CAN_DATA */
//  CAN_DATA.ID = 0x800;
//  CAN_DATA.DATA = 0x8324;
//  CAN_DATA.TIME = 28492;
//  CAN_DATA.LENGTH = 8;
//
//
//
//  /* CAN Setup */
//  HAL_CAN_Start(&hcan);
//
//  /* CAN Rx testing */
//
//	  // check if message is available
//	  if (HAL_CAN_GetRxFifoFillLevel(&hcan, CAN_RX_FIFO0) != 0){
//
//		  // receive message and store header info in CAN_rx_header, and data bytes into CAN_rx_data
//		  HAL_CAN_GetRxMessage(&hcan, CAN_RX_FIFO0, &CAN_rx_header, CAN_rx_data);
//
//		  CAN_DATA.ID = (uint16_t) CAN_rx_header.StdId;
//	  	  CAN_DATA.DATA = (uint64_t) CAN_rx_data;
//		  CAN_DATA.TIME = (uint64_t) CAN_rx_header.Timestamp;
//		  CAN_DATA.LENGTH = (uint8_t) CAN_rx_header.DLC;
//	  }

	  // writing CAN data to SD card

//	  /* open file to write/ create a file if it doesn't exist */
//	  fresult = f_open(&fil, fileName, FA_OPEN_APPEND | FA_READ | FA_WRITE);

//	  // creating message with CAN data
//	  sprintf(SD_message, "ID: %#.3x, Data: %#.4x, Timestamp: %d, Length: %d", CAN_DATA.ID, CAN_DATA.DATA, CAN_DATA.TIME, CAN_DATA.LENGTH);
//
//	  // writing CAN line to SD card and closing file
//	  fresult = f_puts(SD_message, &fil);
//
//	  fresult = f_close(&fil);
//
//  	  send_uart("File2.txt created and the data is written \r\n");
//
//	  /* open file to read */
//	  fresult = f_open(&fil, fileName, FA_READ);
//
//	  /* read string from the file */
//	  f_gets(buffer, fil.fsize, &fil);
//
//	  send_uart(buffer);
//
//	  /* close file */
//	  f_close(&fil);
//
//	  bufclear();







//  /* Testing manually writing to the file */
//
//  	  /*********** the following operation is using PUTS and GETS ****************/
//
//  	  /* open file to write/ create a file if it doesn't exist */
//  	  fresult = f_open(&fil, fileName, FA_OPEN_APPEND | FA_READ | FA_WRITE);
//
//  	  /* writing text */
//  	  fresult = f_puts("Appended text4\n\n", &fil);
//  	  fresult = f_puts("Appended text5\n\n", &fil);
//  	  fresult = f_puts("Appended text6\n\n", &fil);
//
//  	  /* close file */
//  	  fresult = f_close(&fil);
//
//  	  send_uart("File2.txt created and the data is written \r\n");
//
//  	  /* open file to read */
//  	  fresult = f_open(&fil, fileName, FA_READ);
//
//  	  /* read string from the file */
//  	  f_gets(buffer, fil.fsize, &fil);
//
//  	  send_uart(buffer);
//
//  	  /* close file */
//  	  f_close(&fil);
//
//  	  bufclear();

  /* USER CODE END 2 */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of ReadCAN */
  osThreadDef(ReadCAN, StartReadCAN, osPriorityNormal, 0, 128);
  ReadCANHandle = osThreadCreate(osThread(ReadCAN), NULL);

  /* definition and creation of SDwrite */
  osThreadDef(SDwrite, StartSDwrite, osPriorityLow, 0, 128);
  SDwriteHandle = osThreadCreate(osThread(SDwrite), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
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
  hcan.Init.Prescaler = 4;
  hcan.Init.Mode = CAN_MODE_NORMAL;
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

  /* USER CODE END CAN_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

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
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(chip_select_GPIO_Port, chip_select_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : chip_select_Pin */
  GPIO_InitStruct.Pin = chip_select_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(chip_select_GPIO_Port, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartReadCAN */
/**
* @brief Function implementing the ReadCAN thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartReadCAN */
void StartReadCAN(void const * argument)
{
  /* USER CODE BEGIN 5 */

	// CAN setup
	HAL_CAN_Start(&hcan);

	/* testing setting values for CAN_DATA */
	CAN_DATA.ID = 0x800;
	CAN_DATA.DATA = 0x8324;
	CAN_DATA.TIME = 28492;
	CAN_DATA.LENGTH = 8;

  /* Infinite loop */
  for(;;)
  {
	  osDelay(1);

      // check if message is available
	  if (HAL_CAN_GetRxFifoFillLevel(&hcan, CAN_RX_FIFO0) != 0){

		  // receive message and store header info in CAN_rx_header, and data bytes into CAN_rx_data
		  HAL_CAN_GetRxMessage(&hcan, CAN_RX_FIFO0, &CAN_rx_header, CAN_rx_data);

		  CAN_DATA.ID = (uint16_t) CAN_rx_header.StdId;
		  CAN_DATA.TIME = (uint16_t) CAN_rx_header.Timestamp;
		  CAN_DATA.LENGTH = (uint16_t) CAN_rx_header.DLC;
	  }
  }

  // incase accidentally exits task loop
  osThreadTerminate(NULL);
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_StartSDwrite */
/**
* @brief Function implementing the SDwrite thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartSDwrite */
void StartSDwrite(void const * argument)
{
  /* USER CODE BEGIN StartSDwrite */

  /* Infinite loop */
  for(;;)
  {
    osDelay(1);

    // creating message with CAN data
    sprintf(SD_message, "ID: %#.3x, Data: %#.4x, Timestamp: %d, Length: %d", CAN_DATA.ID, CAN_DATA.DATA, CAN_DATA.TIME, CAN_DATA.LENGTH);

    // writing CAN line to SD card
    fresult = f_puts(SD_message, &fil);
  }

  // incase accidentally exit task loop
  osThreadTerminate(NULL);
  /* USER CODE END StartSDwrite */
}

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
