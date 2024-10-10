
/**
 *  @file radio.c
 *  @brief Defines functions packaging data into a CAN buffer and sending it over UART
 *
 *  @date 2024/10/10
 *  @author Aarjav Jain
 */

#include "radio.h"

bool RADIO_is_msg_sent(CAN_QueueMsg_TypeDef* queue_msg)
{
	return queue_msg->is_sent;
}
