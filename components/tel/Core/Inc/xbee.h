
/**
 *  @file xbee.h
 *  @brief header file for xbee.c. Define xBee API packet positions
 *
 *  @date 2024/06/16
 *  @author Evan Owens
 *   */

#ifndef INC_XBEE_H_
#define INC_XBEE_H_

#include <stdint.h>
#include <radio.h>

/*XBee API Packet Overhead*/
#define START_DELIMITER 					0x7E
#define FRAME_TYPE 							0x10 				//0x10 is a transmit request
#define FRAME_ID 							0x01
#define BIT_ADDRESS_16_LOW 					0xFE
#define BIT_ADDRESS_16_HIGH 				0xFE
#define BROADCAST_RADIUS 					0x00
#define OPTIONS 							0x00
#define TYPE 								0x00
#define BIT_ADDRESS_16_LOW 					0xFE
#define BIT_ADDRESS_16_HIGH 				0xFE
#define CAN_TYPE 							0x00

/*XBEE API PACKET Calculation Constants*/
#define MASK_8_BITS 						0xFF
#define UINT8_MAX_SIZE 						0xFF
#define BIT_ADDRESS_64_LENGTH 				8
#define CHECKSUM_LENGTH_SIZE 				4					//size of fields in api packet that are excluded in checksum and length field calculations

/*XBee API Packet Positions*/
#define START_DELIMITER_POSITION 			0
#define MSB_POSITION 						1
#define LSB_POSITION 						2
#define FRAME_TYPE_POSITION 				3
#define FRAME_ID_POSITION 					4
#define BIT_ADDRESS_64_START_POSITION 		5
#define BIT_ADDRESS_16_LOW_POSITION 		13
#define BIT_ADDRESS_16_HIGH_POSITION 		14
#define BROADCAST_RADIUS_POSITION 			15
#define OPTIONS_POSITION 					16
#define TYPE_POSITION 						17
#define LENGTH_POSITION 					18
#define MESSAGE_DATA_START_POSITION 		19



void XBEE_api_overhead_setup(uint8_t api_packet[]);
void XBEE_calculate_checksum(uint8_t api_packet[], uint16_t packet_length);
void XBEE_calculate_length(uint8_t api_packet[], uint16_t packet_length);

#endif /* INC_XBEE_H_ */
