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

#include "can.h"
#include "gpio.h"
#include "spi.h"
#include "usart.h"
#include "i2c.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define KERNEL_LED_DELAY 200
#define READ_IMU_DELAY 	 100

#define CAN_BUFFER_LEN  22
#define IMU_MESSAGE_LEN 17


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

void add_to_IMU_queue(char* type, char* dimension, union FloatBytes data);

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
    /* Retrieve CAN message from queue */
    queue_status = osMessageQueueGet(canMessageQueueHandle, &can_message, NULL, osWaitForever);

    /* Yield if nothing on queue */
    if (queue_status != osOK){
      osThreadYield();
    }

    /* Initialize a CAN buffer */
    uint8_t can_buffer[CAN_BUFFER_LEN] = {0};

    /* TIMESTAMP: 8 ASCII characters */
    for (uint8_t i=0; i<CAN_BUFFER_LEN - 14; i++) {
      /* send 'D' as placeholder */
      can_buffer[i] = 'D';
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

    /* To avoid warnings */
    (void) gy_x;
    (void) gy_y;
    (void) gy_z;
    (void) ax_x;
    (void) ax_y;
    (void) ax_z;
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

  osKernelState_t kernel_status; /* Kernel Status */

  /* Infinite loop */
  while (1) {

    /* Get the kernel status */
    kernel_status = osKernelGetState();

    /* Check if the kernel status is "Running" */
    if (kernel_status == osKernelRunning) {

      /* If running, toggle the LED */
      HAL_GPIO_TogglePin(USER_LED_GPIO_Port, USER_LED_Pin);
    }

    /* Delay */
    osDelay(KERNEL_LED_DELAY);
  }
  /* USER CODE END kernel_LED_task */
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


/* USER CODE END Application */

