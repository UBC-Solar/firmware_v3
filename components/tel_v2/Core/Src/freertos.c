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

#define CAN_BUFFER_LEN  24

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
osThreadId readIMUTaskHandle;
osThreadId readGPSTaskHandle;
osThreadId transmitRTCTaskHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void check_IMU_result(union FloatBytes ax_x, union FloatBytes ax_y, union FloatBytes ax_z,
		      union FloatBytes gy_x, union FloatBytes gy_y, union FloatBytes gy_z);

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
  osThreadDef(StartDefaultTask, startDefaultTask, osPriorityNormal, 0, 128);
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
  /* Infinite loop */
  while (1) {

    /* Wait for thread flags to be set in the CAN Rx FIFO0 Interrupt Callback */
    osSignalWait(CAN_READY, osWaitForever);
    
    /* Disable interrupts */
    taskENTER_CRITICAL();

    /* Check if CAN rx FIFO0 is not empty */
    if (HAL_CAN_GetRxFifoFillLevel(&hcan, CAN_RX_FIFO0) != 0) {
  
      /* Initialize data */
      CAN_RxHeaderTypeDef can_rx_header;
      uint8_t can_data[8];
      uint8_t radio_buffer[CAN_BUFFER_LEN] = {0};

      /* Get CAN message */
      if (HAL_OK != HAL_CAN_GetRxMessage(&hcan, CAN_RX_FIFO0, &can_rx_header, can_data)) {
	      Error_Handler(); // TODO: Should we go into an error handler?
      }

      // TODO: How are we parsing this data?
      // 0-7: Timestamp
      // 8: '#'
      // 9-12: CAN ID
      // 13-20: CAN Data
      // 21: CAN Data Length
      // 22: '\r'             // TODO: Do we need this? maybe just use \0 instead?
      // 23: '\n'

      /* TIMESTAMP */
      time_t current_timestamp = get_current_timestamp();
      
      for (uint8_t i = 0; i < 8; i++) {
        radio_buffer[i] = TIMESTAMP_BYTE(i, current_timestamp);
      }

      /* CAN MESSAGE IDENTIFIER */
      radio_buffer[8] = '#';

      /* CAN ID */ // TODO: Check if this is correct. Are the 0 bytes in the STD in the correct spot?
      if (can_rx_header.IDE == CAN_ID_STD)
      {
        radio_buffer[9]  = 0xFF & (can_rx_header.StdId);
        radio_buffer[10] = 0xFF & (can_rx_header.StdId >> 8); 
      }
      else if (can_rx_header.IDE == CAN_ID_EXT)
      {
        radio_buffer[9]  = 0xFF & (can_rx_header.ExtId);
        radio_buffer[10] = 0xFF & (can_rx_header.ExtId >> 8);
        radio_buffer[11] = 0xFF & (can_rx_header.ExtId >> 16);
        radio_buffer[12] = 0xFF & (can_rx_header.ExtId >> 24);
      }

      /* CAN DATA */
      for (uint8_t i = 0; i < 8; i++) {
        radio_buffer[13 + i] = can_data[i];
      }

      /* CAN DATA LENGTH */
      radio_buffer[21] = can_rx_header.DLC & 0xF;

      /* CARRIAGE RETURN */
      radio_buffer[CAN_BUFFER_LEN - 2] = '\r';

      /* NEW LINE */
      radio_buffer[CAN_BUFFER_LEN - 1] = '\n';

      /* Transmit over Radio */
      HAL_UART_Transmit(&huart1, radio_buffer, sizeof(radio_buffer), 1000);

      /* TODO: Log to SDLogger */
      // FIL *can_file_ptr = sd_open("CAN_Messages.txt");
      // sd_append(can_file_ptr, can_buffer);    // Append can message to the SD card
      // sd_close(can_file_ptr);
    }

    /* Enable interrupts */
    taskEXIT_CRITICAL();
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
    /* Initialize a IMU buffer */
    uint8_t imu_buffer[IMU_MESSAGE_LEN] = {0};

    union FloatBytes ax_x, ax_y, ax_z, gy_x, gy_y, gy_z;

    /* Read accelerator data */
    uint8_t accel_data[NUM_ACCEL_BYTES];

    HAL_I2C_Mem_Read(&hi2c2, IMU_DEVICE_ADDRESS, ACCEL_XOUT_H_REG, 1, accel_data, NUM_ACCEL_BYTES, 1000);

    uint16_t Accel_X_RAW = (uint16_t)(accel_data[0] << 8 | accel_data[1]);
    uint16_t Accel_Y_RAW = (uint16_t)(accel_data[2] << 8 | accel_data[3]);
    uint16_t Accel_Z_RAW = (uint16_t)(accel_data[4] << 8 | accel_data[5]);
    /*
     * Convert the RAW values into acceleration in 'g' we have to divide according to the Full scale value
     * set in FS_SEL. Have configured FS_SEL = 0. So I am dividing by 16384.0
     * For more details check ACCEL_CONFIG Register.
     */
    ax_x.float_value = Accel_X_RAW / 16384.0;  // get the float g
    ax_y.float_value = Accel_Y_RAW / 16384.0;
    ax_z.float_value = Accel_Z_RAW / 16384.0;

    /* Read gyroscope data */
    uint8_t gyro_data[NUM_GYRO_BYTES];

    HAL_I2C_Mem_Read(&hi2c2, IMU_DEVICE_ADDRESS, GYRO_XOUT_H_REG, 1, gyro_data, NUM_GYRO_BYTES, 1000);
    uint16_t Gyro_X_RAW = (uint16_t)(gyro_data[0] << 8 | gyro_data[1]);
    uint16_t Gyro_Y_RAW = (uint16_t)(gyro_data[2] << 8 | gyro_data[3]);
    uint16_t Gyro_Z_RAW = (uint16_t)(gyro_data[4] << 8 | gyro_data[5]);
    /*
     * Convert the RAW values into dps (degrees/s) we have to divide according to the
     * Full scale value set in FS_SEL. Have configured FS_SEL = 0.
     * So I am dividing by 131.0. For more details check GYRO_CONFIG Register
     */
    gy_x.float_value = Gyro_X_RAW / 131.0;  // get the float g
    gy_y.float_value = Gyro_Y_RAW / 131.0;
    gy_z.float_value = Gyro_Z_RAW / 131.0;

    /* Verify IMU is connected */
    check_IMU_result(ax_x, ax_y, ax_z, gy_x, gy_y, gy_z);

    /* Get current epoch Time Stamp */
    time_t current_timestamp = get_current_timestamp();

    /* Transmit IMU data */
    transmit_imu_data(current_timestamp, ax_x.bytes, 'A', 'X');
    transmit_imu_data(current_timestamp, ax_y.bytes, 'A', 'Y');
    transmit_imu_data(current_timestamp, ax_z.bytes, 'A', 'Z');
    transmit_imu_data(current_timestamp, gy_x.bytes, 'G', 'X');
    transmit_imu_data(current_timestamp, gy_y.bytes, 'G', 'Y');
    transmit_imu_data(current_timestamp, gy_z.bytes, 'G', 'Z');

    /* TODO: Log to SDLogger */
    // FIL *imu_file_ptr = sd_open("IMU_Messages.txt");
    // sd_append(imu_file_ptr, imu_buffer);    // Append imu message to the SD card
    // sd_close(imu_file_ptr);

    /* Delay */
    osDelay(READ_IMU_DELAY * 10);
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
    /* Initialize buffers */
    uint8_t receive_buffer[GPS_RCV_BUFFER_SIZE];
    GPS gps_data;
    GPS_msg_t gps_message;
    uint8_t gps_buffer[GPS_MESSAGE_LEN] = {0};

    /* Read in an NMEA message into the buffer */
    if(HAL_I2C_IsDeviceReady(&hi2c1, GPS_DEVICE_ADDRESS, 1, HAL_MAX_DELAY) == HAL_OK) {
	    HAL_I2C_Master_Receive(&hi2c1, GPS_DEVICE_ADDRESS, receive_buffer, sizeof(receive_buffer), HAL_MAX_DELAY);
    }

    /* Parse the buffer data --> gets stored in gps_data; */
    nmea_parse(&gps_data, &receive_buffer);

    /* Create string */
    sprintf(gps_message.data,
	    "Latitude: %.6f %c, Longitude: %.6f %c, Altitude: %.2f meters, HDOP: %.2f, Satellites: %d, Fix: %d, Time: %s",
	    gps_data.latitude, gps_data.latSide,
	    gps_data.longitude, gps_data.lonSide,
	    gps_data.altitude, gps_data.hdop,
	    gps_data.satelliteCount, gps_data.fix,
	    gps_data.lastMeasure);

    /* Null Terminate */
    gps_message.data[sizeof(gps_message.data) - 1] = '\0';

    /* Get current epoch Time Stamp */
    time_t current_timestamp = get_current_timestamp();

    /* TIMESTAMP: 8 Bytes */
    for (uint8_t i = 0; i < 8; i++) {
      gps_buffer[i] = TIMESTAMP_BYTE(i, current_timestamp);
    }

    /*
     * Copy the NMEA data into the buffer, ensuring not to exceed the buffer size
     * Adds 8 to the start to skip the time stamp
     */
    strncpy(gps_buffer + 8, gps_message.data, 150); // Save space for CR+LF

    gps_buffer[GPS_MESSAGE_LEN - 2] = '\r'; // Carriage return
    gps_buffer[GPS_MESSAGE_LEN - 1] = '\n'; // Line feed

    /* Transmit the NMEA message over UART to radio */
    HAL_UART_Transmit(&huart1, gps_buffer, sizeof(gps_buffer), 1000);

    /* TODO: Log to SDLogger */
    // FIL *gps_file_ptr = sd_open("GPS_Messages.txt");
    // sd_append(gps_file_ptr, gps_buffer);    // Append gps message to the SD card
    // sd_close(gps_file_ptr);

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

