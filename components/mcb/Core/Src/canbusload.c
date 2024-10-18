/*
 * canbusload.c
 *
 *  Created on: Oct 17, 2024
 *      Author: makac
 */

#include "can.h"
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

    bits += 1; // SOF (Start of Frame)

    if (msg->header.DLC == 0)
    {

        bits += 11; // Standard ID
    }
    else
    {

        bits += 29; // Extended ID

        bits += 1; // SRR (Substitute Remote Request)

        bits += 1; // Reserved bit (r1)
    }

    bits += 1; // RTR (Remote Transmission Request)

    bits += 1; // IDE (Identifier Extension)

    bits += 1; // Reserved bit (r0)

    bits += 4; // DLC (Data Length Code)

    bits += msg->header.DLC * 8; // Data Length Code field (msg->DLC * 8 bits)

    bits += 15; // CRC field

    bits += 1; // CRC delimiter

    bits += 1; // ACK slot

    bits += 1; // ACK delimiter

    bits += 7; // EOF (End of Frame)

    bits += 3; // IFS (Interframe Space)

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
    bus_load = ((float)average_window_bits / ((float)CAN_WINDOW_SIZE * (float)CAN_BIT_RATE)) * 100.0;

    return bus_load;
}
