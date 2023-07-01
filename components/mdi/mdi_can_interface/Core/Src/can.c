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
 * Takes the CAN message and decodes the data to know in which mode of operation the motor controller should be in and seperates the message data into
 *  into the acceleration and velocity components
 *  
 * @param RxData array of 8 bit wide elements that containes the CAN message sent by the SCN. 
 * @param CAN_msg struct that holds the velocity, acceleration and current state of operation of the motor controller 
 * @retval Modifies CAN_msg struct with the appropriate modes of operation and the values for velocity and accelation
 */
 void CAN_Decode_Velocity_Message(uint8_t RxData[], CAN_msg_t* CAN_msg){

	CAN_msg->velocity =  	
		( ((int32_t) RxData[3]) << 3*8) | 
		( ((int32_t) RxData[2]) << 2*8) | 
		( ((int32_t) RxData[1]) << 1*8) | 
		( ((int32_t) RxData[0]) << 0); 

	CAN_msg->acceleration = 
		( ((uint32_t) RxData[7]) << 3*8) | 
		( ((uint32_t) RxData[6]) << 2*8) | 
		( ((uint32_t) RxData[5]) << 1*8) | 
		( ((uint32_t) RxData[4]) << 0);
	
	if (CAN_msg->velocity == 0){ //enter regen operation mode 
		CAN_msg->regen = REGEN_TRUE; 
		CAN_msg->direction = REVERSE_FALSE; //is this forward or reverse?
	} 
	else if(CAN_msg->velocity < 0){ //enter reverse opereation mode 
		CAN_msg->direction = REVERSE_TRUE;
		CAN_msg->regen = REGEN_FALSE; 
	}  
	else { //Forward operation mode
		CAN_msg->direction = REVERSE_FALSE; 
		CAN_msg->regen = REGEN_FALSE; 
	}
	return; 


 } //end of decode_CAN_velocity_msg 
 

