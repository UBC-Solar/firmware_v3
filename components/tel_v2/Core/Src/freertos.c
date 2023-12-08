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

/* USER CODE END Variables */
/* Definitions for StartDefaultTask */
osThreadId_t StartDefaultTaskHandle;
const osThreadAttr_t StartDefaultTask_attributes = {
  .name = "StartDefaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for readCANTask */
osThreadId_t readCANTaskHandle;
const osThreadAttr_t readCANTask_attributes = {
  .name = "readCANTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for transmitCANTask */
osThreadId_t transmitCANTaskHandle;
const osThreadAttr_t transmitCANTask_attributes = {
  .name = "transmitCANTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for readIMUTask */
osThreadId_t readIMUTaskHandle;
const osThreadAttr_t readIMUTask_attributes = {
  .name = "readIMUTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for transmitIMUTask */
osThreadId_t transmitIMUTaskHandle;
const osThreadAttr_t transmitIMUTask_attributes = {
  .name = "transmitIMUTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for readGPSTask */
osThreadId_t readGPSTaskHandle;
const osThreadAttr_t readGPSTask_attributes = {
  .name = "readGPSTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for transmitGPSTask */
osThreadId_t transmitGPSTaskHandle;
const osThreadAttr_t transmitGPSTask_attributes = {
  .name = "transmitGPSTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for kernelLEDTask */
osThreadId_t kernelLEDTaskHandle;
const osThreadAttr_t kernelLEDTask_attributes = {
  .name = "kernelLEDTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for canMessageQueue */
osMessageQueueId_t canMessageQueueHandle;
const osMessageQueueAttr_t canMessageQueue_attributes = {
  .name = "canMessageQueue"
};
/* Definitions for imuMessageQueue */
osMessageQueueId_t imuMessageQueueHandle;
const osMessageQueueAttr_t imuMessageQueue_attributes = {
  .name = "imuMessageQueue"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void startDefaultTask(void *argument);
void read_CAN_task(void *argument);
void transmit_CAN_task(void *argument);
void read_IMU_task(void *argument);
void transmit_IMU_task(void *argument);
void read_GPS_task(void *argument);
void transmit_GPS_task(void *argument);
void kernel_LED_task(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

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

  /* Create the queue(s) */
  /* creation of canMessageQueue */
  canMessageQueueHandle = osMessageQueueNew (10, sizeof(uint16_t), &canMessageQueue_attributes);

  /* creation of imuMessageQueue */
  imuMessageQueueHandle = osMessageQueueNew (10, sizeof(uint16_t), &imuMessageQueue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of StartDefaultTask */
  StartDefaultTaskHandle = osThreadNew(startDefaultTask, NULL, &StartDefaultTask_attributes);

  /* creation of readCANTask */
  readCANTaskHandle = osThreadNew(read_CAN_task, NULL, &readCANTask_attributes);

  /* creation of transmitCANTask */
  transmitCANTaskHandle = osThreadNew(transmit_CAN_task, NULL, &transmitCANTask_attributes);

  /* creation of readIMUTask */
  readIMUTaskHandle = osThreadNew(read_IMU_task, NULL, &readIMUTask_attributes);

  /* creation of transmitIMUTask */
  transmitIMUTaskHandle = osThreadNew(transmit_IMU_task, NULL, &transmitIMUTask_attributes);

  /* creation of readGPSTask */
  readGPSTaskHandle = osThreadNew(read_GPS_task, NULL, &readGPSTask_attributes);

  /* creation of transmitGPSTask */
  transmitGPSTaskHandle = osThreadNew(transmit_GPS_task, NULL, &transmitGPSTask_attributes);

  /* creation of kernelLEDTask */
  kernelLEDTaskHandle = osThreadNew(kernel_LED_task, NULL, &kernelLEDTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_startDefaultTask */
/**
  * @brief  Function implementing the StartDefaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_startDefaultTask */
void startDefaultTask(void *argument)
{
  /* USER CODE BEGIN startDefaultTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
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
void read_CAN_task(void *argument)
{
  /* USER CODE BEGIN read_CAN_task */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END read_CAN_task */
}

/* USER CODE BEGIN Header_transmit_CAN_task */
/**
* @brief Function implementing the transmitCANTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_transmit_CAN_task */
void transmit_CAN_task(void *argument)
{
  /* USER CODE BEGIN transmit_CAN_task */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END transmit_CAN_task */
}

/* USER CODE BEGIN Header_read_IMU_task */
/**
* @brief Function implementing the readIMUTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_read_IMU_task */
void read_IMU_task(void *argument)
{
  /* USER CODE BEGIN read_IMU_task */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END read_IMU_task */
}

/* USER CODE BEGIN Header_transmit_IMU_task */
/**
* @brief Function implementing the transmitIMUTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_transmit_IMU_task */
void transmit_IMU_task(void *argument)
{
  /* USER CODE BEGIN transmit_IMU_task */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END transmit_IMU_task */
}

/* USER CODE BEGIN Header_read_GPS_task */
/**
* @brief Function implementing the readGPSTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_read_GPS_task */
void read_GPS_task(void *argument)
{
  /* USER CODE BEGIN read_GPS_task */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END read_GPS_task */
}

/* USER CODE BEGIN Header_transmit_GPS_task */
/**
* @brief Function implementing the transmitGPSTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_transmit_GPS_task */
void transmit_GPS_task(void *argument)
{
  /* USER CODE BEGIN transmit_GPS_task */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END transmit_GPS_task */
}

/* USER CODE BEGIN Header_kernel_LED_task */
/**
  * @brief  Function implementing the kernelLEDTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_kernel_LED_task */
void kernel_LED_task(void *argument)
{
  /* USER CODE BEGIN kernel_LED_task */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END kernel_LED_task */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */


/* USER CODE END Application */

