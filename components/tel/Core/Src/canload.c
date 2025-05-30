#include "canload.h"

// Private defines
#ifndef WINDOW_SIZE
#define WINDOW_SIZE 5
#endif

/*
Monday update explaining constants: 
https://ubcsolar26.monday.com/boards/7524367653/pulses/7524368220/posts/3493917025
*/

#define BAUD_RATE 500000
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
#define CANLOAD_MSG_ID                      0x763
#define CANLOAD_DATA_LENGTH                 1

static uint32_t can_total_bits = 0;
uint64_t currentIdx = 0; // Always increments and needs large enough data type to avoid overflow
uint32_t circularBuffer[WINDOW_SIZE] = {0};


/** CAN HEADER DEFINITION */
CAN_TxHeaderTypeDef CANLOAD_busload = {
    .StdId = CANLOAD_MSG_ID,
    .ExtId = 0x0000,
    .IDE = CAN_ID_STD,
    .RTR = CAN_RTR_DATA,
    .DLC = CANLOAD_DATA_LENGTH};


/**
 * @brief Calculates the number of bits in a CAN message.
 *
 * This function calculates the number of bits in a CAN message based on the
 * data length code (DLC) of the message. The result is stored in the global
 * variable `can_total_bits`.
 *
 * This function modifies the global variable `can_total_bits`. It does not
 * return any value.
 *
 * @param DLC The data length code of the CAN message.
 * @param IDE The identifier extension bit of the CAN message.
 */
void CANLOAD_calculate_message_bits(uint32_t DLC, uint32_t IDE)
{
    uint32_t bits = 0;
    bits += SOF_BITS;
    if (IDE == CAN_ID_STD)
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
 * @brief Updates the sliding window with the total number of CAN bus bits.
 *
 * This function updates the sliding window with the total number of CAN bus
 * bits. The sliding window is implemented as a circular buffer with a fixed
 * size. The oldest element in the buffer is removed and replaced with the
 * new value.
 *
 * This function modifies the global variables `currentIdx`, `circularBuffer`,
 * and `can_total_bits`. It does not return any value.
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
    return (float) sliding_sum;
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
	  float total_bits = (float) calculate_total_bits();
	  float window_duration_seconds = (float) WINDOW_SIZE * (float) (CANLOAD_MSG_RATE / 1000.0f);
	  float max_bits_in_window = window_duration_seconds * (float) BAUD_RATE;
	  float load_percentage = (total_bits / max_bits_in_window) * 100.0f;

	  return load_percentage;
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
    return calculate_bus_load();
}

void CAN_tx_canload_msg() {
    CAN_comms_Tx_msg_t CAN_comms_Tx_msg = {
        .data[0] = (uint8_t) CANLOAD_get_bus_load(),
        .header = CANLOAD_busload
    };  

  CANLOAD_calculate_message_bits(CAN_comms_Tx_msg.header.DLC, CAN_comms_Tx_msg.header.IDE);
  CAN_comms_Add_Tx_message(&CAN_comms_Tx_msg);
}
