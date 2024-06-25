/**
* Functions used in the main file of the MDI firmware that drive the CAN instances
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
void CAN_Decode_Velocity_Message(uint8_t localRxData[], CAN_message_t* CAN_msg)
{

	union { //union struct to convert the input stream of 32-bits into IEEE float
		uint32_t concatenated_bits;
		float float_velocity;
	}u;

	u.concatenated_bits =
		( ((int32_t) localRxData[3]) << 3*8) |
		( ((int32_t) localRxData[2]) << 2*8) |
		( ((int32_t) localRxData[1]) << 1*8) |
		( ((int32_t) localRxData[0]) << 0);

	CAN_msg -> velocity = u.float_velocity;

	u.concatenated_bits =
		( ((uint32_t) localRxData[7]) << 3*8) |
		( ((uint32_t) localRxData[6]) << 2*8) |
		( ((uint32_t) localRxData[5]) << 1*8) |
		( ((uint32_t) localRxData[4]) << 0);

	CAN_msg -> acceleration = u.float_velocity;

	if (CAN_msg->velocity == 0){
		// regen switch on MCB off, still moving forward
		CAN_msg->regen = REGEN_FALSE;
		CAN_msg->FWD_direction = FORWARD_TRUE;
	}
	else if(CAN_msg->velocity == -100){
		//enter reverse operation mode
		CAN_msg->FWD_direction = FORWARD_FALSE;
		CAN_msg->regen = REGEN_FALSE;
	}
	else if((CAN_msg->velocity == 100)){
		//Forward operation mode
		CAN_msg->FWD_direction = FORWARD_TRUE;
		CAN_msg->regen = REGEN_TRUE;
	}
	else if(CAN_msg ->cruise_control_enable == TRUE){
		//enter cruise control
		//TODO
	}
	else {
		//defualt to stationary car
		CAN_msg->FWD_direction = FORWARD_TRUE;
		CAN_msg->regen = REGEN_FALSE;
		CAN_msg ->acceleration = 0;
	}
	
	//set power mode. It should always be in PWR mode
	CAN_msg->PWR_mode_on = ECO_MODE_ON;
	return;
} //end of decode_CAN_velocity_msg

/**
	* Sends Test message for debugging. Should mimic the message that we would recieve in car
	* 
	* @param TxData pointer to the data that we want to transmit
	* @param velocity 32bit velocity 
	* @param acceleration 32bit acceleration
*/
void Send_Test_Message(uint8_t* TxData, int32_t velocity, uint32_t acceleration)
{
	TxData[0] = velocity >> 0 ;
	TxData[1] = velocity >> 8 ;
	TxData[2] = velocity >> 16;
	TxData[3] = velocity >> 24;
	TxData[4] = acceleration >> 0 ;
	TxData[5] = acceleration >> 8 ;
	TxData[6] = acceleration >> 16;
	TxData[7] = acceleration >> 24;
	HAL_Delay(20);
} //end of Send_Test_Message

/**
* Receives the Frame0 CAN message form the Mitsuba Motor Controller and parses the message to properly populate the can_msg struct
*
* @param RxData array of 8 bit wide elements that contains the Frame0 CAN message sent by the controller
* @param CAN_msg struct that holds the velocity, acceleration and current state of operation of the motor controller as well as output message data.
*/
void Decode_Frame0 (uint8_t localRxData[], CAN_message_t* CAN_msg)
{

	//10 bit number
	uint32_t BatteryVoltage = ((uint32_t)(localRxData[1] & 0x03) << 1*8) |
			 	 	 	 	 	  ((uint32_t) localRxData[0] << 0) ;

	CAN_msg -> busVoltage = (float) BatteryVoltage / 2.0; //according to datasheet 0.5 V / LSB

	//9 bit number
	uint32_t BatteryCurrent = ((uint32_t)(localRxData[2] & 0x07) << 6)|
			 	 	 	 	 	 ((uint32_t)(localRxData[1] & 0xFC) >> 2) ;

	//Sign the Current
	if(localRxData[2] & 0x08)
		CAN_msg -> busCurrent = (float) BatteryCurrent * -1.0; //datasheet 1A per LSB
	else
		CAN_msg -> busCurrent = (float) BatteryCurrent * 1.0;

	//5 bit number
	uint32_t FET_Temperature = ((uint32_t)(localRxData[4] & 0x07) << 2) |
	 			 	           ((uint32_t)(localRxData[3] & 0xC0) >> 6) ;

	CAN_msg -> controllerHeatsinkTemp = (float) FET_Temperature * 5.0; //datasheet 5 Celsius per LSB

	//NOTE no direct motor temperature measurement. Set to 0 for now.
	CAN_msg -> motorTemp = 0;
}

