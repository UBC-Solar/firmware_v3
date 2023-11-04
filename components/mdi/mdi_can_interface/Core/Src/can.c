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


