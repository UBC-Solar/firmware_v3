
/**
 *  @file radio.c
 *  @brief Defines functions packaging data into a CAN buffer and sending it over UART
 *
 *  @date 2024/06/08
 *  @author Aarjav Jain
 */

#include "radio.h"
#include "can.h"
#include "usart.h"
#include "rtc.h"]

// 0-7: Timestamp
// 8: '#'
// 9-12: CAN ID
// 13-20: CAN Data
// 21: CAN Data Length
// 22: '\r'             // TODO: Do we need this? maybe just use \0 instead?
// 23: '\n'


/**
 * @brief Copies the source array to the buffer array from start_index to end_index
 * @param buffer: uint8_t pointer to the buffer array
 * @param startIndex: uint32_t start index of the buffer array
 * @param end_index: uint32_t end index of the buffer array
 * @param source: uint8_t pointer to the source array
 * @return void
*/
static void copy_to_buffer(uint8_t* buffer, uint32_t start_index, uint32_t end_index, uint8_t* source) {
  if (end_index >= start_index) {
    for (uint32_t i = start_index; i <= end_index; i++) {
      buffer[i] = source[i - start_index];
    }
  }
}


/**
 * @brief Copies the timestamp from the can radio messages to the radio buffer
 * @param buffer: uint8_t pointer to the buffer array3f5a467443
 * @param tx_CAN_msg: CAN_Radio_msg_t pointer to the CAN message to transmit
 * @return void
*/
static void copy_timestamp(uint8_t* buffer, CAN_Radio_msg_t *tx_CAN_msg) {
  for (uint8_t i = 0; i < CAN_TIMESTAMP_LENGTH; i++) {
	   buffer[TIMESTAMP_INDEX_END - i] = (char) UTILS_GET_BYTE_AT_INDEX(i, tx_CAN_msg->timestamp.double_as_int);
  }
}


/**
 * @brief Copies the CAN ID to the source array
 * @param source: uint8_t pointer to the source array
 * @param tx_CAN_msg: CAN_Radio_msg_t pointer to the CAN message
 * @param can_id_type: uint8_t CAN ID type
 * @return void
*/
static void copy_CAN_id(uint8_t* source, CAN_Radio_msg_t *tx_CAN_msg, uint32_t can_id_type) {
    uint8_t num_bytes_id = (can_id_type == CAN_ID_STD) ? NUM_STD_ID_BYTES : NUM_EXT_ID_BYTES;
    uint32_t can_id = (can_id_type == CAN_ID_STD) ? tx_CAN_msg->header.StdId : tx_CAN_msg->header.ExtId;

    for (uint8_t byte_idx = 0; byte_idx < num_bytes_id; byte_idx++) {                   // Fill in reverse order
        source[CAN_ID_INDEX_END - byte_idx] = MASK_8_BITS & (can_id >> (byte_idx * BITS_IN_BYTE));
    }
}


/**
 * @brief Sends a CAN message over the radio by packing it into a CAN_BUFFER_LEN buffer 
 *        and transmits it over UART
 * @param tx_CAN_msg: CAN_Radio_msg_t pointer to the CAN message to be sent
 * @return void
*/
void RADIO_tx_CAN_msg(CAN_Radio_msg_t *tx_CAN_msg)
{
  uint8_t radio_buffer[CAN_BUFFER_LEN] = {0};
  copy_timestamp(radio_buffer, tx_CAN_msg);                                                  /* TIMESTAMP */
  radio_buffer[CAN_MESSAGE_IDENTIFIER_INDEX] = CAN_MESSAGE_IDENTIFIER;                       /* CAN MESSAGE IDENTIFIER */
  copy_CAN_id(radio_buffer, tx_CAN_msg, tx_CAN_msg->header.IDE);                             /* CAN ID */
  copy_to_buffer(radio_buffer, CAN_DATA_INDEX_START, CAN_DATA_INDEX_END, tx_CAN_msg->data);  /* CAN DATA */
  radio_buffer[CAN_DATA_LENGTH_INDEX] = tx_CAN_msg->header.DLC & MASK_4_BITS;                /* CAN DATA LENGTH */
  radio_buffer[CAN_CARRIAGE_RETURN_INDEX] = '\r';                                            /* CARRIAGE RETURN */
  radio_buffer[CAN_NEW_LINE_INDEX] = '\n';                                                   /* NEW LINE */

  HAL_UART_Transmit(&huart1, radio_buffer, sizeof(radio_buffer), CAN_TRANSMIT_TIMEOUT);                     /* Transmit over UART */
}   

