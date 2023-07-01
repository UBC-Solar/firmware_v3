/**
 * This is the header file for the CAN driver.
 *
 */

#ifndef CAN_H
#define CAN_H 
#endif

#include "stm32f1xx_hal.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#define BUFFER_SIZE 2

#define REVERSE_TRUE 1 
#define REVERSE_FALSE 0 

#define REGEN_TRUE 1 
#define REGEN_FALSE 0 

//Eco mode would be standard operation and maximum efficiency
//Power mode would be for passing or when we need a boost up a hill
#define ECO_ON 1
#define POWER_ON 0

#define UINT10_MAX 0x3FF


//Limit voltage out into the motor 
#define MAX_VOLTAGE_OUT 0x3C1 //Coresponds to 4.7V (0.938*10'b1_111_111_111)


typedef struct
{
	uint16_t id;
	int32_t  velocity;
	uint32_t acceleration; 
	bool direction; 
	bool power_or_eco; 
	bool regen; 
} CAN_message_t;


void Send_Voltage(uint16_t parsed_voltage, uint8_t DAC_ADDR, I2C_HandleTypeDef *hi2c1);
void CAN_Decode_Velocity_Message(uint8_t RxData[], CAN_message_t* CAN_msg); 


