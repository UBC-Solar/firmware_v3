/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : freertos.c
 * Description        : Code for freertos applications
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2022 STMicroelectronics.
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
#include "usart.h"

#include "imu.h"
#include <stdio.h>
#include "i2c.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define KERNEL_LED_DELAY 200

#define CAN_MESSAGE_QUEUE_SIZE 10

#define IMU_QUEUE_SIZE 10

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
union FloatBytes {
    float float_value;
    uint8_t bytes[4];
} FloatBytes;

const osThreadAttr_t readCANTask_attributes = {
  .name = "readCANTask",
  .stack_size = 128 * 8,
  .priority = (osPriority_t) osPriorityNormal,
};

const osThreadAttr_t transmitMessageTask_attributes = {
  .name = "transmitMessageTask",
  .stack_size = 128 * 8,
  .priority = (osPriority_t) osPriorityNormal,
};

const osThreadAttr_t kernelLEDTask_attributes = {
  .name = "kernelLEDTask",
  .stack_size = 128 * 8,
  .priority = (osPriority_t) osPriorityNormal,
};

const osThreadAttr_t changeRadioSettingsTask_attributes = {
  .name = "changeRadioSettings",
  .stack_size = 128 * 8,
  .priority = (osPriority_t) osPriorityNormal,
};

const osMessageQueueAttr_t canMessageQueue_attributes = {
  .name = "canMessageQueue"
};

const osMessageQueueAttr_t imuMessageQueue_attributes = {
  .name = "imuMessageQueue"
};

