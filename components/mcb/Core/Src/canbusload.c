/*
 * canbusload.c
 *
 *  Created on: Oct 17, 2024
 *      Author: makac
 */

#include "canbusload.h"

uint32_t can_total_bits = 0;
static uint64_t counter_window = 0;
uint64_t current_bus_load = 0;
uint32_t currentIdx = 0;
uint32_t circularBuffer[CANLOAD_WINDOW_SIZE] = {0};

uint32_t calculate_CAN_message_bits(CAN_msg_t *msg)
{
    uint32_t bits = 0;
    bits += CANLOAD_SOF_BITS;
    if (msg->header.DLC == 0)
    {
        bits += CANLOAD_STANDARD_ID_BITS;
    }
    else
    {
        bits += CANLOAD_EXTENDED_ID_BITS;
        bits += CANLOAD_SRR_BITS;
        bits += CANLOAD_RESERVED_BIT_R1;
    }
    bits += CANLOAD_RTR_BITS;
    bits += CANLOAD_IDE_BITS;
    bits += CANLOAD_RESERVED_BIT_R0;
    bits += CANLOAD_DLC_BITS;
    bits += msg->header.DLC * 8;
    bits += CANLOAD_CRC_BITS;
    bits += CANLOAD_CRC_DELIMITER_BITS;
    bits += CANLOAD_ACK_SLOT_BITS;      
    bits += CANLOAD_ACK_DELIMITER_BITS;
    bits += CANLOAD_EOF_BITS;
    bits += CANLOAD_IFS_BITS;
    return bits;
}

void update_sliding_window(uint32_t bits)
{
    uint8_t removeIdx = currentIdx % CANLOAD_WINDOW_SIZE;
    circularBuffer[removeIdx] = bits;
    can_total_bits = 0;
    currentIdx++;
}

float calculate_total_bits()
{
    uint32_t sliding_sum = 0;
    for (uint8_t i = 0; i < CANLOAD_WINDOW_SIZE; i++)
    {
        sliding_sum += circularBuffer[i];
    }
    return (float) sliding_sum;
}

float calculate_bus_load()
{
    return ((float) calculate_total_bits() / ((float)CANLOAD_WINDOW_SIZE * (float)CANLOAD_BIT_RATE)) * 100.0;
}

float CANLOAD_get_bus_load()
{    
	if(counter_window % 10 == 0){
        // Runs every 1 second
		  update_sliding_window(can_total_bits);
	}
	if(counter_window % 50 == 0){
        // Runs every 5 seconds
		 current_bus_load = calculate_bus_load();	
	}
    counter_window++;
    return current_bus_load;
}
