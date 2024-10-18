/*
 * canbusload.h
 *
 *  Created on: Oct 17, 2024
 *      Author: makac
 */

#ifndef INC_CANBUSLOAD_H_
#define INC_CANBUSLOAD_H_

#define CAN_BIT_RATE 500000
#define CAN_WINDOW_SIZE 5

extern uint32_t can_total_bits;
extern uint32_t average_window_bits;
extern float bus_load;

uint32_t calculate_CAN_message_bits(CAN_msg_t* msg);
void slidingWindowAverage(uint32_t bits);
float getSlidingWindowAverage();
float getCANBusLoad();


#endif /* INC_CANBUSLOAD_H_ */
