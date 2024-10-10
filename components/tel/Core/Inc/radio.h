
/**
 *  @file radio.h
 *  @brief header file for radio.c. Define buffer lengths
 *
 *  @date 2024/10/10
 *  @author Aarjav Jain
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __RADIO_TRANSNIT_H__
#define __RADIO_TRANSNIT_H__

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stdint.h"
#include "can.h"
#include <stdbool.h>

/* RADIO MESSAGE TYPEDEF */

/* PROTOTYPES */
bool RADIO_is_msg_sent(CAN_QueueMsg_TypeDef* queue_msg);

#endif /* __RADIO_H__ */