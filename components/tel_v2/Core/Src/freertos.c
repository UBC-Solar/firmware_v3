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
//#include "sdcard.h"     // From sdCardLib in "Libraries"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define KERNEL_LED_DELAY 200		// 200 milliseconds
#define READ_IMU_DELAY 	 100		// 100 milliseconds
#define READ_GPS_DELAY   5 * 60 * 1000  // 5 minutes
#define TRANSMIT_RTC_DELAY 5000 // 5000 milliseconds

#define CAN_BUFFER_LEN  22
#define IMU_MESSAGE_LEN 17
#define GPS_MESSAGE_LEN 200

#define GPS_RCV_BUFFER_SIZE 512


union FloatBytes {
    float float_value;
    uint8_t bytes[4];
} FloatBytes;

/* Use this MACRO to split the time stamp into individual bytes */
#define TIMESTAMP_BYTE(i, timestamp) ((timestamp >> (i * 8)) & 0xFF);

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
  .stack_size = 1024 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for transmitGPSTask */
osThreadId_t transmitGPSTaskHandle;
const osThreadAttr_t transmitGPSTask_attributes = {
  .name = "transmitGPSTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for transmitRTCTask */
osThreadId_t transmitRTCTaskHandle;
const osThreadAttr_t transmitRTCTask_attributes = {
  .name = "transmitRTCTask",
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
/* Definitions for gpsMessageQueue */
osMessageQueueId_t gpsMessageQueueHandle;
const osMessageQueueAttr_t gpsMessageQueue_attributes = {
  .name = "gpsMessageQueue"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

void add_to_IMU_queue(char* type, char* dimension, union FloatBytes data);
void add_to_GPS_queue(GPS *gps_data);

/* USER CODE END FunctionPrototypes */

void startDefaultTask(void *argument);
void read_CAN_task(void *argument);
void transmit_CAN_task(void *argument);
void read_IMU_task(void *argument);
void transmit_IMU_task(void *argument);
void read_GPS_task(void *argument);
void transmit_GPS_task(void *argument);
void transmit_RTC_task(void *argument);

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

  /* creation of gpsMessageQueue */
  gpsMessageQueueHandle = osMessageQueueNew (10, sizeof(uint16_t), &gpsMessageQueue_attributes);

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

  /* creation of transmitRTCTask */
  transmitRTCTaskHandle = osThreadNew(transmit_RTC_task, NULL, &transmitRTCTask_attributes);

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

    HAL_GPIO_TogglePin(USER_LED_GPIO_Port, USER_LED_Pin);
    osDelay(100);
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

  static CAN_msg_t can_message;	/* Can message */
  osStatus_t queue_status;	/* CAN Message Queue Status */

  /* Infinite loop */
  while (1) {

    /* Check if there are messages in the queue */
    if (osMessageQueueGetCount(canMessageQueueHandle) == 0) {
      osThreadYield();
    }

    /* Retrieve CAN message from queue */
    queue_status = osMessageQueueGet(canMessageQueueHandle, &can_message, NULL, osWaitForever);

    /* Yield if nothing on queue */
    if (queue_status != osOK){
      osThreadYield();
    }

    /* Initialize a CAN buffer */
    uint8_t can_buffer[CAN_BUFFER_LEN] = {0};

    /* Get current epoch Time Stamp */
    time_t current_timestamp = get_current_timestamp();

    /* TIMESTAMP: 8 Bytes */
    for (uint8_t i=0; i<CAN_BUFFER_LEN - 14; i++) {
      /* Put each byte in position 'i' */
      can_buffer[i] = TIMESTAMP_BYTE(i, current_timestamp);
    }

    /* CAN MESSAGE IDENTIFIER */
    can_buffer[CAN_BUFFER_LEN - 14] = '#';

    /* CAN ID: 4 ASCII characters */
    uint8_t id_h = 0xFFUL & (can_message.header.StdId >> 8);
    uint8_t id_l = 0xFFUL & (can_message.header.StdId);

    can_buffer[CAN_BUFFER_LEN - 13] = id_h;
    can_buffer[CAN_BUFFER_LEN - 12] = id_l;


    /* CAN DATA: 16 ASCII characters */
    for (uint8_t i=0; i<8; i++) {
      /* Copy each byte */
      can_buffer[i + CAN_BUFFER_LEN - 11]= can_message.data[i];
    }


    /* CAN DATA LENGTH: 1 ASCII character */
    uint8_t length = "0123456789ABCDEF"[ can_message.header.DLC & 0xFUL];
    can_buffer[CAN_BUFFER_LEN - 3] = length;

    /* NEW LINE: 1 ASCII character */
    can_buffer[CAN_BUFFER_LEN - 2] = '\r';

    /* CARRIAGE RETURN: 1 ASCII character */
    can_buffer[CAN_BUFFER_LEN - 1] = '\n';

    /* Transmit over Radio */
    HAL_UART_Transmit(&huart1, can_buffer, sizeof(can_buffer), 1000);

    /* TODO: Log to SDLogger */
//    FIL *can_file_ptr = sd_open("CAN_Messages.txt");
//    sd_append(can_file_ptr, can_buffer);    // Append can message to the SD card
//    sd_close(can_file_ptr);

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

  osStatus_t imu_queue_status;	/* IMU Queue Status */
  IMU_msg_t imu_message;	/* IMU Message */

  /* Infinite loop */
  while(1)
  {
    /* Check if there are messages in the queue */
    if (osMessageQueueGetCount(imuMessageQueueHandle) == 0) {
	osThreadYield();
    }

    /* Get IMU Message from Queue */
    imu_queue_status = osMessageQueueGet(imuMessageQueueHandle, &imu_message, NULL, osWaitForever);

    /* Yield thread if status not ok */
    if (imu_queue_status != osOK){
      osThreadYield();
    }

    /* Initialize a IMU buffer */
    uint8_t imu_buffer[IMU_MESSAGE_LEN] = {0};

    /* Get current epoch Time Stamp */
    time_t current_timestamp = get_current_timestamp();

    /* TIMESTAMP: 8 Bytes */
    for (uint8_t i=0; i<IMU_MESSAGE_LEN - 9; i++) {
      /* Put each byte in position 'i' */
      imu_buffer[i] = TIMESTAMP_BYTE(i, current_timestamp);
    }

    /* IMU ID */
    imu_buffer[IMU_MESSAGE_LEN - 9] = '@';

    /* IMU Data from queue */
    imu_buffer[IMU_MESSAGE_LEN - 8] = imu_message.imu_type;
    imu_buffer[IMU_MESSAGE_LEN - 7] = imu_message.dimension;

    /* Copy data */
    for (int i = 0; i < 4; i++) {
	imu_buffer[i + IMU_MESSAGE_LEN - 6] = imu_message.data[i];
    }

    /* NEW LINE */
    imu_buffer[IMU_MESSAGE_LEN - 2] = '\r';

    /* CARRIAGE RETURN */
    imu_buffer[IMU_MESSAGE_LEN - 1] = '\n';

    /* Transmit over Radio */
    HAL_UART_Transmit(&huart1, imu_buffer, sizeof(imu_buffer), 1000);

    /* TODO: Log to SDLogger */
//    FIL *imu_file_ptr = sd_open("IMU_Messages.txt");
//    sd_append(imu_file_ptr, imu_buffer);    // Append imu message to the SD card
//    sd_close(imu_file_ptr);

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

  osStatus_t nmea_queue_status;
  GPS_msg_t gps_message;

  /* Infinite loop */
  while(1) {

    /* Check if there are messages in the queue */
    if (osMessageQueueGetCount(gpsMessageQueueHandle) == 0) {
      osThreadYield(); // Yield to other tasks if the queue is empty
    }

    /* Get a message from the queue */
    nmea_queue_status = osMessageQueueGet(gpsMessageQueueHandle, &gps_message, NULL, osWaitForever);

    /* Check if the queue status is OK */
    if (nmea_queue_status != osOK){
      /* If message retrieval failed, yield and continue the loop */
      osThreadYield();
      continue; // Skip the rest of this loop iteration
    }

    /* Initialize an NMEA buffer */
    uint8_t gps_buffer[GPS_MESSAGE_LEN] = {0};

    /* Get current epoch Time Stamp */
    time_t current_timestamp = get_current_timestamp();

    /* TIMESTAMP: 8 Bytes */
    for (uint8_t i=0; i<8; i++) {
      /* Put each byte in position 'i' */
      gps_buffer[i] = TIMESTAMP_BYTE(i, current_timestamp);
    }

    /*
     * Copy the NMEA data into the buffer, ensuring not to exceed the buffer size
     * Adds 8 to the start to skip the time stamp
     */
    strncpy(gps_buffer + 8, gps_message.data, 150); // Save space for CR+LF

    /* NEW LINE */
    gps_buffer[GPS_MESSAGE_LEN - 2] = '\r'; // Carriage return

    /* CARRIAGE RETURN */
    gps_buffer[GPS_MESSAGE_LEN - 1] = '\n'; // Line feed

    /* Transmit the NMEA message over UART to radio */
    HAL_UART_Transmit(&huart1, gps_buffer, sizeof(gps_buffer), 1000);

    /* TODO: Log to SDLogger */
//    FIL *gps_file_ptr = sd_open("GPS_Messages.txt");
//    sd_append(gps_file_ptr, gps_buffer);    // Append gps message to the SD card
//    sd_close(gps_file_ptr);

  }

  /* USER CODE END transmit_GPS_task */
}

/* USER CODE BEGIN Header_transmit_RTC_task */
/**
* @brief Function implementing the transmitRTCTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_transmit_RTC_task */
void transmit_RTC_task(void *argument)
{
  /* USER CODE BEGIN transmit_RTC_task */
  /* Infinite loop */
  for(;;)
  {
    // Get rtc timestamp
    time_t timestamp = get_current_timestamp();
    uint8_t data_send[8];

    // Populate data_send array
    for (int i = 0; i < 8; i++) {
        data_send[i] = (timestamp >> (8 * i)) & 0xFF;
    }
    
    // Transmit message on CAN
    HAL_CAN_AddTxMessage(&hcan, &rtc_timestamp_header, data_send, &can_mailbox);
    
    osDelay(TRANSMIT_RTC_DELAY);
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

