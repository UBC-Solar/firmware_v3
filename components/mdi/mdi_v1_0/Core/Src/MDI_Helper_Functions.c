/**
 * Functions used in the main file of the MDI firmware
*/
#include "main.h"
#include "can.h"
#include "MDI_Helper_Functions.h"
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
uint16_t Parse_Acceleration(uint32_t pedal_data)
{

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
    * by the DAC, where 0 is 0V and 1023 is Vmax. This value is a relative percentage value that comes from the another parse function.
    * @param DAC_ADDR this is the I2C bit address of the DAC we want to send the I2C signal to
    * @param hi2c1 is the HAL handle for I2C
*/
void Send_Voltage(float parsed_voltage, uint8_t DAC_ADDR, I2C_HandleTypeDef* hi2c1)
{
 	uint8_t DAC_msg_buffer[2];
 	uint16_t dac_data = 0;
 	//parsed_voltage is a percentage, multiply by 10 bit max number, and we should get expected value
 	dac_data = UINT10_MAX * parsed_voltage;

    if(dac_data > 1023) dac_data = UINT10_MAX; //shouldnt be needed if the input parsed voltage is always a float between 0-1

 	dac_data = dac_data << 2;
    DAC_msg_buffer[0] = dac_data >> 8;
    DAC_msg_buffer[1] = dac_data;
    HAL_I2C_Master_Transmit(hi2c1, DAC_ADDR, DAC_msg_buffer, BUFFER_SIZE, HAL_MAX_DELAY);
}

uint8_t getBit(uint8_t msb, uint8_t two, uint8_t three, uint8_t four, uint8_t five, uint8_t six, uint8_t seven, uint8_t lsb){
               uint8_t byte = (msb * 128 + two * 64 + three * 32 + four * 16 + five * 8 + six * 4 + seven * 2 + lsb * 1);
  	
    if(byte == 255 && msb == 0){
  		byte = 0;
  	}
  	return byte;
}

void split_32_bit_number(uint32_t number, uint8_t *bytes){
    bytes[0] = (number >> 24) & 0xFF;
    bytes[1] = (number >> 16) & 0xFF;
    bytes[2] = (number >> 8) & 0xFF;
    bytes[3] = number & 0xFF;
}