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
#include "radio.h"
#include "CAN_comms.h"

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

/* Create the Queue */
// https://community.st.com/t5/stm32-mcus-products/osmessageget-crashing/td-p/257324
// See this link for issues why osMessageGet may crash
// osMessageQDef needs a pointer to the structure type, which needs to be a "dummy", not pointer to typedef.
osMessageQDef(CAN_MSG_Rx_Queue, MAX_CAN_MSGS_IN_POOL, &msg_t);              // Define message queue
osMessageQId  CAN_MSG_Rx_Queue;

/* USER CODE END Variables */
/* Definitions for StartDefaultTask */
osThreadId_t StartDefaultTaskHandle;
const osThreadAttr_t StartDefaultTask_attributes = {
  .name = "StartDefaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for readIMUTask */
osThreadId_t readIMUTaskHandle;
const osThreadAttr_t readIMUTask_attributes = {
  .name = "readIMUTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for readGPSTask */
osThreadId_t readGPSTaskHandle;
const osThreadAttr_t readGPSTask_attributes = {
  .name = "readGPSTask",
  .stack_size = 1536 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for transmitDiagnosticsTask */
osThreadId_t transmitDiagnosticsTaskHandle;
const osThreadAttr_t transmitDiagnosticsTask_attributes = {
  .name = "transmitDiagnosticsTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void check_IMU_result(union Utils_FloatBytes_t ax_x, union Utils_FloatBytes_t ax_y, union Utils_FloatBytes_t ax_z,
		      union Utils_FloatBytes_t gy_x, union Utils_FloatBytes_t gy_y, union Utils_FloatBytes_t gy_z);
void send_CAN_Radio(CAN_Radio_msg_t *tx_CAN_msg);


/* USER CODE END FunctionPrototypes */

void startDefaultTask(void *argument);
void read_IMU_task(void *argument);
void read_GPS_task(void *argument);
void transmit_Diagnostics_task(void *argument);
void CAN_comms_Rx_callback(CAN_comms_Rx_msg_t CAN_comms_Rx_msg);


void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  CAN_FilterTypeDef CAN_filter = {
    .FilterIdHigh = 0x0000,
    .FilterMaskIdHigh = 0x0000,
    .FilterIdLow = 0x0000,
    .FilterMaskIdLow = 0x0000,
    .FilterFIFOAssignment = CAN_FILTER_FIFO0,
    .FilterBank = 0,
    .FilterMode = CAN_FILTERMODE_IDMASK,
    .FilterScale = CAN_FILTERSCALE_16BIT,
    .FilterActivation = CAN_FILTER_ENABLE
  };

  CAN_comms_config_t CAN_comms_config = {
    .hcan = &hcan,
    .CAN_Filter = CAN_filter,
    .CAN_comms_Rx_callback = CAN_comms_Rx_callback
  };

  CAN_comms_init(&CAN_comms_config);
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

  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of StartDefaultTask */
  StartDefaultTaskHandle = osThreadNew(startDefaultTask, NULL, &StartDefaultTask_attributes);

  /* creation of readIMUTask */
  readIMUTaskHandle = osThreadNew(read_IMU_task, NULL, &readIMUTask_attributes);

  /* creation of readGPSTask */
  readGPSTaskHandle = osThreadNew(read_GPS_task, NULL, &readGPSTask_attributes);

  /* creation of transmitDiagnosticsTask */
  transmitDiagnosticsTaskHandle = osThreadNew(transmit_Diagnostics_task, NULL, &transmitDiagnosticsTask_attributes);

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
  CAN_Init();

  /* Infinite loop */
  IWDG_inf_refresh_with_delay();

  /* USER CODE END startDefaultTask */
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
  while(1)
  {
    HAL_StatusTypeDef imu_status = HAL_OK;

    //IMU_send_as_CAN_msg_single_delay(&imu_status);                // Send IMU data as CAN message
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
void read_GPS_task(void *argument)
{
  /* USER CODE BEGIN read_GPS_task */
  //GPS_wait_for_fix();                     // Try to get a fix first.

  /* Infinite loop */
  while(1) {
    //GPS_delayed_rx_and_tx_as_CAN();       // Once a fix is obtained create and send GPS message as CAN
	  osDelay(1000);
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
void transmit_Diagnostics_task(void *argument)
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

void CAN_comms_Rx_callback(CAN_comms_Rx_msg_t* CAN_comms_Rx_msg)
{
  return; // Code for parsing the CAN RX msg
}

/* USER CODE END Application */

