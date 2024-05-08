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
#include "can.h"
#include "gpio.h"
#include "spi.h"
#include "usart.h"
#include "i2c.h"
#include "nmea_parse.h"
#include "sdcard.h"     // From sdCardLib in "Libraries"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define KERNEL_LED_DELAY 200		// 200 milliseconds
#define READ_IMU_DELAY 	 100		// 100 milliseconds
#define READ_GPS_DELAY   5 * 60 * 1000  // 5 minutes

#define CAN_BUFFER_LEN  22
#define IMU_MESSAGE_LEN 17
#define GPS_MESSAGE_LEN 200

#define GPS_RCV_BUFFER_SIZE 512


union FloatBytes {
    float float_value;
    uint8_t bytes[4];
} FloatBytes;

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
osThreadId StartDefaultTaskHandle;
osThreadId readCANTaskHandle;
osThreadId readIMUTaskHandle;
osThreadId readGPSTaskHandle;
osThreadId transmitRTCTaskHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

void add_to_IMU_queue(char* type, char* dimension, union FloatBytes data);
void add_to_GPS_queue(GPS *gps_data);

/* USER CODE END FunctionPrototypes */

void startDefaultTask(void const * argument);
void read_CAN_task(void const * argument);
void read_IMU_task(void const * argument);
void read_GPS_task(void const * argument);
void transmit_RTC_task(void const * argument);

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
  osThreadDef(readGPSTask, read_GPS_task, osPriorityNormal, 0, 1024);
  readGPSTaskHandle = osThreadCreate(osThread(readGPSTask), NULL);

  /* definition and creation of transmitRTCTask */
  osThreadDef(transmitRTCTask, transmit_RTC_task, osPriorityNormal, 0, 512);
  transmitRTCTaskHandle = osThreadCreate(osThread(transmitRTCTask), NULL);

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
void read_CAN_task(void const * argument)
{
  /* USER CODE BEGIN read_CAN_task */

  static HAL_StatusTypeDef rx_status;	/* CAN Rx Status */
  static CAN_msg_t current_can_message; /* CAN message data */

  /* Infinite loop */
  while (1) {

    /* Wait for thread flags to be set in the CAN Rx FIFO0 Interrupt Callback */
    osThreadFlagsWait(CAN_READY, osFlagsWaitAll, osWaitForever);

    /* If CAN Rx FIFO0 is not empty */
    if (HAL_CAN_GetRxFifoFillLevel(&hcan, CAN_RX_FIFO0) != 0) {

      /* There are multiple CAN IDs being passed through the filter, pull out the current message */
      rx_status = HAL_CAN_GetRxMessage(&hcan, CAN_RX_FIFO0, &can_rx_header, current_can_data);

      /* Check the rx status */
      if (rx_status != HAL_OK) {
	  Error_Handler();
      }

      /* Package into CAN_msg_t */
      current_can_message.header = can_rx_header;

      /* Copy all bytes of data */
      for (uint8_t i = 0; i < 8; i++) {
	current_can_message.data[i] = current_can_data[i];
      }

      /* Add CAN message onto canMessageQueue */
      osMessageQueuePut(canMessageQueueHandle, &current_can_message, 0U, 0U);
    }

    /* Enables Interrupts */
    HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING);

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

  union FloatBytes gy_x, gy_y, gy_z, ax_x, ax_y, ax_z;

  /* Infinite loop */
  while(1)
  {
    /* Get Data */
    gy_x.float_value = gyro(GYRO_X);
    gy_y.float_value = gyro(GYRO_Y);
    gy_z.float_value = gyro(GYRO_Z);
    ax_x.float_value = accel(ACCEL_X);
    ax_y.float_value = accel(ACCEL_Y);
    ax_z.float_value = accel(ACCEL_Z);

    /* Add to IMU Queue */
    add_to_IMU_queue("G", "X", gy_x);
    add_to_IMU_queue("G", "Y", gy_y);
    add_to_IMU_queue("G", "Z", gy_z);
    add_to_IMU_queue("A", "X", ax_x);
    add_to_IMU_queue("A", "Y", ax_y);
    add_to_IMU_queue("A", "Z", ax_z);

    /* Delay */
    osDelay(READ_IMU_DELAY);

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

    /* Initialize a receive buffer */
    uint8_t receive_buffer[GPS_RCV_BUFFER_SIZE];

    /* Initialize a GPS data struct */
    GPS parsed_GPS_data;

    /* Read in an NMEA message into the buffer */
    readNMEA(&receive_buffer);

    /* Parse the buffer data --> gets stored in parsed_GPS_data */
    nmea_parse(&parsed_GPS_data, &receive_buffer);

    /* Add to the GPS Queue */
    add_to_GPS_queue(&parsed_GPS_data);

    /* Delay */
    osDelay(READ_GPS_DELAY);

  }

  /* USER CODE END read_GPS_task */
}

/* USER CODE BEGIN Header_transmit_RTC_task */
/**
* @brief Function implementing the transmitRTCTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_transmit_RTC_task */
void transmit_RTC_task(void const * argument)
{
  /* USER CODE BEGIN transmit_RTC_task */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END transmit_RTC_task */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */


/*
 * Stores the data gathered from the IMU into the queue
 */
void add_to_IMU_queue(char* type, char* dimension, union FloatBytes data){
    IMU_msg_t imu_message;

    imu_message.imu_type = type[0];
    imu_message.dimension = dimension[0];
    for (int i = 0; i < 4; i++) {
        imu_message.data[i] = data.bytes[i];
    }

    osMessageQueuePut(imuMessageQueueHandle, &imu_message, 0U, 0U);
}

/*
 * Stores the data gathered from the GPS into the queue
 */
void add_to_GPS_queue(GPS *gps_data) {

  /* If there is a fix... */
  if(gps_data->fix == 1){

    /* Create an nmea_msg */
    GPS_msg_t nmea_msg;

    /* Create string */
    sprintf(nmea_msg.data,
	    "Latitude: %.6f %c, Longitude: %.6f %c, Altitude: %.2f meters, HDOP: %.2f, Satellites: %d, Fix: %d, Time: %s",
	    gps_data->latitude, gps_data->latSide,
	    gps_data->longitude, gps_data->lonSide,
	    gps_data->altitude, gps_data->hdop,
	    gps_data->satelliteCount, gps_data->fix,
	    gps_data->lastMeasure);

    /* Null Terminate */
    nmea_msg.data[sizeof(nmea_msg.data) - 1] = '\0';

    /* Add the message to the queue */
    osMessageQueuePut(gpsMessageQueueHandle, &nmea_msg, 0U, 0U);
  }
}


/* USER CODE END Application */

