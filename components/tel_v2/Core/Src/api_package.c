#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define START_DELIMITER 0X7E
#define FRAME_TYPE 0X10
#define FRAME_ID 0x01;
#define BIT_ADDRESS_16_LOW 0xFE;
#define BIT_ADDRESS_16_HIGH 0xFE;

#define BROADCAST_RADIUS 0x00;

#define OPTIONS 0x00;

uint8_t bit_address[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

//SETUP FOR API 0x10 FRAME TYPES (instead of LEGACY 0x00)
/*Take unpackaged CAN, IMU, or GPS arrays and package them into API Frame*/

uint16_t apiPackage(uint8_t raw_outbox[], uint8_t end_position, uint8_t unsized_packet[], uint8_t MessageType){
  uint16_t outbox_position = 19; //standard position after static API frame data

  //setup API frame
  unsized_packet[0] = START_DELIMITER;
  unsized_packet[1] = 0x00; //length first byte.
  unsized_packet[3] = FRAME_TYPE;
  unsized_packet[4] = FRAME_ID;

  for(int i = 0; i < 8; i++){
      unsized_packet[i+5] = bit_address[i];
  }

  unsized_packet[13] = BIT_ADDRESS_16_LOW;
  unsized_packet[14] = BIT_ADDRESS_16_HIGH;
  unsized_packet[15] = BROADCAST_RADIUS;
  unsized_packet[16] = OPTIONS;
  unsized_packet[17] = MessageType; //Type of messages in API frame (CAN, IMU, or GPS)


  uint8_t lengthValue = 0; //Keep track of the length of each sub message. This value will then be entered before each message in API Frame
  uint8_t lengthPosition = 18;


  //get individual messages, and package them in length value format
  for (int i = 0; i <= end_position; i++){

      unsized_packet[outbox_position] = raw_outbox[i];
      lengthValue++;

      if(raw_outbox[i] != 'n')
	{
	  outbox_position++;
      }
      //end of message is reached: Length value is filled before message.
      else
	{
	  unsized_packet[lengthPosition] = lengthValue;
	  lengthPosition += (1 + lengthValue);
	  lengthValue = 0;
	  outbox_position += 2;
	}
  }

  unsized_packet[2] = outbox_position - 5; //Length second byte (ignore start delimiter, length fields, checksum, and one empty byte (4 bytes)
  uint16_t totalBytes = 0; //used to calculate checksum

  //add up all bytes to calculate checksum
  for(int i =3; i< outbox_position-2; i++){
      totalBytes += unsized_packet[i];

  }

  uint8_t checksum = 0xFF -(totalBytes & 0xFF);

  unsized_packet[outbox_position-2] = checksum; //set checksum

  return outbox_position-1; //used to scale sized array.



}





