
/**
 *  @file radio.c
 *  @brief Defines functions packaging data into a CAN buffer and sending it over UART
 *
 *  @date 2024/06/08
 *  @author Aarjav Jain
 */

#include "radio.h"

// 0-7: Timestamp
// 8: '#'
// 9-12: CAN ID
// 13-20: CAN Data
// 21: CAN Data Length
// 22: '\r'             // TODO: Do we need this? maybe just use \0 instead?
// 23: '\n'

//api_buffer global variables
uint8_t api_buffer[API_BUFFER_SIZE];
uint16_t api_buffer_position = 0;
uint8_t api_message_count = 0;





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

    for (uint8_t byte_idx = 0; byte_idx < num_bytes_id; byte_idx++) {                   // Fill in reverse order
        source[CAN_ID_INDEX_END - byte_idx] = MASK_8_BITS & (tx_CAN_msg->header.StdId >> (byte_idx * BITS_IN_BYTE));
    }
}


/**
 * @brief Takes api_buffer and packages it into an api frame. transmits this api frame over uart.
 * @param radio_buffer[]: pointer to an array in which the messages are being added, api_buffer_postion: position in api_buffer where messages end.
 * @return void
*/
void RADIO_tx_API_Packager(uint8_t api_buffer[], uint16_t api_buffer_position)
{
 uint16_t api_packet_size = API_OVERHEAD_SIZE + api_buffer_position;
 uint8_t api_packet[api_packet_size];
 XBEE_api_overhead_setup(api_packet);

 for (int i = 0; i<api_buffer_position; i++){
	 api_packet[MESSAGE_DATA_START_POSITION + i] = api_buffer[i];
 }

 XBEE_calculate_length(api_packet, api_packet_size);
 XBEE_calculate_checksum(api_packet, api_packet_size);

 HAL_UART_Transmit(&huart1, api_packet, sizeof(api_packet), CAN_TRANSMIT_TIMEOUT);                     /* Transmit over UART */
}



/**
 * @brief Takes CAN radio buffer and adds it to an API buffer Array. Once enough messages are in the array, array is sent for packaging.
 * @param radio_buffer[]: pointer to an array in which the messages are being added.
 * @return void
*/
void RADIO_tx_API_Accumulator(uint8_t radio_message[])
{

  for (int i = 0; i<CAN_BUFFER_LEN; i++){
	  api_buffer[i+api_buffer_position] = radio_message[i];
  }

  api_message_count++;
  api_buffer_position = api_buffer_position + CAN_BUFFER_LEN;

  if (api_message_count == API_MAX_MSGS){
	  RADIO_tx_API_Packager(api_buffer, api_buffer_position);
	  api_message_count = 0;
	  api_buffer_position = 0;
  }
}

/**
 * @brief Packages a CAN message into a CAN_BUFFER_LEN buffer
 *        and stores it in an array for api packing and transmission
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

  RADIO_tx_API_Accumulator(radio_buffer);

  // HAL_UART_Transmit(&huart1, radio_buffer, sizeof(radio_buffer), CAN_TRANSMIT_TIMEOUT);                     /* Transmit over UART */
}