/**
* Receives the Frame2 CAN message form the Mitsuba Motor Controller and parses the message to properly populate the can_msg struct
*
* @param RxData array of 8 bit wide elements that contains the Frame2 CAN message sent by the controller
* @param CAN_msg struct that holds the velocity, acceleration and current state of operation of the motor controller as well as output message data.
*/
void Decode_Frame2 (uint8_t localRxData[], CAN_message_t* CAN_msg)
{

	bool CurrentLimit = localRxData[2] & 0x02;
	bool OverCurrent =  localRxData[2] & 0x10;

	CAN_msg->motorCurrentFlag = CurrentLimit;
	CAN_msg->softwareOverCurrent = OverCurrent;

}

void get501(uint8_t* message501, CAN_message_t CanMessage)
{

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

 	message501[3] = getBit(message64[0], message64[1], message64[2], message64[3],
 					  message64[4], message64[5], message64[6], message64[7]);

 	message501[2] = getBit(message64[8], message64[9], message64[10], message64[11],
 					  message64[12], message64[13], message64[14], message64[15]);

 	message501[1] = getBit(message64[16], message64[17], message64[18], message64[19],
 					  message64[20], message64[21], message64[22], message64[23]);

 	message501[0] = getBit(message64[24], message64[25], message64[26], message64[27],
 					  message64[28], message64[29], message64[30], message64[31]);

 	message501[7] = getBit(message64[32], message64[33], message64[34], message64[35],
 					  message64[36], message64[37], message64[38], message64[39]);

 	message501[6] = getBit(message64[40], message64[41], message64[42], message64[43],
 					  message64[44], message64[45], message64[46], message64[47]);

 	message501[5] = getBit(message64[48], message64[49], message64[50], message64[51],
 					  message64[52], message64[53], message64[54], message64[55]);

 	message501[4] = getBit(message64[56], message64[57], message64[58], message64[59],
 					  message64[60], message64[61], message64[62], message64[63]);
}

void get502(uint8_t* message502, CAN_message_t CanMessage)
{

	uint8_t busVoltageSplit[4];
 	split_32_bit_number(CanMessage.busVoltage, busVoltageSplit);
 	uint8_t busCurrentSplit[4];
 	split_32_bit_number(CanMessage.busCurrent, busCurrentSplit);
  
 	message502[3] = busVoltageSplit[0];
 	message502[2] = busVoltageSplit[1];
 	message502[1] = busVoltageSplit[2];
 	message502[0] = busVoltageSplit[3];

 	message502[7] = busCurrentSplit[0];
 	message502[6] = busCurrentSplit[1];
 	message502[5] = busCurrentSplit[2];
	message502[4] = busCurrentSplit[3];

}

void get503(uint8_t* message503, CAN_message_t CanMessage)
{

 	uint8_t motorVelocitySplit[4];
 	split_float_number(CanMessage.motorVelocity, motorVelocitySplit);
 	uint8_t vehicleVelocitytSplit[4];
 	split_float_number(CanMessage.vehicleVelocity, vehicleVelocitytSplit);

 	message503[3] = motorVelocitySplit[0];
 	message503[2] = motorVelocitySplit[1];
 	message503[1] = motorVelocitySplit[2];
 	message503[0] = motorVelocitySplit[3];

 	message503[7] = vehicleVelocitytSplit[0];
 	message503[6] = vehicleVelocitytSplit[1];
 	message503[5] = vehicleVelocitytSplit[2];
	message503[4] = vehicleVelocitytSplit[3];
}

 void get50B(uint8_t* message50B, CAN_message_t CanMessage)
 {

 	uint8_t motorTempSplit[4];
 	split_32_bit_number(CanMessage.motorTemp, motorTempSplit);
 	uint8_t controllerHeatsinkTempSplit[4];
 	split_32_bit_number(CanMessage.controllerHeatsinkTemp,  controllerHeatsinkTempSplit);
   
 	message50B[3] = motorTempSplit[0];
 	message50B[2] = motorTempSplit[1];
 	message50B[1] = motorTempSplit[2];
 	message50B[0] = motorTempSplit[3];
 	message50B[7] = controllerHeatsinkTempSplit[0];
 	message50B[6] = controllerHeatsinkTempSplit[1];
 	message50B[5] = controllerHeatsinkTempSplit[2];
	message50B[4] = controllerHeatsinkTempSplit[3];
}

void CopyRxData(uint8_t* globalRxData, uint8_t* localRxData)
{
	uint8_t i = 0;
	for(i = 0 ; i < 8; i++){
		localRxData[i] = globalRxData[i];
	}
	return;
}

