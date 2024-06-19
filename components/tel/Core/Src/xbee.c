/**
 *  @file xbee.c
 *  @brief Defines functions related to xBee module, including setting up xBee packet overhead, and handling return frames
 *
 *  @date 2024/06/16
 *  @author Evan Owens
 */

#include "xbee.h"
#include <math.h>


/**
 * @brief Adds up all the bytes after length and before checksum in an api packet. Packet_length-1 is the final position in the api_packet,
 * and will contain checksums, so we exclude it when adding up bytes.
 *
 * @param api packet: api packet array
 * @return uint16_t total_bytes in api packet before checksum and after length fields
*/

uint16_t XBEE_sum_bytes (uint8_t api_packet[], uint16_t packet_length)
{
  uint16_t total_bytes = 0;
  for (int i = FRAME_TYPE_POSITION; i < packet_length -1; i++){
		 total_bytes += api_packet[i];
  }
  return total_bytes;
}


/**
 * @brief Sets up overhead for an API Packet
 *
 * @param api packet: api packet array
 * @return api_packet with setup api overhead
*/
void XBEE_api_overhead_setup(uint8_t api_packet[])
{

  uint8_t BIT_ADDRESS_64[] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
  api_packet[START_DELIMITER_POSITION] = START_DELIMITER;
  api_packet[FRAME_TYPE_POSITION] = FRAME_TYPE;
  api_packet[FRAME_ID_POSITION] = FRAME_ID;
  api_packet[BIT_ADDRESS_16_LOW_POSITION] = BIT_ADDRESS_16_LOW;
  api_packet[BIT_ADDRESS_16_HIGH_POSITION] = BIT_ADDRESS_16_HIGH;
  api_packet[BROADCAST_RADIUS_POSITION] = BROADCAST_RADIUS;
  api_packet[OPTIONS_POSITION] = OPTIONS;
  api_packet[TYPE_POSITION] = CAN_TYPE;
  api_packet[LENGTH_POSITION] = API_MAX_MSGS;


  for (int i = 0; i < BIT_ADDRESS_64_LENGTH; i++){
	  api_packet[i + BIT_ADDRESS_64_START_POSITION] = BIT_ADDRESS_64[i];
  }

}


/**
 * @brief Calculates and adds the MSB and LSB for an API packet. MSB and LSB meaning most significant and least significant byte based
 * off the number of bytes between the length and checksum fields. This length is found by taking the packet_length, and subtracting the constant
 * CHECKSUM_LENGTH_SIZE number of bytes that encapsulates the start delimiter, length fields, and checksum.
 *
 * @param api packet: api packet array
 * @return api_packet with setup length fields
*/
void XBEE_calculate_length(uint8_t api_packet[], uint16_t packet_length)
{
 uint16_t total_bytes = packet_length - CHECKSUM_LENGTH_SIZE;
 uint16_t lsb;
 uint16_t msb;
 lsb = total_bytes % 256;
 msb = (uint16_t)(floor(total_bytes / 256));
 api_packet[MSB_POSITION] = msb;
 api_packet[LSB_POSITION] = lsb;
}



/**
 * @brief Calculates the checksum of an api frame and adds it to the api_packet. The checksum is found by adding up all the bytes between
 * the length and checksum fields in an api packet, and then subtracting the lowest 8 bits of the number from 0xFF)
 *
 * @param api packet: api packet array
 * @return api_packet with setup checksum field
*/
void XBEE_calculate_checksum(uint8_t api_packet[], uint16_t packet_length)
{
 uint16_t total_bytes = XBEE_sum_bytes(api_packet, packet_length);
 uint16_t checksum;
 checksum = UINT8_MAX_SIZE - (total_bytes & MASK_8_BITS);
 api_packet[packet_length-1] = checksum; //packet_length -1 is final position in api_packet array

}

