#include "canbusload.h"

// Private defines
#ifndef WINDOW_SIZE
#define WINDOW_SIZE 5
#endif

#define SOF_BITS 1
#define STANDARD_ID_BITS 11
#define EXTENDED_ID_BITS 29
#define SRR_BITS 1
#define RESERVED_BIT_R1 1
#define RTR_BITS 1
#define IDE_BITS 1
#define RESERVED_BIT_R0 1
#define DLC_BITS 4
#define CRC_BITS 15
#define CRC_DELIMITER_BITS 1
#define ACK_SLOT_BITS 1
#define ACK_DELIMITER_BITS 1
#define EOF_BITS 7
#define IFS_BITS 3

static uint32_t can_total_bits = 0;
static uint64_t current_bus_load = 0;
uint32_t currentIdx = 0;
uint32_t circularBuffer[WINDOW_SIZE] = {0};

/**
 * @brief Calculates the number of bits in a CAN message.
 *
 * This function calculates the number of bits in a CAN message based on the
 * message's header and data length.
 *
 * This function does not modify any global variables or have any side effects.
 *
 * @param msg A pointer to a CAN message structure.
 * @return The number of bits in the CAN message as a 32-bit integer.
 */
void CANLOAD_calculate_message_bits(uint32_t DLC)
{
    uint32_t bits = 0;
    bits += SOF_BITS;
    if (DLC == 0)
    {
        bits += STANDARD_ID_BITS;
    }
    else
    {
        bits += EXTENDED_ID_BITS;
        bits += SRR_BITS;
        bits += RESERVED_BIT_R1;
    }
    bits += RTR_BITS;
    bits += IDE_BITS;
    bits += RESERVED_BIT_R0;
    bits += DLC_BITS;
    bits += DLC * 8;
    bits += CRC_BITS;
    bits += CRC_DELIMITER_BITS;
    bits += ACK_SLOT_BITS;
    bits += ACK_DELIMITER_BITS;
    bits += EOF_BITS;
    bits += IFS_BITS;

    can_total_bits += bits;
}

/**
 * @brief Updates the sliding window buffer with new CAN bus data bits.
 *
 * This function updates the circular buffer used for sliding window calculations
 * of CAN bus load. It stores the given number of bits in the circular buffer at
 * the current index, overwrites the oldest entry if necessary, and increments
 * the current index.
 *
 * This function modifies the global variables `circularBuffer` and `can_total_bits`.
 * It does not return any value.
 *
 * @param bits The number of CAN bus data bits to add to the sliding window.
 */
void CANLOAD_update_sliding_window()
{
    uint8_t removeIdx = currentIdx % WINDOW_SIZE;
    circularBuffer[removeIdx] = can_total_bits;
    can_total_bits = 0;
    currentIdx++;
}

/**
 * @brief Calculates the total number of CAN bus bits in the sliding window.
 *
 * This function iterates over the circular buffer to calculate the sum of
 * CAN bus data bits currently in the sliding window. The result is returned
 * as a floating-point value.
 *
 * This function does not modify any global variables or have any side effects.
 *
 * @return The total number of CAN bus bits in the sliding window as a float.
 */
float calculate_total_bits()
{
    uint32_t sliding_sum = 0;
    for (uint8_t i = 0; i < WINDOW_SIZE; i++)
    {
        sliding_sum += circularBuffer[i];
    }
    return (float)sliding_sum;
}

/**
 * @brief Calculates the current bus load as a percentage.
 *
 * This function calculates the current bus load as a percentage based on the
 * total number of CAN bus bits in the sliding window and the bit rate of the
 * CAN bus. The result is returned as a floating-point value.
 *
 * This function does not modify any global variables or have any side effects.
 *
 * @return The current bus load as a percentage as a float.
 */
float calculate_bus_load()
{
    return ((float)calculate_total_bits() / ((float)WINDOW_SIZE * (float)CANLOAD_BAUD_RATE)) * 100.0;
}

/**
 * @brief Returns the current bus load as a percentage.
 *
 * This function returns the current bus load as a percentage based on the
 * total number of CAN bus bits in the sliding window and the bit rate of the
 * CAN bus. The result is returned as a floating-point value.
 *
 * This function calls the `update_sliding_window` function every 1 second
 * and the `calculate_bus_load` function every 5 seconds.
 *
 * @return The current bus load as a percentage as a float.
 */
float CANLOAD_get_bus_load()
{
    current_bus_load = calculate_bus_load();
    return current_bus_load;
}
