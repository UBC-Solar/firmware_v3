/*
 * canbusload.h
 *
 *  Created on: Oct 17, 2024
 *      Author: makac
 */

#ifndef INC_CANBUSLOAD_H_
#define INC_CANBUSLOAD_H_

#include "can.h"

#define CAN_BIT_RATE 500000
#define CAN_WINDOW_SIZE 5

#define CAN_SOF_BITS 1
#define CAN_STANDARD_ID_BITS 11
#define CAN_EXTENDED_ID_BITS 29
#define CAN_SRR_BITS 1
#define CAN_RESERVED_BIT_R1 1
#define CAN_RTR_BITS 1
#define CAN_IDE_BITS 1
#define CAN_RESERVED_BIT_R0 1
#define CAN_DLC_BITS 4
#define CAN_CRC_BITS 15
#define CAN_CRC_DELIMITER_BITS 1
#define CAN_ACK_SLOT_BITS 1
#define CAN_ACK_DELIMITER_BITS 1
#define CAN_EOF_BITS 7
#define CAN_IFS_BITS 3

extern uint32_t can_total_bits;
extern uint32_t can_count;
extern uint32_t average_window_bits;
extern float bus_load;

uint32_t calculate_CAN_message_bits(CAN_msg_t* msg);
void slidingWindowAverage(uint32_t bits);
float getSlidingWindowAverage();
float getCANBusLoad();


#endif /* INC_CANBUSLOAD_H_ */
