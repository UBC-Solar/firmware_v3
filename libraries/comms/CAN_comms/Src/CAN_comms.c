/**
 * @file CAN_comms.c
 * @author Kyle Groulx
 * @brief This file contains the implementation for the CAN communications layer. 
 * See README.md for how to implement this library.
 * 
 * @copyright Copyright (c) 2024
 * 
 */

/* Includes */
#include "CAN_comms.h"


/* Private defines */
#define CAN_RX_TASK_STACK_SIZE 512
#define CAN_RX_QUEUE_SIZE 16
#define CAN_RX_STRUCT_SIZE sizeof(CAN_comms_Rx_msg_t)

/* Private varibles */
CAN_comms_config_t CAN_comms_config;
osMessageQueueId_t CAN_comms_Rx_queue;
osThreadId_t task_CAN_comms_Rx_handle;
const osThreadAttr_t task_CAN_comms_Rx_attributes = {
  .name = "task_CAN_comms_Rx",
  .stack_size = CAN_RX_TASK_STACK_SIZE,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes */
void task_CAN_comms_Rx(void const* argument);


/**
 * @brief  Initializes CAN communications layer
 * 
 * This function copies the config struct to the global variable and creates the CAN_comms_Rx task.
 * 
 * @attention This function should be called AFTER osKernelInitialize() and BEFORE osKernelStart().
 * 
 * @param  config: Pointer to the config struct
 * @return None
 */
void CAN_comms_init(CAN_comms_config_t* config)
{
    /* Check config is not NULL */
    if (config == NULL)
    {
        return; // TODO: Error handle5
    }

    /* Set config */
    CAN_comms_config = *config;

    /* Create queues */
    CAN_comms_Rx_queue = osMessageQueueNew(CAN_RX_QUEUE_SIZE, CAN_RX_STRUCT_SIZE, NULL);

    /* Create tasks */
    task_CAN_comms_Rx_handle = osThreadNew(task_CAN_comms_Rx, NULL, &task_CAN_comms_Rx_attributes);
}

/**
 * @brief  Interrupt Service Routine for CAN Rx FIFO
 * This function gets the CAN message from the CAN Rx FIFO and adds it to the CAN_comms_Rx_queue.
 * 
 * @attention This function needs to be added to the CAN Rx FIFO interrupt handler.
 * 
 * @param  None
 * @return None
 */
void ISR_CAN_comms_Rx()
{
    /* Get CAN message */
    CAN_RxHeaderTypeDef CAN_Rx_header;
    uint8_t CAN_Rx_data[CAN_DATA_SIZE] = {0u};
    if(HAL_OK != HAL_CAN_GetRxMessage(CAN_comms_config.hcan, CAN_comms_config.RX_FIFO, &CAN_Rx_header, CAN_Rx_data))
    {
        return; 
    }

    /* Populate CAN_Rx_msg */
    CAN_comms_Rx_msg_t CAN_Rx_msg;
    CAN_Rx_msg.header = CAN_Rx_header;
    memcpy(CAN_Rx_msg.data, CAN_Rx_data, CAN_DATA_SIZE);


    /* Add CAN message to the queue */
    if(osOK != osMessageQueuePut(CAN_comms_Rx_queue, &CAN_Rx_msg, 0, 0))
    {
        return; // TODO: Error handle
    }
}

/**
 * @brief Task for handling CAN Rx messages
 * This task waits for a message in the CAN_comms_Rx_queue and calls the callback function from the config struct.
 * 
 * @param argument: Unused
 * @return None
 */
void task_CAN_comms_Rx(void const* argument)
{
    /* Infinite loop */
    while (true)
    {
        /* Wait until there is a message in the queue */ 
        CAN_comms_Rx_msg_t CAN_comms_Rx_msg;
        if (osOK == osMessageQueueGet(CAN_comms_Rx_queue, &CAN_comms_Rx_msg, NULL, osWaitForever))
        {
            /* Call the handle function pointer */
            CAN_comms_config.CAN_comms_Rx_callback(CAN_comms_Rx_msg);
        }
        else
        {
            // TODO: Error handle
        }
    }
}
