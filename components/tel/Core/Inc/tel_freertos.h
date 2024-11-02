/**
 *  @file tel_freertos.h
 *  @brief header file for freertos.c
 */


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TEL_FREERTOS_H__
#define __TEL_FREERTOS_H__


/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "CAN_comms.h"

/* DEFINES */
#define NUM_USART1_TX_SEMAPHORES        1
#define RADIO_TX_QUEUE_SIZE            32

/* SEMAPHORES */
extern osSemaphoreId_t usart1_tx_semaphore;

/* QUEUES */
extern osMessageQueueId_t radio_tx_queue;


#endif /* __RADIO_H__ */
