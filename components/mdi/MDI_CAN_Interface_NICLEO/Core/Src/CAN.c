/**
 * Function implementations for enabling and using CAN messaging.
 */
#include "CAN.h"
#include "stm32f1xx_hal_can.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx.h"
#include <stdio.h>
#include <stdlib.h>


/**
 * @brief Takes data from pedal (last 32 bits of message) and converts to 10 bit value
 * @param pedal_data: bits 32-63 of 0x401 message data element 
 * @retval parsed_voltage is the 10 bits to be sent to the DAC
 * 
*/
uint16_t parsed_voltage Parse_Acc(uint32_t pedal_data){
    const uint32_t MAX_32BIT_NUM = UINT32_MAX;
    const uint16_t MAX_10BIT_NUM = 0x3FF;
    return (uint16_t)((double)pedal_data / MAX_32BIT_NUM * MAX_10BIT_NUM); //cast to double to do floating point math instead of integer math
}


uint32_t parse(uint8_t one, uint8_t two ,uint8_t three ,uint8_t four){
    uint32_t result;
    result = (one << 24) | (two << 16)  | (three << 8) | four;
    //puts the 4 8 bit arrays correlated with the current part of the message in to one 32 bit integer
    return result;
}

void CAN_process(CAN_msg_t *msg1){
    
    
    //variable declarations
    uint32_t result;
    double current;
    
    //the parsing of the 4 arrays
    //arrows are for the pointer input instead of the dot operator
    result = parse(msg1->data[4], msg1->data[5], msg1->data[6], msg1->data[7]);
    
    //giving a percentage of the recieved signal out of the maximum possible current
    current = (float) result / MAX;

    //mostly just for testing, but can export to wherever or do whatever we need to 
    //printf("%f", current);
}

/**
 * @brief Takes the CAN message and decodes the data to know in which mode of operation the motor controller should be in and seperates it into the acceleration 
 * and velocity components 
 * @param 64 Bit CAN message coming from the Speeed Control Node (ID 0x401), CAN_velocity_msg_struct 
 * @retval Modifies CAN_msg struct with the approaties modes of operation abd the values for ac
 */
 void decode_CAN_velocity_message(uint8_t RxData[], CAN_msg_t CAN_msg){

	CAN_msg.velocity = (RxData[3] << 3) | (RxData[2] << 2) | (RxData[1] << 1) | (RxData[0] << 0); 

	CAN_msg.accelaration = (RxData[7] << 3) | (RxData[6] << 2) | (RxData[5] << 1) | (RxData[4] << 0);
	
	if (CAN_msg.velocity == 0) //enter regen operation mode 
		CAN_msg.regen = REGEN_TRUE; 
		CAN_msg.direction = REVERSE_TRUE;
	else if(CAN_msg.velocity < 0) //enter reverse opereation mode 
		CAN_msg.direction = REVERSE_TRUE;
		CAN_msg.regen = REGEN_FALSE;  
	else //Forward operation mode
		CAN_msg.direction = REVERSE_FALSE; 
		CAN_msg.regen = REGEN_FALSE; 
	
	return; 


 } //end of decode_CAN_velocity_msg 
 


 
