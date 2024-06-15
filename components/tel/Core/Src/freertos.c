/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include <stdio.h>
#include <time.h>
#include "can.h"
#include "gpio.h"
#include "spi.h"
#include "usart.h"
#include "i2c.h"
#include "nmea_parse.h"
#include "rtc.h"
#include "fatfs_sd.h"
#include "imu.h"
#include "iwdg.h"
#include "sd_logger.h"
#include "main.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define KERNEL_LED_DELAY   200	      // 200 milliseconds
#define READ_IMU_DELAY     100	      // 100 milliseconds
#define READ_GPS_DELAY     10 * 1000  // 10 seconds (change to 5 minutes later)
#define TRANSMIT_RTC_DELAY 5000       // 5000 milliseconds
#define DEFAULT_TASK_DELAY 100        // 100 milliseconds
#define TRANSMIT_DIAGNOSTICS_DELAY 2000 // 2000 milliseconds
#define GPS_WAIT_MSG_DELAY 10 * 1000    // 10 seconds/10000 milliseconds wait before checking for GPS msg
#define IMU_SINGLE_DELAY 125 // 125ms single delay, * 2 = 250ms per group of IMU messages.
#define CAN_BUFFER_LEN  24

#define GPS_MESSAGE_LEN 200

#define GPS_RCV_BUFFER_SIZE 512


union FloatBytes {
    float float_value;
    uint8_t bytes[4];
} FloatBytes;


/* Use this MACRO to split the time stamp into individual bytes */
#define GET_BYTE_FROM_WORD(i, word) ((word >> (i * 8)) & 0xFF);


/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

extern FIL* logfile;

CAN_msg_t msg_t;

int drive_cmd_count; // Used for only logging this 10 times per second

/* Define Memory Pool for the CAN_MSG queue data */
osPoolDef (CAN_MSG_memory_pool, 32, CAN_msg_t);  // Declare memory pool
osPoolId  CAN_MSG_memory_pool;                 // Memory pool ID

/* Create the Queue */
// https://community.st.com/t5/stm32-mcus-products/osmessageget-crashing/td-p/257324
// See this link for issues why osMessageGet may crash
// osMessageQDef needs a pointer to the structure type, which needs to be a "dummy", not pointer to typedef.
osMessageQDef(CAN_MSG_Rx_Queue, 32, &msg_t);              // Define message queue
osMessageQId  CAN_MSG_Rx_Queue;

/* USER CODE END Variables */
osThreadId StartDefaultTaskHandle;
osThreadId readCANTaskHandle;
osThreadId readIMUTaskHandle;
osThreadId readGPSTaskHandle;
osThreadId transmitDiagnosticsTaskHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void check_IMU_result(union FloatBytes ax_x, union FloatBytes ax_y, union FloatBytes ax_z,
		      union FloatBytes gy_x, union FloatBytes gy_y, union FloatBytes gy_z);

void send_CAN_Radio(CAN_Radio_msg_t *tx_CAN_msg);


/* USER CODE END FunctionPrototypes */

