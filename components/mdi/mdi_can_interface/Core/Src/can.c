/**
 * CAN functions used in the main file of the MDI firmware
 */
#include "main.h"
#include "can.h"
#include "stm32f1xx_hal_can.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx.h"
#include <stdio.h>
#include <stdlib.h>



/**
 * Takes the CAN message and decodes the data to know in which mode of operation the motor controller should be in and separates the message data into
 * the acceleration and velocity components
 *  
 * @param RxData array of 8 bit wide elements that contains the CAN message sent by the SCN.
 * @param CAN_msg struct that holds the velocity, acceleration and current state of operation of the motor controller 
 * @retval Modifies CAN_msg struct with the appropriate modes of operation and the values for velocity and acceleration
 */
 void CAN_Decode_Velocity_Message(uint8_t RxData[], CAN_message_t* CAN_msg){

	union { //union struct to convert the input stream of 32-bits into IEEE float
		uint32_t concatenated_bits;
		float float_velocity;
	}u;

	CAN_msg->velocity =  	
		( ((int32_t) RxData[3]) << 3*8) | 
		( ((int32_t) RxData[2]) << 2*8) | 
		( ((int32_t) RxData[1]) << 1*8) | 
		( ((int32_t) RxData[0]) << 0); 

	u.concatenated_bits =
		( ((uint32_t) RxData[7]) << 3*8) |
		( ((uint32_t) RxData[6]) << 2*8) |
		( ((uint32_t) RxData[5]) << 1*8) |
		( ((uint32_t) RxData[4]) << 0);
	
	CAN_msg -> acceleration = u.float_velocity;

	if (CAN_msg->velocity == 0){ //enter regen operation mode 
		CAN_msg->regen = REGEN_TRUE; 
		CAN_msg->direction = REVERSE_FALSE; //is this forward or reverse?
	} 
	else if(CAN_msg->velocity < 0){ //enter reverse operation mode
		CAN_msg->direction = REVERSE_TRUE;
		CAN_msg->regen = REGEN_FALSE; 
	}  
	else { //Forward operation mode
		CAN_msg->direction = REVERSE_FALSE; 
		CAN_msg->regen = REGEN_FALSE; 
	}
	return; 


 } //end of decode_CAN_velocity_msg 

 void Decode_Frame0 (uint8_t RxData[], CAN_message_t* CAN_msg){

	 //10 bit number
	 uint16_t BatteryVoltage = ((uint32_t)(RxData[1] & 0x03) << 1*8) |
			 	 	 	 	 	  ((uint32_t) RxData[0] << 0) ;
	 //9 bit number
	 int16_t BatteryCurrent = ((uint32_t)(RxData[2] & 0x07) << 2*8)|
			 	 	 	 	 	 ((uint32_t)(RxData[1] & 0xFC) << 1*8) ;
	 //Align Bits
	 BatteryCurrent = BatteryCurrent >> 2;

	 //Sign the Current
	 if(RxData[2] & 0x08)
		 BatteryCurrent = ((uint16_t) BatteryCurrent) * -1;
	 else
		 BatteryCurrent = ((uint16_t) BatteryCurrent) * 1;

	 //5 bit number
	 uint16_t FET_Temperature = ((uint32_t)(RxData[4] & 0x07) << 1*8)|
	 			 	            ((uint32_t)(RxData[3] & 0xC0) << 0*8) ;
	 //Align Bits
	 FET_Temperature = FET_Temperature >> 6;

	 //Fill Struct
	 CAN_msg -> busVoltage = BatteryVoltage;
	 CAN_msg -> busCurrent = BatteryCurrent;
	 CAN_msg -> controllerHeatsinkTemp = FET_Temperature;


 }

 void Decode_Frame2 (uint8_t RxData[], CAN_message_t* CAN_msg){

	 bool CurrentLimit = RxData[2] & 0x02;
	 bool OverCurrent =  RxData[2] & 0x10;

	 CAN_msg->motorCurrentFlag = CurrentLimit;
	 CAN_msg->softwareOverCurrent = OverCurrent;

 }

 uint8_t getBit(uint8_t msb, uint8_t two, uint8_t three, uint8_t four, uint8_t five, uint8_t six, uint8_t seven, uint8_t lsb){
  	uint8_t byte = (msb * 128 + two * 64 + three * 32 + four * 16 + five * 8 + six * 4 + seven * 2 + lsb * 1);
  	if(byte == 255 && msb == 0){
  		byte = 0;
  	}
  	 return byte;
  }

  void split_32_bit_number(uint32_t number, uint8_t *bytes) {
      bytes[0] = (number >> 24) & 0xFF;
      bytes[1] = (number >> 16) & 0xFF;
      bytes[2] = (number >> 8) & 0xFF;
      bytes[3] = number & 0xFF;
  }

 void get501(uint8_t* message501, CAN_message_t CanMessage){

 	  uint8_t message64[64];

 	  message64[0] = 0;
 	  message64[1] = CanMessage.motorCurrentFlag;
 	  message64[2] = CanMessage.velocityFlag;
 	  message64[3] = CanMessage.busCurrentFlag;
 	  message64[4] = CanMessage.busVoltageUpperLimitFlag;
 	  message64[5] = CanMessage.busVoltageLowerLimitFlag;
 	  message64[6] = CanMessage.heatsinkTemp;
 	  for (int i = 7; i <= 15; i++){
 		  message64[i] = 0;
 	  }
 	  message64[16] = CanMessage.hardwareOverCurrent;
 	  message64[17] = CanMessage.softwareOverCurrent;
 	  message64[18] = CanMessage.DCBusOverVoltage;
 	  for (int i = 19; i <= 63; i++){
 				  message64[i] = 0;
 	  }

 	  message501[0] = getBit(message64[0], message64[1], message64[2], message64[3],
 					  message64[4], message64[5], message64[6], message64[7]);

 	  message501[1] = getBit(message64[8], message64[9], message64[10], message64[11],
 					  message64[12], message64[13], message64[14], message64[15]);

 	  message501[2] = getBit(message64[16], message64[17], message64[18], message64[19],
 					  message64[20], message64[21], message64[22], message64[23]);

 	  message501[3] = getBit(message64[24], message64[25], message64[26], message64[27],
 					  message64[28], message64[29], message64[30], message64[31]);

 	  message501[4] = getBit(message64[32], message64[33], message64[34], message64[35],
 					  message64[36], message64[37], message64[38], message64[39]);

 	  message501[5] = getBit(message64[40], message64[41], message64[42], message64[43],
 					  message64[44], message64[45], message64[46], message64[47]);

 	  message501[6] = getBit(message64[48], message64[49], message64[50], message64[51],
 					  message64[52], message64[53], message64[54], message64[55]);

 	  message501[7] = getBit(message64[56], message64[57], message64[58], message64[59],
 					  message64[60], message64[61], message64[62], message64[63]);

 }

 void get502(uint8_t* message502, CAN_message_t CanMessage){

 	uint8_t busVoltageSplit[4];
 	split_32_bit_number(CanMessage.busVoltage, busVoltageSplit);
 	uint8_t busCurrentSplit[4];
 	split_32_bit_number(CanMessage.busCurrent, busCurrentSplit);

 	message502[0] = busVoltageSplit[0];
 	message502[1] = busVoltageSplit[1];
 	message502[2] = busVoltageSplit[2];
 	message502[3] = busVoltageSplit[3];
 	message502[4] = busCurrentSplit[0];
 	message502[5] = busCurrentSplit[1];
 	message502[6] = busCurrentSplit[2];
 	message502[7] = busCurrentSplit[3];

 }

 void get503(uint8_t* message503, CAN_message_t CanMessage){

 	uint8_t motorVelocitySplit[4];
 	split_32_bit_number(CanMessage.motorVelocity, motorVelocitySplit);
 	uint8_t vehicleVelocitytSplit[4];
 	split_32_bit_number(CanMessage.vehicleVelocity, vehicleVelocitytSplit);

 	message503[0] = motorVelocitySplit[0];
 	message503[1] = motorVelocitySplit[1];
 	message503[2] = motorVelocitySplit[2];
 	message503[3] = motorVelocitySplit[3];
 	message503[4] = vehicleVelocitytSplit[0];
 	message503[5] = vehicleVelocitytSplit[1];
 	message503[6] = vehicleVelocitytSplit[2];
 	message503[7] = vehicleVelocitytSplit[3];

 }

 void get50B(uint8_t* message50B, CAN_message_t CanMessage){

 	uint8_t motorTempSplit[4];
 	split_32_bit_number(CanMessage.motorTemp, motorTempSplit);
 	uint8_t controllerHeatsinkTempSplit[4];
 	split_32_bit_number(CanMessage.controllerHeatsinkTemp,  controllerHeatsinkTempSplit);

 	message50B[0] = motorTempSplit[0];
 	message50B[1] = motorTempSplit[1];
 	message50B[2] = motorTempSplit[2];
 	message50B[3] = motorTempSplit[3];
 	message50B[4] = controllerHeatsinkTempSplit[0];
 	message50B[5] = controllerHeatsinkTempSplit[1];
 	message50B[6] = controllerHeatsinkTempSplit[2];
 	message50B[7] = controllerHeatsinkTempSplit[3];

 }
