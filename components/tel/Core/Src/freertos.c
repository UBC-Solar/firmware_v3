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
osThreadId transmitRTCTaskHandle;
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
  Can_Init();

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
  while (1) {
    //printf("read_CAN_task()\n\r");
    /* Wait for thread flags to be set in the CAN Rx FIFO0 Interrupt Callback */
    osSignalWait(CAN_READY, osWaitForever);
    
    #ifndef DEBUG
      HAL_IWDG_Refresh(&hiwdg);
    #endif
    /*
     * Control Flow:
     * Wait for Flag from Interrupt
     * After flag occurs, read messages from queue repeatedly until it is empty
     * Once empty, wait for flag again.
     */

    /* Get CAN Message from Queue */
    while(1) {
      uint8_t radio_buffer[CAN_BUFFER_LEN] = {0};
      evt = osMessageGet(CAN_MSG_Rx_Queue, osWaitForever);
      if (evt.status == osEventMessage) {
	  HAL_GPIO_TogglePin(USER_LED_GPIO_Port, USER_LED_Pin);
	  rx_CAN_msg = evt.value.p; // Get pointer from the queue union
    


	  /* Perform rtc syncing check if the message is 0x751 and if RTC is reset to 2000-01-01 */
      if (rx_CAN_msg->header.StdId == RTC_TIMESTAMP_MSG_ID && checkAndSetRTCReset())
      {
        sync_memorator_rtc(rx_CAN_msg);
      }


	 // 0-7: Timestamp
	 // 8: '#'
	 // 9-12: CAN ID
	 // 13-20: CAN Data
	 // 21: CAN Data Length
	 // 22: '\r'             // TODO: Do we need this? maybe just use \0 instead?
	 // 23: '\n'

	 /* TIMESTAMP */

	 for (uint8_t i = 0; i < 8; i++) {
	   radio_buffer[7 - i] = (char) GET_BYTE_FROM_WORD(i, rx_CAN_msg->timestamp.double_as_int);
	 }
//	  printf("id = %u rx_CAN_msg->timestamp.double_value): %f\r\n", rx_CAN_msg->header.StdId, rx_CAN_msg->timestamp.double_value);

	 /* CAN MESSAGE IDENTIFIER */
	 radio_buffer[8] = '#';

	 /* CAN ID */ // TODO: Check if this is correct. Are the 0 bytes in the STD in the correct spot?
	 if (rx_CAN_msg->header.IDE == CAN_ID_STD)
	 {
	   radio_buffer[12]  = 0xFF & (rx_CAN_msg->header.StdId);
	   radio_buffer[11] = 0xFF & (rx_CAN_msg->header.StdId >> 8);
	 }
	 else if (rx_CAN_msg->header.IDE == CAN_ID_EXT)
	 {
	   radio_buffer[12]  = 0xFF & (rx_CAN_msg->header.ExtId);
	   radio_buffer[11] = 0xFF & (rx_CAN_msg->header.ExtId >> 8);
	   radio_buffer[10] = 0xFF & (rx_CAN_msg->header.ExtId >> 16);
	   radio_buffer[9] = 0xFF & (rx_CAN_msg->header.ExtId >> 24);
	 }

	 /* CAN DATA */
	 for (uint8_t i = 0; i < 8; i++) {
	   radio_buffer[13 + i] = rx_CAN_msg->data[i];
	 }

	 /* CAN DATA LENGTH */
	 radio_buffer[21] = rx_CAN_msg->header.DLC & 0xF;

	 /* CARRIAGE RETURN */
	 radio_buffer[CAN_BUFFER_LEN - 2] = '\r';

	 /* NEW LINE */
	 radio_buffer[CAN_BUFFER_LEN - 1] = '\n';

	 /* Transmit over Radio */
	 HAL_UART_Transmit(&huart1, radio_buffer, sizeof(radio_buffer), 1000);

	/* Free the memory allocated for this message */
	osPoolFree(CAN_MSG_memory_pool, rx_CAN_msg);

       }
      else break;
    }
//    else osDelay(5);
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
    //printf("read_IMU_task()\n\r");
    /* Initialize a IMU buffer */
    uint8_t imu_buffer[IMU_MESSAGE_LEN] = {0};

    union FloatBytes ax_x, ax_y, ax_z, gy_x, gy_y, gy_z;

    /* Read accelerator data */
    uint8_t accel_data[NUM_ACCEL_BYTES];
    HAL_StatusTypeDef imu_status = HAL_OK;

    imu_status |= HAL_I2C_Mem_Read(&hi2c2, IMU_DEVICE_ADDRESS, ACCEL_XOUT_H_REG, 1, accel_data, NUM_ACCEL_BYTES, 1000);

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

    osDelay(250);

    /* Read gyroscope data */
    uint8_t gyro_data[NUM_GYRO_BYTES];

    imu_status |= HAL_I2C_Mem_Read(&hi2c2, IMU_DEVICE_ADDRESS, GYRO_XOUT_H_REG, 1, gyro_data, NUM_GYRO_BYTES, 1000);
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
    // check_IMU_result(ax_x, ax_y, ax_z, gy_x, gy_y, gy_z); // TODO set flag

    /* Get current epoch Time Stamp */
    union DoubleBytes current_timestamp;
    current_timestamp.double_value = get_current_timestamp();

    /* Transmit IMU data */
//    transmit_imu_data(current_timestamp.double_as_int, ax_x.bytes, 'A', 'X');
//    transmit_imu_data(current_timestamp.double_as_int, ax_y.bytes, 'A', 'Y');
//    transmit_imu_data(current_timestamp.double_as_int, ax_z.bytes, 'A', 'Z');
//    transmit_imu_data(current_timestamp.double_as_int, gy_x.bytes, 'G', 'X');
//    transmit_imu_data(current_timestamp.double_as_int, gy_y.bytes, 'G', 'Y');
//    transmit_imu_data(current_timestamp.double_as_int, gy_z.bytes, 'G', 'Z');

    CAN_Radio_msg_t x_axis_data, y_axis_data, z_axis_data;

    /* Set headers */
    x_axis_data.header = IMU_x_axis_header;
    y_axis_data.header = IMU_y_axis_header;
    z_axis_data.header = IMU_z_axis_header;

    /* Assign the timestamp */
    x_axis_data.timestamp = current_timestamp;
    y_axis_data.timestamp = current_timestamp;
    z_axis_data.timestamp = current_timestamp;

    for (int i = 0; i < 4; i++) {
	// X-axis data
	x_axis_data.data[i] = ax_x.bytes[i];
	x_axis_data.data[4+i] = gy_x.bytes[i];

	// Y-axis data
	y_axis_data.data[i] = ax_y.bytes[i];
	y_axis_data.data[4+i] = gy_y.bytes[i];

	// Z-axis data
	z_axis_data.data[i] = ax_z.bytes[i];
	z_axis_data.data[4+i] = gy_z.bytes[i];
    }

    /* Transmit the messages */
    HAL_CAN_AddTxMessage(&hcan, &x_axis_data.header, x_axis_data.data, &can_mailbox);
    send_CAN_Radio(&x_axis_data);

    HAL_CAN_AddTxMessage(&hcan, &y_axis_data.header, y_axis_data.data, &can_mailbox);
    send_CAN_Radio(&y_axis_data);

    HAL_CAN_AddTxMessage(&hcan, &z_axis_data.header, z_axis_data.data, &can_mailbox);
    send_CAN_Radio(&z_axis_data);

    // Update diagnostics
    g_tel_diagnostics.imu_fail = (imu_status != HAL_OK);

    /* Delay */
//    osDelay(READ_IMU_DELAY * 5); // 500 ms
    osDelay(250);
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
  /* USER CODE _task */
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

    /* Only want to package GPS message if there is a fix. Otherwise no need */
    if (gps_data.fix == 1)
    {
    	g_tel_diagnostics.gps_fix = true;
		/* Get current epoch Time Stamp */
		union DoubleBytes current_timestamp;
		current_timestamp.double_value = get_current_timestamp();

		CAN_Radio_msg_t latitude_msg, longitude_msg, altitude_hdop_msg, side_and_count_msg;
		union DoubleBytes latitude_bytes, longitude_bytes;
		union FloatBytes altitude_bytes, hdop_bytes;

		/* Assign headers */
		latitude_msg.header = GPS_latitude_header;
		longitude_msg.header = GPS_longitude_header;
		altitude_hdop_msg.header = GPS_altitude_hdop_header;
		side_and_count_msg.header = GPS_side_count_header;

		/* Assign timestamps */
		latitude_msg.timestamp = current_timestamp;
		longitude_msg.timestamp = current_timestamp;
		altitude_hdop_msg.timestamp = current_timestamp;
		side_and_count_msg.timestamp = current_timestamp;

		/* Assign data as double/float so it can be read as uint64/uint8x4 */
		latitude_bytes.double_value = gps_data.latitude;
		longitude_bytes.double_value = gps_data.longitude;
		altitude_bytes.float_value = gps_data.altitude;
		hdop_bytes.float_value = gps_data.hdop;

		for  (uint8_t i=0; i < 8; i++) {
		latitude_msg.data[7 - i] = GET_BYTE_FROM_WORD(i, latitude_bytes.double_as_int);
		longitude_msg.data[7 - i] = GET_BYTE_FROM_WORD(i, longitude_bytes.double_as_int);
		altitude_hdop_msg.data[3 - i] = altitude_bytes.bytes[i];
		altitude_hdop_msg.data[7 - i] = hdop_bytes.bytes[i];
		}

		/* Satellite Count Cast */
		uint32_t sat_count = (uint32_t) gps_data.satelliteCount;
		side_and_count_msg.data[0] = gps_data.latSide;
		side_and_count_msg.data[1] = gps_data.lonSide;
		for  (uint8_t i=0; i < 4; i++) {
		side_and_count_msg.data[5 - i] = ((sat_count >> (8 * i)) && 0xFF);
		}
		side_and_count_msg.data[6] = 0;
		side_and_count_msg.data[7] = 0;

		/* Transmit a message every 2 seconds */
		HAL_CAN_AddTxMessage(&hcan, &latitude_msg.header, latitude_msg.data, &can_mailbox);
		send_CAN_Radio(&latitude_msg);
		osDelay(2000);

		HAL_CAN_AddTxMessage(&hcan, &longitude_msg.header, longitude_msg.data, &can_mailbox);
		send_CAN_Radio(&longitude_msg);
		osDelay(2000);

		HAL_CAN_AddTxMessage(&hcan, &altitude_hdop_msg.header, altitude_hdop_msg.data, &can_mailbox);
		send_CAN_Radio(&altitude_hdop_msg);
		osDelay(2000);

		HAL_CAN_AddTxMessage(&hcan, &side_and_count_msg.header, side_and_count_msg.data, &can_mailbox);
		send_CAN_Radio(&side_and_count_msg);
		osDelay(4000); // 4000 so we have 2 + 2 + 2 + 4 = 10 seconds total
    }
    else
    {
    	g_tel_diagnostics.gps_fix = false;
    	osDelay(GPS_WAIT_MSG_DELAY);
    }
  }

  /* USER CODE END read_GPS_task */
}

/* USER CODE BEGIN Header_transmit_RTC_task */

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
    uint8_t data_send = 0x00;

    CAN_Radio_msg_t diagnostics_msg;
    diagnostics_msg.header = tel_diagnostics_header;
    
    union DoubleBytes current_timestamp;
    current_timestamp.double_value = get_current_timestamp();

    diagnostics_msg.timestamp = current_timestamp;

    if(g_tel_diagnostics.rtc_reset) 
      SET_BIT(data_send, 1 << 0);
    if(g_tel_diagnostics.gps_fix)
      SET_BIT(data_send, 1 << 1);
    if(g_tel_diagnostics.imu_fail)
      SET_BIT(data_send, 1 << 2);
    if(g_tel_diagnostics.watchdog_reset)
      SET_BIT(data_send, 1 << 3);
    
    diagnostics_msg.data[0] = data_send;
    
    HAL_CAN_AddTxMessage(&hcan, &diagnostics_msg.header, diagnostics_msg.data, &can_mailbox);
    send_CAN_Radio(&diagnostics_msg);

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