void startDefaultTask(void const * argument);
void read_CAN_task(void const * argument);
void read_IMU_task(void const * argument);
void read_GPS_task(void const * argument);
void transmit_Diagnostics_task(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  drive_cmd_count = 0;

  /* USER CODE END Init */

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

  CAN_MSG_memory_pool = osPoolCreate(osPool(CAN_MSG_memory_pool));                 // create memory pool
  CAN_MSG_Rx_Queue = osMessageCreate(osMessageQ(CAN_MSG_Rx_Queue), NULL);  // create msg queue


  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of StartDefaultTask */
  osThreadDef(StartDefaultTask, startDefaultTask, osPriorityLow, 0, 128);
  StartDefaultTaskHandle = osThreadCreate(osThread(StartDefaultTask), NULL);

  /* definition and creation of readCANTask */
  osThreadDef(readCANTask, read_CAN_task, osPriorityNormal, 0, 512);
  readCANTaskHandle = osThreadCreate(osThread(readCANTask), NULL);

  /* definition and creation of readIMUTask */
  osThreadDef(readIMUTask, read_IMU_task, osPriorityNormal, 0, 512);
  readIMUTaskHandle = osThreadCreate(osThread(readIMUTask), NULL);

  /* definition and creation of readGPSTask */
  osThreadDef(readGPSTask, read_GPS_task, osPriorityNormal, 0, 1536);
  readGPSTaskHandle = osThreadCreate(osThread(readGPSTask), NULL);

  /* definition and creation of transmitDiagnosticsTask */
  osThreadDef(transmitDiagnosticsTask, transmit_Diagnostics_task, osPriorityNormal, 0, 512);
  transmitDiagnosticsTaskHandle = osThreadCreate(osThread(transmitDiagnosticsTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_startDefaultTask */
/**
  * @brief  Function implementing the StartDefaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_startDefaultTask */
void startDefaultTask(void const * argument)
{
  /* USER CODE BEGIN startDefaultTask */
  CAN_Init();

  /* Infinite loop */
  for(;;)
  {
    #ifndef DEBUG
      HAL_IWDG_Refresh(&hiwdg);
    #endif
    osDelay(DEFAULT_TASK_DELAY);
  }
  /* USER CODE END startDefaultTask */
}

/* USER CODE BEGIN Header_read_CAN_task */
/**
* @brief Function implementing the readCANTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_read_CAN_task */
void read_CAN_task(void const * argument)
{
  /* USER CODE BEGIN read_CAN_task */
  CAN_msg_t *rx_CAN_msg;
  osEvent evt;

  /* Infinite loop */
  while (true) {
    /* Wait for thread flags to be set in the CAN Rx FIFO0 Interrupt Callback */
    osSignalWait(CAN_READY, osWaitForever);
    
    #ifndef DEBUG
      HAL_IWDG_Refresh(&hiwdg);
    #endif
    /*
     * Control Flow:
     * Wait for Flag from Interrupt
     * After flag occurs, read messages from queue repeatedly until it is empty
     * Once empty,    // HAL_StatusTypeDef imu_status = HAL_OK;

    /* Get CAN Message from Queue */
    while(true) {
      evt = osMessageGet(CAN_MSG_Rx_Queue, osWaitForever);

      if (evt.status == osEventMessage) {
        rx_CAN_msg = evt.value.p;                       // Get pointer to CAN message from the queue union
        CAN_handle_rx_msg(rx_CAN_msg);                 // Handle the CAN message  
      }
      else break;
    }
  }
}

/* USER CODE BEGIN Header_read_IMU_task */
/**
* @brief Function implementing the readIMUTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_read_IMU_task */
void read_IMU_task(void const * argument)
{
  /* USER CODE BEGIN read_IMU_task */

  /* Infinite loop */
  while(1)
  {
    HAL_StatusTypeDef imu_status = HAL_OK;

    IMU_send_as_CAN_msg_single_delay(&imu_status);                // Send IMU data as CAN message
    g_tel_diagnostics.imu_fail = (imu_status != HAL_OK);        // Update diagnostics
    osDelay(IMU_SINGLE_DELAY);
  }

  /* USER CODE END read_IMU_task */
}


/* USER CODE BEGIN Header_read_GPS_task */
/**
* @brief Function implementing the readGPSTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_read_GPS_task */
void read_GPS_task(void const * argument)
{
  /* USER CODE BEGIN read_GPS_task */

  /* Infinite loop */
  while(1) {
    GPS_delayed_rx_and_tx_as_CAN();       // Once a fix is obtained create and send GPS message as CAN
  }

  /* USER CODE END read_GPS_task */
}


/* USER CODE BEGIN Header_transmit_Diagnostics_task */
/**
* @brief Function implementing the transmitDiagnosticsTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_transmit_Diagnostics_task */
void transmit_Diagnostics_task(void const * argument)
{
  /* USER CODE BEGIN transmit_Diagnostics_task */
  /* Infinite loop */
  for(;;)
  {
    CAN_diagnostic_msg_tx_radio_bus();
    osDelay(TRANSMIT_DIAGNOSTICS_DELAY);
  }
  /* USER CODE END transmit_Diagnostics_task */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/*
 * Performs IMU Data Check
 */
void check_IMU_result(union FloatBytes ax_x, union FloatBytes ax_y, union FloatBytes ax_z,
		      union FloatBytes gy_x, union FloatBytes gy_y, union FloatBytes gy_z)
{
  if (ax_x.float_value == 0.0f && ax_y.float_value == 0.0f && ax_z.float_value == 0.0f &&
      gy_x.float_value == 0.0f && gy_y.float_value == 0.0f && gy_z.float_value == 0.0f) {
      // This means there was a disconnect in one of the IMU cables and it needs to be re-initialized
      initIMU();
  }
}

void send_CAN_Radio(CAN_Radio_msg_t *tx_CAN_msg)
{
  uint8_t radio_buffer[CAN_BUFFER_LEN] = {0};

  /* TIMESTAMP */
  for (uint8_t i = 0; i < 8; i++) {
    radio_buffer[7 - i] = (char) GET_BYTE_FROM_WORD(i, tx_CAN_msg->timestamp.double_as_int);
  }

  /* CAN MESSAGE IDENTIFIER */
  radio_buffer[8] = '#';

  /* CAN ID */
  if (tx_CAN_msg->header.IDE == CAN_ID_STD)
  {
    radio_buffer[12]  = 0xFF & (tx_CAN_msg->header.StdId);
    radio_buffer[11] = 0xFF & (tx_CAN_msg->header.StdId >> 8);
  }
  else if (tx_CAN_msg->header.IDE == CAN_ID_EXT)
  {
     radio_buffer[12]  = 0xFF & (tx_CAN_msg->header.ExtId);
     radio_buffer[11] = 0xFF & (tx_CAN_msg->header.ExtId >> 8);
     radio_buffer[10] = 0xFF & (tx_CAN_msg->header.ExtId >> 16);
     radio_buffer[9] = 0xFF & (tx_CAN_msg->header.ExtId >> 24);
  }

   /* CAN DATA */
   for (uint8_t i = 0; i < 8; i++) {
     radio_buffer[13 + i] = tx_CAN_msg->data[i];
   }

   /* CAN DATA LENGTH */
   radio_buffer[21] = tx_CAN_msg->header.DLC & 0xF;

   /* CARRIAGE RETURN */
   radio_buffer[CAN_BUFFER_LEN - 2] = '\r';

   /* NEW LINE */
   radio_buffer[CAN_BUFFER_LEN - 1] = '\n';

   HAL_UART_Transmit(&huart1, radio_buffer, sizeof(radio_buffer), 1000);

}

/* USER CODE END Application */

