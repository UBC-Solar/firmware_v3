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
#include "can.h"
#include "tel_freertos.h"
#include "string.h"


/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
typedef StaticTask_t osStaticThreadDef_t;
/* USER CODE BEGIN PTD */
typedef StaticQueue_t osStaticMessageQDef_t;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define CAN_RX_STRUCT_SIZE 		sizeof(CAN_Rx_msg_t)
#define CAN_TX_STRUCT_SIZE 		sizeof(CAN_Tx_msg_t)
#define CAN_RX_QUEUE_SIZE 		16
#define CAN_TX_QUEUE_SIZE		16
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* SEMAPHORES */
osSemaphoreId_t usart1_tx_semaphore;


/*	User Defined Tasks	*/
/* Definitions for CAN_rx_queue */
osMessageQueueId_t CAN_rx_queueHandle;
uint8_t CAN_rx_queueBuffer[ CAN_RX_QUEUE_SIZE * CAN_RX_STRUCT_SIZE ];
osStaticMessageQDef_t CAN_rx_queueControlBlock;
const osMessageQueueAttr_t CAN_rx_queue_attributes = {
  .name = "CAN_rx_queue",
  .cb_mem = &CAN_rx_queueControlBlock,
  .cb_size = sizeof(CAN_rx_queueControlBlock),
  .mq_mem = &CAN_rx_queueBuffer,
  .mq_size = sizeof(CAN_rx_queueBuffer)
};

osMessageQueueId_t CAN_tx_queueHandle;
uint8_t CAN_tx_queueBuffer[ CAN_TX_QUEUE_SIZE * CAN_TX_STRUCT_SIZE ];
osStaticMessageQDef_t CAN_tx_queueControlBlock;
const osMessageQueueAttr_t CAN_tx_queue_attributes = {
  .name = "CAN_tx_queue",
  .cb_mem = &CAN_tx_queueControlBlock,
  .cb_size = sizeof(CAN_tx_queueControlBlock),
  .mq_mem = &CAN_tx_queueBuffer,
  .mq_size = sizeof(CAN_tx_queueBuffer)
};

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for IMU_Task */
osThreadId_t IMU_TaskHandle;
const osThreadAttr_t IMU_Task_attributes = {
  .name = "IMU_Task",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for GPS_Task */
osThreadId_t GPS_TaskHandle;
const osThreadAttr_t GPS_Task_attributes = {
  .name = "GPS_Task",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for CAN_Rx_Task */
osThreadId_t CAN_Rx_TaskHandle;
uint32_t CAN_Rx_TaskBuffer[ 128 ];
osStaticThreadDef_t CAN_Rx_TaskControlBlock;
const osThreadAttr_t CAN_Rx_Task_attributes = {
  .name = "CAN_Rx_Task",
  .cb_mem = &CAN_Rx_TaskControlBlock,
  .cb_size = sizeof(CAN_Rx_TaskControlBlock),
  .stack_mem = &CAN_Rx_TaskBuffer[0],
  .stack_size = sizeof(CAN_Rx_TaskBuffer),
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for CAN_Tx_Task */
osThreadId_t CAN_Tx_TaskHandle;
uint32_t CAN_Tx_TaskBuffer[ 128 ];
osStaticThreadDef_t CAN_Tx_TaskControlBlock;
const osThreadAttr_t CAN_Tx_Task_attributes = {
  .name = "CAN_Tx_Task",
  .cb_mem = &CAN_Tx_TaskControlBlock,
  .cb_size = sizeof(CAN_Tx_TaskControlBlock),
  .stack_mem = &CAN_Tx_TaskBuffer[0],
  .stack_size = sizeof(CAN_Tx_TaskBuffer),
  .priority = (osPriority_t) osPriorityLow,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */


/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void IMU_task(void *argument);
void GPS_task(void *argument);
void CAN_Rx_task(void *argument);
void CAN_Tx_task(void *argument);

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

    usart1_tx_semaphore = osSemaphoreNew(NUM_USART1_TX_SEMAPHORES, NUM_USART1_TX_SEMAPHORES, NULL);

  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
    /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
    /* add queues, ... */
  CAN_rx_queueHandle = osMessageQueueNew (CAN_RX_QUEUE_SIZE, CAN_RX_STRUCT_SIZE, &CAN_rx_queue_attributes);
  CAN_tx_queueHandle = osMessageQueueNew(CAN_TX_QUEUE_SIZE, CAN_TX_STRUCT_SIZE, &CAN_tx_queue_attributes);
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of IMU_Task */
  IMU_TaskHandle = osThreadNew(IMU_task, NULL, &IMU_Task_attributes);

  /* creation of GPS_Task */
  GPS_TaskHandle = osThreadNew(GPS_task, NULL, &GPS_Task_attributes);

  /* creation of CAN_Rx_Task */
  CAN_Rx_TaskHandle = osThreadNew(CAN_Rx_task, NULL, &CAN_Rx_Task_attributes);

  /* creation of CAN_Tx_Task */
  CAN_Tx_TaskHandle = osThreadNew(CAN_Tx_task, NULL, &CAN_Tx_Task_attributes);

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
        IWDG_Refresh(&hiwdg);	                                 // Refresh the IWDG to ensure no reset occurs
        osDelay(REFRESH_DELAY);
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
	CAN_TxHeaderTypeDef my_header = {
	    .StdId = 0x111,
	    .ExtId = 0x0000,
	    .IDE = CAN_ID_STD,
	    .RTR = CAN_RTR_DATA,
	    .DLC = 8};
	uint8_t my_data[8] = {0};
	CAN_Tx_msg_t my_message;

	my_message.header = my_header;
	memcpy(my_message.data, my_data, 8);

  /* Infinite loop */
  for(;;)
  {

	  CAN_Add_Message(&my_message);
	  osDelay(100);
  }
  /* USER CODE END GPS_task */
}

/* USER CODE BEGIN Header_CAN_Rx_task */
/**
* @brief Function implementing the CAN_Rx_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_CAN_Rx_task */
void CAN_Rx_task(void *argument)
{
  /* USER CODE BEGIN CAN_Rx_task */
	CAN_Init();
  /* Infinite loop */
  for(;;)
  {
	  CAN_Rx_msg_t CAN_Rx_msg;
	  if (osOK == osMessageQueueGet(CAN_rx_queueHandle, &CAN_Rx_msg, NULL, osWaitForever))
	  {
		  can_diagnostic.success_rx++;
		  CAN_Rx_callback(&CAN_Rx_msg);
	  }


  }
  /* USER CODE END CAN_Rx_task */
}

/* USER CODE BEGIN Header_CAN_Tx_task */
/**
* @brief Function implementing the CAN_Tx_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_CAN_Tx_task */
void CAN_Tx_task(void *argument)
{
  /* USER CODE BEGIN CAN_Tx_task */
  /* Infinite loop */
  for(;;)
  {
	  CAN_Tx_msg_t CAN_Tx_msg;

	  if (osOK == osMessageQueueGet(CAN_tx_queueHandle, &CAN_Tx_msg, NULL, osWaitForever))
	  {
		  uint32_t can_mailbox;

		  if (HAL_OK == HAL_CAN_AddTxMessage(&hcan, &CAN_Tx_msg.header, CAN_Tx_msg.data, &can_mailbox))
		  {
			  can_diagnostic.success_tx++;
		  }
		  else
		  {
			  can_diagnostic.hal_failure_tx++;
		  }
	  }


  }
  /* USER CODE END CAN_Tx_task */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

