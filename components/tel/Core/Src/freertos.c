/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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

#include "iwdg.h"
#include "tel_freertos.h"
#include "canload.h"
#include "can.h"
#include "cpu_load.h"
#include "radio.h"
#include "gps.h"
#include "nmea_parse.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
typedef StaticTask_t osStaticThreadDef_t;
/* USER CODE BEGIN PTD */

typedef StaticTask_t osStaticMessageQDef_t;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define WINDOW_SIZE 10
#define FREQUENCY_MS 100
#define NUM_USART1_TX_SEMAPHORES        1

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* SEMAPHORES */
osSemaphoreId_t usart1_tx_semaphore;

/* QUEUES */
osMessageQueueId_t radio_tx_queue;
uint8_t radio_tx_queue_buffer[ RADIO_QUEUE_SIZE * RADIO_MSG_TYPEDEF_SIZE ];
osStaticMessageQDef_t radio_tx_queue_cb;
const osMessageQueueAttr_t radio_tx_queue_attributes = {
  .name = "radio_tx_queue",
  .cb_mem = &radio_tx_queue_cb,
  .cb_size = sizeof(radio_tx_queue_cb),
  .mq_mem = &radio_tx_queue_buffer,
  .mq_size = sizeof(radio_tx_queue_buffer)
};

/* CPU Load Config */


/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
uint32_t defaultTaskBuffer[ 128 ];
osStaticThreadDef_t defaultTaskControlBlock;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .cb_mem = &defaultTaskControlBlock,
  .cb_size = sizeof(defaultTaskControlBlock),
  .stack_mem = &defaultTaskBuffer[0],
  .stack_size = sizeof(defaultTaskBuffer),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for IMU_Task */
osThreadId_t IMU_TaskHandle;
uint32_t IMU_TaskBuffer[ 128 ];
osStaticThreadDef_t IMU_TaskControlBlock;
const osThreadAttr_t IMU_Task_attributes = {
  .name = "IMU_Task",
  .cb_mem = &IMU_TaskControlBlock,
  .cb_size = sizeof(IMU_TaskControlBlock),
  .stack_mem = &IMU_TaskBuffer[0],
  .stack_size = sizeof(IMU_TaskBuffer),
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for GPS_Task */
osThreadId_t GPS_TaskHandle;
uint32_t GPS_TaskBuffer[ 128 ];
osStaticThreadDef_t GPS_TaskControlBlock;
const osThreadAttr_t GPS_Task_attributes = {
  .name = "GPS_Task",
  .cb_mem = &GPS_TaskControlBlock,
  .cb_size = sizeof(GPS_TaskControlBlock),
  .stack_mem = &GPS_TaskBuffer[0],
  .stack_size = sizeof(GPS_TaskBuffer),
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for CANLoad_Task */
osThreadId_t CANLoad_TaskHandle;
uint32_t CANLoad_TaskBuffer[ 128 ];
osStaticThreadDef_t CANLoad_TaskControlBlock;
const osThreadAttr_t CANLoad_Task_attributes = {
  .name = "CANLoad_Task",
  .cb_mem = &CANLoad_TaskControlBlock,
  .cb_size = sizeof(CANLoad_TaskControlBlock),
  .stack_mem = &CANLoad_TaskBuffer[0],
  .stack_size = sizeof(CANLoad_TaskBuffer),
  .priority = (osPriority_t) osPriorityLow,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void IMU_task(void *argument);
void GPS_task(void *argument);
void CANLoad_task(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

	CPU_LOAD_config_t user_config = {
	    .window_size = WINDOW_SIZE,
	    .frequency_ms = FREQUENCY_MS,
	    .timer = htim2
	};

    CAN_tasks_init();                         // Rx CAN Filter, Rx callback using CAN comms
    CPU_LOAD_init(&user_config);

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
    /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
    /* add semaphores, ... */

  usart1_tx_semaphore = osSemaphoreNew(NUM_USART1_TX_SEMAPHORES, NUM_USART1_TX_SEMAPHORES, NULL);

  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
    /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
    /* add queues, ... */

  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of IMU_Task */
  IMU_TaskHandle = osThreadNew(IMU_task, NULL, &IMU_Task_attributes);

  /* creation of GPS_Task */
  GPS_TaskHandle = osThreadNew(GPS_task, NULL, &GPS_Task_attributes);

  /* creation of CANLoad_Task */
  CANLoad_TaskHandle = osThreadNew(CANLoad_task, NULL, &CANLoad_Task_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
    /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
    /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */

    /* Infinite loop */
    for(;;)
    {
		CAN_cpu_load_can_tx();
        IWDG_Refresh(&hiwdg);	                                 // Refresh the IWDG to ensure no reset occurs
        osDelay(REFRESH_DELAY_MS);

    }

  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_IMU_task */
/**
* @brief Function implementing the IMU_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_IMU_task */
void IMU_task(void *argument)
{
  /* USER CODE BEGIN IMU_task */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END IMU_task */
}

/* USER CODE BEGIN Header_GPS_task */
/**
* @brief Function implementing the GPS_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_GPS_task */
void GPS_task(void *argument)
{
  /* USER CODE BEGIN GPS_task */
  /* Infinite loop */
  for(;;)
  {
    osDelay(500);

    uint8_t msg[20] = "New GPS read\r\n\n";
    HAL_UART_Transmit(&huart5, msg, 20, 100);

    memset(g_gps_data, 0, GPS_MESSAGE_LEN);

    read_i2c_gps_module(g_gps_data);

    osDelay(500);

   if(g_gps_read_okay)
   {
     HAL_UART_Transmit(&huart5, g_gps_data, GPS_MESSAGE_LEN, 100);

     GPS gps_data = {0};

     nmea_parse(&gps_data, g_gps_data);

     CAN_tx_gps_data_msg(&gps_data);

     g_gps_read_okay = false;
   }
   else
   {
     strncpy(g_gps_data, "GPS not connected\r\n", GPS_MESSAGE_LEN);
     HAL_UART_Transmit(&huart5, g_gps_data, GPS_MESSAGE_LEN, 100);
   }

    HAL_GPIO_TogglePin(USER_LED_GPIO_Port, USER_LED_Pin);
  }
  /* USER CODE END GPS_task */
}

/* USER CODE BEGIN Header_CANLoad_task */
/**
* @brief Function implementing the CANLoad_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_CANLoad_task */
void CANLoad_task(void *argument)
{
  /* USER CODE BEGIN CANLoad_task */
  /* Infinite loop */
  for(;;)
  {
   CANLOAD_update_sliding_window();
   CAN_tx_canload_msg();
   osDelay(CANLOAD_MSG_RATE);
  }
  /* USER CODE END CANLoad_task */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

