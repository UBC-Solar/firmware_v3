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
 * Takes data from pedal (last 32 bits of message) and converts to 10 bit value.
 *  
 * @param pedal_data bits 32-63 of 0x401 message data element 
 * @retval parsed_voltage is the 10 bits to be sent to the DAC
*/
uint16_t Parse_Acceleration(uint32_t pedal_data){

    uint16_t output; 
	uint16_t MAX_OUT = 0.90 * (float) UINT10_MAX;

    output = ((float)pedal_data / (float) UINT32_MAX ) * (float)UINT10_MAX;
	
    if (output >  MAX_OUT){ //cap the output voltage to our voltage limit
        output =  MAX_OUT;
	}
    return output; 
}

/**
 * Sends a desired voltage to the DAC, by transmitting the correct I2C message.
 * 
 * @param uint16_t parsed_voltage should be a number between 0 and 1023. This value will be scaled to a corresponding voltage
 * by the DAC, where 0 is 0V and 1023 is Vmax. This value is a relative porcentage value that comes from the parsed_voltage function. 
 * @param DAC_ADDR this is the I2C bit address of the DAC we want to send the I2C signal to
 * @param hi2c1 is the HAL handle for I2C
 */
void Send_Voltage(float parsed_voltage, uint8_t DAC_ADDR, I2C_HandleTypeDef* hi2c1)
    {
		uint8_t DAC_msg_buffer[2];
		uint16_t dac_data = 0;
		//parsed_voltage is a percentage, multiply by 10 bit max number, and we should get expected value
		dac_data = UINT10_MAX * parsed_voltage;

//		if(parsed_voltage > 1023) parsed_voltage = 1023; //not required if we are already initializing the max to 1023 but just in case for now
    	//dac_data = parsed_voltage << 2;

		dac_data = dac_data << 2;
    	DAC_msg_buffer[0] = dac_data >> 8;
    	DAC_msg_buffer[1] = dac_data;
    	HAL_I2C_Master_Transmit(hi2c1, DAC_ADDR, DAC_msg_buffer, BUFFER_SIZE, HAL_MAX_DELAY);
    }

/**
 * Takes the 0-1.0 number that represents the desired regen amount as a porcentage and sends it as a 10bit number to a DAC.
 * 
 * @param regen is float between 0 and 1.0 represeting a porcentage  
 * @param DAC_REGEN_ADDR is the I2C bit address of the DAC we want to send the I2C signal to (in this case the DAC for the regen signal)
 * @param hi2c1 is the handle for the I2C
 */
void Send_Regen(float regen, uint8_t DAC_REGEN_ADDR, I2C_HandleTypeDef* hi2c1){

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
 * Sends Test message for debugging. Should mimic the message that we would recieve in car
 * 
 * @param TxData pointer to the data that we want to transmit
 * @param velocity 32bit velocity 
 * @param acceleration 32bit acceleration
 */
void Send_Test_Message(uint8_t* TxData, int32_t velocity, uint32_t acceleration){
    
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
