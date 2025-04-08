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
#include "diagnostic.h"

/* SEMAPHORES */
extern osSemaphoreId_t usart1_tx_semaphore;

/* QUEUES */
extern osMessageQueueId_t radio_tx_queue;

/* PRIVATE DEFINES */
#define DIAGNOSTIC_TASK_DELAY   1000    // 1 second


#endif /* __RADIO_H__ */
