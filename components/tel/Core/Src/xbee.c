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
 * @brief Adds up all the bytes after length and before checksum in an api packet
 *
 * @param api packet: api packet array
 * @return uint16_t total_bytes in api packet before checksum and after length fields
*/

uint16_t XBEE_sum_bytes (uint8_t api_packet[], uint16_t packet_length)
{
  uint16_t total_bytes = 0;
  for (int i = 3; i < packet_length; i++){
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
  uint8_t bit_address_64[] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

  api_packet[START_DELIMITER_POSITION] = START_DELIMITER;
  api_packet[FRAME_TYPE_POSITION] = FRAME_TYPE;
  api_packet[FRAME_ID_POSITION] = FRAME_ID;
  api_packet[BIT_ADDRESS_16_LOW_POSITION] = BIT_ADDRESS_16_LOW;
  api_packet[BIT_ADDRESS_16_HIGH_POSITION] = BIT_ADDRESS_16_HIGH;
  api_packet[BROADCAST_RADIUS_POSITION] = BROADCAST_RADIUS;
  api_packet[OPTIONS_POSITION] = OPTIONS;

  for (int i =0; i < 8; i++){
	  api_packet[i] = bit_address_64[i];
  }

}


/**
 * @brief Calculates and adds the MSB and LSB for an API packet
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
 msb = (uint16_t)(floor(total_bytes /256));
 api_packet[MSB_POSITION] = msb;
 api_packet[LSB_POSITION] = lsb;
}



/**
 * @brief Calculates the checksum of an api frame and adds it to the api_packet
 *
 * @param api packet: api packet array
 * @return api_packet with setup checksum field
*/
void XBEE_calculate_checksum(uint8_t api_packet[], uint16_t packet_length)
{
 uint16_t total_bytes = XBEE_sum_bytes(api_packet, packet_length);
 uint16_t checksum;
 checksum = 255 - (total_bytes & 255);
 api_packet[packet_length] = checksum;

}

