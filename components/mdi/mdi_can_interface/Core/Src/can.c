/**
 * Functions used in the main file of the MDI firmware
 */
#include "main.h"
#include "can.h"
#include "stm32f1xx_hal_can.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx.h"
#include <stdio.h>
#include <stdlib.h>


/**
 * @brief Takes data from pedal (last 32 bits of message) and converts to 10 bit value. 
 * @param pedal_data: bits 32-63 of 0x401 message data element 
 * @retval parsed_voltage is the 10 bits to be sent to the DAC
 * 
*/
uint16_t Parse_ACC(uint32_t pedal_data){

    uint16_t output; 
	uint16_t MAX_OUT = 0.90 * (float) UINT10_MAX;

    output = ((float)pedal_data / (float) UINT32_MAX ) * (float)UINT10_MAX;
	
    if (output >  MAX_OUT) //cap the output voltage to our voltage limit
        output =  MAX_OUT;
    
    return output; 
}

/**
 * @brief Sends a desired voltage to the DAC, by transmitting the correct I2C message.
 * @param uint16_t parsed_voltage should be a number between 0 and 1023. This value will be scaled to a corresponding voltage
 * by the DAC, where 0 is 0V and 1023 is Vmax. This value is a relative porcentage value that comes from the parsed_voltage function. 
 * @param DAC_ADDR: this is the I2C bit address of the DAC we want to send the I2C signal to
 * @param *hi2c1 is the HAL handle for I2C
 * @retval None
 */
void Send_Voltage(uint16_t parsed_voltage, uint8_t DAC_ADDR, I2C_HandleTypeDef *hi2c1)
    {
		uint8_t DAC_msg_buffer[2];
		uint16_t dac_data;
		if(parsed_voltage > 1023) parsed_voltage = 1023;
    	dac_data = parsed_voltage << 2;
    	DAC_msg_buffer[0] = dac_data >> 8;
    	DAC_msg_buffer[1] = dac_data;
    	HAL_I2C_Master_Transmit(hi2c1, DAC_ADDR, DAC_msg_buffer, BUFFER_SIZE, HAL_MAX_DELAY);
    }

/**
 * @brief Takes the 0-1.0 number that represents the desired regen amount as a porcentage and sends it as a 10bit number to a DAC.
 * @param regen: is float between 0 and 1.0 represeting a porcentage  
 * @param DAC_REGEN_ADDR: this is the I2C bit address of the DAC we want to send the I2C signal to (in this case the DAC for the regen signal)
 * @param *hi2c1 is the handle for the I2C
 * @retval None
 */
void Send_Regen(float regen, uint8_t DAC_REGEN_ADDR, I2C_HandleTypeDef *hi2c1){

	uint8_t DAC_msg_buffer[2];
	uint16_t dac_data;
	
	if(regen > 1.0) regen = 1.0;

	dac_data = regen * (float)UINT10_MAX; //converts to 10bit 
	
    	
	dac_data =  dac_data << 2;
    DAC_msg_buffer[0] = dac_data >> 8;
    DAC_msg_buffer[1] = dac_data; 
    HAL_I2C_Master_Transmit(hi2c1, DAC_REGEN_ADDR, DAC_msg_buffer, BUFFER_SIZE, HAL_MAX_DELAY);
}
/**
 * @brief Takes the CAN message and decodes the data to know in which mode of operation the motor controller should be in and seperates the message data into
 *  into the acceleration and velocity components 
 * @param RxData: array of 8 bit wide elements that containes the CAN message sent by the SCN. 
 * @param *CAN_msg: struct that holds the velocity, acceleration and current state of operation of the motor controller 
 * @retval Modifies CAN_msg struct with the appropriate modes of operation and the values for velocity and accelation
 */
 void decode_CAN_velocity_message(uint8_t RxData[], CAN_msg_t* CAN_msg){

	CAN_msg->velocity =  ( (int32_t) RxData[3] << 3*8) | ( (int32_t) RxData[2] << 2*8) | ( (int32_t) RxData[1] << 1*8) | ( (int32_t) RxData[0] << 0); 

	CAN_msg->acceleration = (uint32_t) ( (uint32_t) RxData[7] << 3*8) | ( (uint32_t) RxData[6] << 2*8) | ( (uint32_t) RxData[5] << 1*8) | ((uint32_t) RxData[4] << 0);
	
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
 
/**
 * @brief Sends Test message for debugging. Should mimic the message that we would recieve in car
 * @param TxData: pointer to the data that we want to transmit
 * @param velocity: 32bit velocity 
 * @param acceleration:  32bit acceleration
 * @retval none
 */
void send_test_message(uint8_t* TxData, int32_t velocity, uint32_t acceleration){
    
    TxData[0] = velocity >> 0 ; 
    TxData[1] = velocity >> 8 ; 
    TxData[2] = velocity >> 16; 
    TxData[3] = velocity >> 24; 
    
    TxData[4] = acceleration >> 0 ; 
    TxData[5] = acceleration >> 8 ; 
    TxData[6] = acceleration >> 16; 
    TxData[7] = acceleration >> 24;
	
 	
	HAL_Delay(20);
	  
} //end of send_test_message 

