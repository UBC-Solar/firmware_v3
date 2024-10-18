/*
 * canbusload.c
 *
 *  Created on: Oct 17, 2024
 *      Author: makac
 */

#include "canbusload.h"

uint32_t can_count = 0;
uint32_t can_total_bits = 0;
uint32_t average_window_bits = 0;
float bus_load = 0;
uint32_t can_window_average = 0;
uint32_t can_bus_load = 0;

uint32_t currentIdx = 0;
uint32_t sum = 0;
uint32_t last = 0;
uint32_t circularBuffer[CAN_WINDOW_SIZE] = {0};
uint8_t isFirstWindow = 1;

uint32_t calculate_CAN_message_bits(CAN_msg_t *msg)
{

    uint32_t bits = 0;

    bits += CAN_SOF_BITS; // SOF (Start of Frame)

    if (msg->header.DLC == 0)
    {
        bits += CAN_STANDARD_ID_BITS; // Standard ID
    }
    else
    {
        bits += CAN_EXTENDED_ID_BITS; // Extended ID
        bits += CAN_SRR_BITS;         // SRR (Substitute Remote Request)
        bits += CAN_RESERVED_BIT_R1;  // Reserved bit (r1)
    }

    bits += CAN_RTR_BITS;           // RTR (Remote Transmission Request)
    bits += CAN_IDE_BITS;           // IDE (Identifier Extension)
    bits += CAN_RESERVED_BIT_R0;    // Reserved bit (r0)
    bits += CAN_DLC_BITS;           // DLC (Data Length Code)
    bits += msg->header.DLC * 8;    // Data Length Code field (msg->DLC * 8 bits)
    bits += CAN_CRC_BITS;           // CRC field
    bits += CAN_CRC_DELIMITER_BITS; // CRC delimiter
    bits += CAN_ACK_SLOT_BITS;      // ACK slot
    bits += CAN_ACK_DELIMITER_BITS; // ACK delimiter
    bits += CAN_EOF_BITS;           // EOF (End of Frame)
    bits += CAN_IFS_BITS;           // IFS (Interframe Space)

    return bits;
}

void slidingWindowAverage(uint32_t bits)
{
    // index of 'stale' element to be removed
    uint8_t removeIdx = currentIdx % CAN_WINDOW_SIZE;
    sum -= circularBuffer[removeIdx];

    // add new element to the buffer
    circularBuffer[removeIdx] = bits;

    // add new element to the sum
    sum += bits;

    can_total_bits = 0;

    currentIdx++;
}

float getSlidingWindowAverage()
{
    uint32_t sliding_sum = 0;

    for (uint8_t i = 0; i < CAN_WINDOW_SIZE; i++)
    {
        sliding_sum += circularBuffer[i];
    }

    return (float)sliding_sum;
}

float getCANBusLoad()
{

    average_window_bits = getSlidingWindowAverage();
    bus_load = ((float) average_window_bits / ((float)CAN_WINDOW_SIZE * (float)CAN_BIT_RATE)) * 100.0;

    return bus_load;
}