/* Definitions for readimu */
osThreadId_t readimuHandle;
const osThreadAttr_t readimu_attributes = {
  .name = "readimu",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Definitions for transmitdata */
osThreadId_t transmitIMUHandle;
const osThreadAttr_t transmitIMU_attributes = {
  .name = "transmitIMU",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

osThreadId_t readCANTaskHandle;
osThreadId_t kernelLEDTaskHandle;
osThreadId_t transmitMessageTaskHandle;
osThreadId_t changeRadioSettingsTaskHandle;

osMessageQueueId_t canMessageQueueHandle;
osMessageQueueId_t imuMessageQueueHandle;


/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

void kernelLEDTask (void *argument);
void readCANTask(void *argument);
void transmitMessageTask(void *argument);
void changeRadioSettingsTask(void *argument);

void sendByte(char c);
void sendChar(char c);

void StartReadIMU(void *argument);
void StartTransmitIMU(void *argument);

void addtoIMUQueue(char* type, char* dimension, union FloatBytes data);

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  // Initialize IMU
  initIMU();

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

  canMessageQueueHandle = osMessageQueueNew(CAN_MESSAGE_QUEUE_SIZE, sizeof(CAN_msg_t), &canMessageQueue_attributes);
  imuMessageQueueHandle = osMessageQueueNew(IMU_QUEUE_SIZE, sizeof(IMU_msg_t), &imuMessageQueue_attributes);
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
//  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */

  // kernelLEDTaskHandle = osThreadNew(kernelLEDTask, NULL, &kernelLEDTask_attributes);
  
  readCANTaskHandle = osThreadNew(readCANTask, NULL, &readCANTask_attributes);
  transmitMessageTaskHandle = osThreadNew(transmitMessageTask, NULL, &transmitMessageTask_attributes);

  readimuHandle = osThreadNew(StartReadIMU, NULL, &readimu_attributes);
  transmitIMUHandle = osThreadNew(StartTransmitIMU, NULL, &transmitIMU_attributes);

  // changeRadioSettingsTaskHandle = osThreadNew(changeRadioSettingsTask, NULL, &changeRadioSettingsTask_attributes);

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
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

__NO_RETURN void kernelLEDTask (void *argument) {
  osKernelState_t kernel_status;

  while (1) {
    kernel_status = osKernelGetState();

    if (kernel_status == osKernelRunning) {
      HAL_GPIO_TogglePin(KERNEL_LED_GPIO_Port, KERNEL_LED_Pin);
    }

    osDelay(KERNEL_LED_DELAY);
  }
}

__NO_RETURN void readCANTask(void *argument) {
  static HAL_StatusTypeDef rx_status;
  static CAN_msg_t current_can_message;

  while (1) {
    // wait for thread flags to be set
    osThreadFlagsWait(CAN_READY, osFlagsWaitAll, osWaitForever);

    if (HAL_CAN_GetRxFifoFillLevel(&hcan, CAN_RX_FIFO0) != 0) {
      // there are multiple CAN IDs being passed through the filter, pull out the current message
      rx_status = HAL_CAN_GetRxMessage(&hcan, CAN_RX_FIFO0, &can_rx_header, current_can_data);
      HAL_GPIO_TogglePin(KERNEL_LED_GPIO_Port, KERNEL_LED_Pin);

      // package into CAN_msg_t
      current_can_message.header = can_rx_header;


      for (uint8_t i=0; i<8; i++) {
        current_can_message.data[i] = current_can_data[i];
      }

      // priority=0, timeout=0
      osMessageQueuePut(canMessageQueueHandle, &current_can_message, 0U, 0U);
    }

    HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING);

  }
}

__NO_RETURN void transmitMessageTask(void *argument) {
  static CAN_msg_t can_message;
  osStatus_t queue_status;

  while (1) {
    // retrieve CAN message from queue
    queue_status = osMessageQueueGet(canMessageQueueHandle, &can_message, NULL, osWaitForever);

    if (queue_status != osOK){
      osThreadYield();
    }

    uint8_t can_buffer[21] = {0};

    // TIMESTAMP: 8 ASCII characters
    for (uint8_t i=0; i<8; i++) {
      // send 'D' as placeholder
        can_buffer[i] = 'D';
    }

    // CAN ID: 4 ASCII characters
    uint8_t id_h = 0xFFUL & (can_message.header.StdId >> 8);
    uint8_t id_l = 0xFFUL & (can_message.header.StdId);

    can_buffer[8] = id_h;
    can_buffer[9] = id_l;


    // CAN DATA: 16 ASCII characters
    for (uint8_t i=0; i<8; i++) {
      // can_stream[2+i] = 0xFFUL & (can_message.data[i]);
        can_buffer[i + 10]= can_message.data[i];
    }

    // CAN DATA LENGTH: 1 ASCII character
    uint8_t length = "0123456789ABCDEF"[ can_message.header.DLC & 0xFUL];
    can_buffer[18] = length;

    // NEW LINE: 1 ASCII character
    can_buffer[19] = '\n';

    // CARRIAGE RETURN: 1 ASCII character
    can_buffer[20] = '\r';

    HAL_UART_Transmit(&huart3, can_buffer, sizeof(can_buffer), 1000);

  }
}

__NO_RETURN void changeRadioSettingsTask(void *argument) {

  while(1) {
    // enter command mode
    uint8_t tripleplus[3] = {'+', '+', '+'};

    osDelay(1000);
    HAL_UART_Transmit(&huart3, tripleplus, sizeof(tripleplus), 1000);
    osDelay(2000);
    
    uint8_t check_dl[5] = "ATDL\r";
    HAL_UART_Transmit(&huart3, check_dl, sizeof(check_dl), 1000);
    
    osDelay(1000);

    // send destination low change AT command
    uint8_t change_dl[15] = "ATDL 418C9CBD\r";
    HAL_UART_Transmit(&huart3, change_dl, sizeof(change_dl), 1000);

    osDelay(1000);

    // send ATDL
    HAL_UART_Transmit(&huart3, check_dl, sizeof(check_dl), 1000);
    
    osDelay(1000);
    
    // send "ATAC" (apply changes)
    uint8_t apply_changes[5] = "ATAC\r";
    HAL_UART_Transmit(&huart3, apply_changes, sizeof(apply_changes), 1000);

    osDelay(1000);

    // send "ATWR" (write changes to non-volatile memory)
    uint8_t write_changes[5] = "ATWR\r";
    HAL_UART_Transmit(&huart3, write_changes, sizeof(write_changes), 1000);

    osDelay(1000);

    HAL_GPIO_WritePin(KERNEL_LED_GPIO_Port, KERNEL_LED_Pin, GPIO_PIN_SET);

    osThreadExit();
  }
}

void sendChar(char c)
{
  HAL_StatusTypeDef uart_tx_status;

  //Convert to ASCII
  uint8_t c_H = "0123456789ABCDEF"[(c >> 4) & 0xFUL];
  uint8_t c_L = "0123456789ABCDEF"[ c & 0xFUL];

  uart_tx_status = HAL_UART_Transmit(&huart3, &c_H, 1, 1000);
  uart_tx_status = HAL_UART_Transmit(&huart3, &c_L, 1, 1000);
}

__NO_RETURN void StartReadIMU(void *argument){
  union FloatBytes gy_x, gy_y, gy_z, ax_x, ax_y, ax_z;
  /* Infinite loop */
  while(1)
  {
    gy_x.float_value = gyro(GYRO_X);
    gy_y.float_value = gyro(GYRO_Y);
    gy_z.float_value = gyro(GYRO_Z);
    ax_x.float_value = accel(ACCEL_X);
    ax_y.float_value = accel(ACCEL_Y);
    ax_z.float_value = accel(ACCEL_Z);

    // IMU DATA
    addtoIMUQueue("G", "X", gy_x);
    addtoIMUQueue("G", "Y", gy_y);
    addtoIMUQueue("G", "Z", gy_z);
    addtoIMUQueue("A", "X", ax_x);
    addtoIMUQueue("A", "Y", ax_y);
    addtoIMUQueue("A", "Z", ax_z);

    osDelay(1000);
  }

  // In case we accidentally exit from task loop
  osThreadTerminate(NULL);

}

void addtoIMUQueue(char* type, char* dimension, union FloatBytes data){
    IMU_msg_t imu_message;

    imu_message.imu_type = type[0];
    imu_message.dimension = dimension[0];
    for (int i = 0; i < 4; i++) {
        imu_message.data[i] = data.bytes[i];
    }

    osMessageQueuePut(imuMessageQueueHandle, &imu_message, 0U, 0U);
}

__NO_RETURN void StartTransmitIMU(void *argument){
  osStatus_t imu_queue_status;
  IMU_msg_t imu_message;

  while(1)
  {
    // Check if there are messages in the queue
    if (osMessageQueueGetCount(imuMessageQueueHandle) == 0) {
        osThreadYield();
    }

    imu_queue_status = osMessageQueueGet(imuMessageQueueHandle, &imu_message, NULL, osWaitForever);

    if (imu_queue_status != osOK){
      osThreadYield();
    }
    uint8_t imu_buffer[9] = {0};

    // IMU ID
    imu_buffer[0] = '@';

    // IMU Data from queue
    imu_buffer[1] = imu_message.imu_type;
    imu_buffer[2] = imu_message.dimension;
    for (int i = 0; i < 4; i++) {
        imu_buffer[i + 3] = imu_message.data[i];
    }

    // NEW LINE
    imu_buffer[7] = '\n';

    // CARRIAGE RETURN
    imu_buffer[8] = '\r';

    HAL_UART_Transmit(&huart3, imu_buffer, sizeof(imu_buffer), 1000);
  }

  // In case we accidentally exit from task loop
  osThreadTerminate(NULL);
}

/* USER CODE END Application */

