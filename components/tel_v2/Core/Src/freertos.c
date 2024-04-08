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

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define KERNEL_LED_DELAY 200		// 200 milliseconds
#define READ_IMU_DELAY 	 100		// 100 milliseconds
#define READ_GPS_DELAY   5 * 1000  // 5 seconds (change to 5 minutes later)
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
osThreadId StartDefaultTaskHandle;
osThreadId readCANTaskHandle;
osThreadId transmitCANTaskHandle;
osThreadId readIMUTaskHandle;
osThreadId transmitIMUTaskHandle;
osThreadId readGPSTaskHandle;
osThreadId transmitGPSTaskHandle;
osThreadId transmitRTCTaskHandle;
osMessageQId canMessageQueueHandle;
osMessageQId imuMessageQueueHandle;
osMessageQId gpsMessageQueueHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

void add_to_IMU_queue(char* type, char* dimension, union FloatBytes data);
void add_to_GPS_queue(GPS *gps_data);
void check_IMU_result(union FloatBytes ax_x, union FloatBytes ax_y, union FloatBytes ax_z,
		      union FloatBytes gy_x, union FloatBytes gy_y, union FloatBytes gy_z);

/* USER CODE END FunctionPrototypes */

void startDefaultTask(void const * argument);
void read_CAN_task(void const * argument);
void transmit_CAN_task(void const * argument);
void read_IMU_task(void const * argument);
void transmit_IMU_task(void const * argument);
void read_GPS_task(void const * argument);
void transmit_GPS_task(void const * argument);
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

  /* Create the queue(s) */
  /* definition and creation of canMessageQueue */
  osMessageQDef(canMessageQueue, 10, uint16_t);
  canMessageQueueHandle = osMessageCreate(osMessageQ(canMessageQueue), NULL);

  /* definition and creation of imuMessageQueue */
  osMessageQDef(imuMessageQueue, 10, uint16_t);
  imuMessageQueueHandle = osMessageCreate(osMessageQ(imuMessageQueue), NULL);

  /* definition and creation of gpsMessageQueue */
  osMessageQDef(gpsMessageQueue, 10, uint16_t);
  gpsMessageQueueHandle = osMessageCreate(osMessageQ(gpsMessageQueue), NULL);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of StartDefaultTask */
  osThreadDef(StartDefaultTask, startDefaultTask, osPriorityNormal, 0, 128);
  StartDefaultTaskHandle = osThreadCreate(osThread(StartDefaultTask), NULL);

  /* definition and creation of readCANTask */
  osThreadDef(readCANTask, read_CAN_task, osPriorityNormal, 0, 128);
  readCANTaskHandle = osThreadCreate(osThread(readCANTask), NULL);

  /* definition and creation of transmitCANTask */
  osThreadDef(transmitCANTask, transmit_CAN_task, osPriorityNormal, 0, 128);
  transmitCANTaskHandle = osThreadCreate(osThread(transmitCANTask), NULL);

  /* definition and creation of readIMUTask */
  osThreadDef(readIMUTask, read_IMU_task, osPriorityNormal, 0, 128);
  readIMUTaskHandle = osThreadCreate(osThread(readIMUTask), NULL);

  /* definition and creation of transmitIMUTask */
  osThreadDef(transmitIMUTask, transmit_IMU_task, osPriorityNormal, 0, 128);
  transmitIMUTaskHandle = osThreadCreate(osThread(transmitIMUTask), NULL);

  /* definition and creation of readGPSTask */
  osThreadDef(readGPSTask, read_GPS_task, osPriorityNormal, 0, 1024);
  readGPSTaskHandle = osThreadCreate(osThread(readGPSTask), NULL);

  /* definition and creation of transmitGPSTask */
  osThreadDef(transmitGPSTask, transmit_GPS_task, osPriorityNormal, 0, 512);
  transmitGPSTaskHandle = osThreadCreate(osThread(transmitGPSTask), NULL);

  /* definition and creation of transmitRTCTask */
  osThreadDef(transmitRTCTask, transmit_RTC_task, osPriorityNormal, 0, 128);
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

    HAL_GPIO_TogglePin(USER_LED_GPIO_Port, USER_LED_Pin);
    printf("Default task toggle pin\n\r");
    osDelay(1000);
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
    osSignalWait(CAN_READY, osWaitForever);

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
      osMailPut(canMessageQueueHandle, &current_can_message);
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
void transmit_CAN_task(void const * argument)
{
  /* USER CODE BEGIN transmit_CAN_task */

  CAN_msg_t *can_message;	/* Can message */
  osEvent rcv_message;	/* CAN Message Queue Status */

  /* Infinite loop */
  while (1) {

    /* Check if there are messages in the queue */
//    if (osMessageGetCount(canMessageQueueHandle) == 0) {
//      osThreadYield();
//    }

    // https://www.keil.com/pack/doc/CMSIS/RTOS/html/group__CMSIS__RTOS__Definitions.html#structosEvent

    /* Retrieve CAN message from queue */
    rcv_message = osMessageGet(canMessageQueueHandle, osWaitForever);
    if (rcv_message.status != osOK) {
	printf("yielding transmit CAN thread\n\r");
	osThreadYield();
    }

    can_message = (CAN_msg_t *) rcv_message.value.p;

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
    uint8_t id_h = 0xFFUL & (can_message->header.StdId >> 8);
    uint8_t id_l = 0xFFUL & (can_message->header.StdId);

    can_buffer[CAN_BUFFER_LEN - 13] = id_h;
    can_buffer[CAN_BUFFER_LEN - 12] = id_l;


    /* CAN DATA: 16 ASCII characters */
    for (uint8_t i=0; i<8; i++) {
      /* Copy each byte */
      can_buffer[i + CAN_BUFFER_LEN - 11]= can_message->data[i];
    }


    /* CAN DATA LENGTH: 1 ASCII character */
    uint8_t length = "0123456789ABCDEF"[ can_message->header.DLC & 0xFUL];
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
void read_IMU_task(void const * argument)
{
  /* USER CODE BEGIN read_IMU_task */

  /* Infinite loop */
  while(1)
  {
    union FloatBytes gy_x, gy_y, gy_z, ax_x, ax_y, ax_z;

    /* Read ACCEL 6 Bytes */
    printf("reading ACCEL values from IMU\n\r");

    uint8_t accel_data[6];

    HAL_I2C_Mem_Read(&hi2c2, IMU_DEVICE_ADDRESS, ACCEL_XOUT_H_REG, 1, accel_data, NUM_ACCEL_BYTES, 1000);
    uint16_t Accel_X_RAW = (uint16_t)(accel_data[0] << 8 | accel_data [1]);
    uint16_t Accel_Y_RAW = (uint16_t)(accel_data[2] << 8 | accel_data [3]);
    uint16_t Accel_Z_RAW = (uint16_t)(accel_data[4] << 8 | accel_data [5]);
    /*
     * Convert the RAW values into acceleration in 'g' we have to divide according to the Full scale value
     * set in FS_SEL. Have configured FS_SEL = 0. So I am dividing by 16384.0
     * For more details check ACCEL_CONFIG Register.
     */
    ax_x.float_value = Accel_X_RAW / 16384.0;  // get the float g
    ax_y.float_value = Accel_Y_RAW / 16384.0;
    ax_z.float_value = Accel_Z_RAW / 16384.0;

    printf("A_x: %.2f, A_y: %.2f, A_z: %.2f\n\r", ax_x.float_value, ax_y.float_value, ax_z.float_value);

    /* Read ACCEL 6 Bytes */
    printf("reading GYRO values from IMU\n\r");

    uint8_t gyro_data[6];

    HAL_I2C_Mem_Read(&hi2c2, IMU_DEVICE_ADDRESS, GYRO_XOUT_H_REG, 1, gyro_data, NUM_GYRO_BYTES, 1000);
    uint16_t Gyro_X_RAW = (uint16_t)(gyro_data[0] << 8 | gyro_data [1]);
    uint16_t Gyro_Y_RAW = (uint16_t)(gyro_data[2] << 8 | gyro_data [3]);
    uint16_t Gyro_Z_RAW = (uint16_t)(gyro_data[4] << 8 | gyro_data [5]);
    /*
     * Convert the RAW values into dps (degrees/s) we have to divide according to the
     * Full scale value set in FS_SEL. Have configured FS_SEL = 0.
     * So I am dividing by 131.0. For more details check GYRO_CONFIG Register
     */
    gy_x.float_value = Gyro_X_RAW / 131.0;  // get the float g
    gy_y.float_value = Gyro_Y_RAW / 131.0;
    gy_z.float_value = Gyro_Z_RAW / 131.0;

    printf("G_x: %.2f, G_y: %.2f, G_z: %.2f\n\r", gy_x.float_value, gy_y.float_value, gy_z.float_value);

    check_IMU_result(ax_x, ax_y, ax_z, gy_x, gy_y, gy_z);

    /* Add to IMU Queue */
    add_to_IMU_queue("G", "X", gy_x);
    add_to_IMU_queue("G", "Y", gy_y);
    add_to_IMU_queue("G", "Z", gy_z);
    add_to_IMU_queue("A", "X", ax_x);
    add_to_IMU_queue("A", "Y", ax_y);
    add_to_IMU_queue("A", "Z", ax_z);

    /* Delay */
    osDelay(READ_IMU_DELAY * 10);

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
void transmit_IMU_task(void const * argument)
{
  /* USER CODE BEGIN transmit_IMU_task */

  osEvent rcv_message;	/* IMU Queue Status */
  IMU_msg_t *imu_message;	/* IMU Message */

  /* Infinite loop */
  while(1)
  {
    /* Check if there are messages in the queue */
//    if (osMessageQueueGetCount(imuMessageQueueHandle) == 0) {
//	osThreadYield();
//    }
    printf("Getting message from IMU Queue and sending over radio\n\r");

    /* Get IMU Message from Queue */
    rcv_message = osMessageGet(imuMessageQueueHandle, osWaitForever);
    if (rcv_message.status != osOK) {
	printf("yielding transmit IMU thread\n\r");
	osDelay(READ_IMU_DELAY / 2);
	continue;
    }

    imu_message = (IMU_msg_t *) rcv_message.value.p;

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
    imu_buffer[IMU_MESSAGE_LEN - 8] = imu_message->imu_type;
    imu_buffer[IMU_MESSAGE_LEN - 7] = imu_message->dimension;

    /* Copy data */
    for (int i = 0; i < 4; i++) {
	imu_buffer[i + IMU_MESSAGE_LEN - 6] = imu_message->data[i];
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
void read_GPS_task(void const * argument)
{
  /* USER CODE BEGIN read_GPS_task */
  /* Infinite loop */
  while(1) {
    printf("Reading GPS for transmission\n\r");

    /* Initialize a receive buffer */
    uint8_t receive_buffer[GPS_RCV_BUFFER_SIZE];

    /* Initialize a GPS data struct */
    GPS parsed_GPS_data;

    /* Read in an NMEA message into the buffer */
    if(HAL_I2C_IsDeviceReady(&hi2c1, GPS_DEVICE_ADDRESS, 1, HAL_MAX_DELAY) == HAL_OK) {
	HAL_I2C_Master_Receive(&hi2c1, GPS_DEVICE_ADDRESS, receive_buffer, sizeof(receive_buffer), HAL_MAX_DELAY);
	printf("Got New GPS Data\n\r");
    }

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
void transmit_GPS_task(void const * argument)
{
  /* USER CODE BEGIN transmit_GPS_task */

  osEvent rcv_message;
  GPS_msg_t *gps_message;

  /* Infinite loop */
  while(1) {


    printf("Getting a GPS message from the Queue for Transmission\n\r");
    /* Get a message from the queue */
    rcv_message = osMessageGet(gpsMessageQueueHandle, osWaitForever);
    if (rcv_message.status != osOK) {
      printf("GPS queue empty, yielding thread\n\r");
      osDelay(READ_GPS_DELAY / 2);
    }

    gps_message = (GPS_msg_t *) rcv_message.value.p;

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
    strncpy(gps_buffer + 8, gps_message->data, 150); // Save space for CR+LF

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
void transmit_RTC_task(void const * argument)
{
  /* USER CODE BEGIN transmit_RTC_task */
  /* Infinite loop */
  for(;;)
  {
    printf("Sending RTC timestmap\n\r");
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

    osMailPut(imuMessageQueueHandle, &imu_message);
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
    osMailPut(gpsMessageQueueHandle, &nmea_msg);
  }
}

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

/* USER CODE END Application */

