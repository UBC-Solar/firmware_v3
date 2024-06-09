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
#include "radio_transmit.h"

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
/* USER CODE BEGIN Variables */

CAN_msg_t msg_t;

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
void check_IMU_result(union Utils_FloatBytes_t ax_x, union Utils_FloatBytes_t ax_y, union Utils_FloatBytes_t ax_z,
		      union Utils_FloatBytes_t gy_x, union Utils_FloatBytes_t gy_y, union Utils_FloatBytes_t gy_z);
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
  osThreadDef(readIMUTask, read_IMU_task, osPriorityNormal, 0, 1024);
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
   IWDG_refresh_with_default_delay();
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
    
    IWDG_refresh();

    /*
     * Control Flow:
     * Wait for Flag from Interrupt
     * After flag occurs, read messages from queue repeatedly until it is empty
     * Once empty,    // HAL_StatusTypeDef imu_status = HAL_OK;

    /* Get CAN Message from Queue */
    while(true) {
      evt = osMessageGet(CAN_MSG_Rx_Queue, osWaitForever);

      if (evt.status == osEventMessage) {
        HAL_GPIO_TogglePin(USER_LED_GPIO_Port, USER_LED_Pin);
        rx_CAN_msg = evt.value.p;                       // Get pointer from the queue union
    
        RTC_check_and_sync_rtc(rx_CAN_msg);             // Sync RTC with memorator message. Also sets rtc reset

        CAN_Radio_msg_t tx_CAN_msg;
        CAN_rx_to_radio(rx_CAN_msg, &tx_CAN_msg);        // Convert CAN message to radio message
        RADIO_TRANSMIT_CAN_msg(&tx_CAN_msg);             // Send CAN on radio

        osPoolFree(CAN_MSG_memory_pool, rx_CAN_msg);    // Free the memory pool
      }
      else break;
    }
  }

  /* USER CODE END read_CAN_task */
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

    IMU_send_as_CAN_msg_with_delay(&imu_status);                // Send IMU data as CAN message
    g_tel_diagnostics.imu_fail = (imu_status != HAL_OK);        // Update diagnostics
    osDelay(IMU_SINGLE_DELAY);
    osDelay(IMU_SINGLE_DELAY * 2);
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
    GPS_receive_and_transmit_as_CAN();
    osDelay(10 * 1000);
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
    RADIO_TRANSMIT_diagnostic_msg();
    osDelay(TRANSMIT_DIAGNOSTICS_DELAY);
  }
  /* USER CODE END transmit_Diagnostics_task */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

