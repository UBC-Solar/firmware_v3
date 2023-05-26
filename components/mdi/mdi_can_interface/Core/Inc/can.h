/**
 * This is the header file for the CAN driver.
 *
 */

#ifndef CAN_H
#define CAN_H //put include guard above #includes based on Andrew input

#include "stm32f1xx_hal.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#define MAX_MESSAGE_LENGTH 31
#define BUFFER_SIZE 2

#define REVERSE_TRUE 1 
#define REVERSE_FALSE 0 

#define REGEN_TRUE 1 
#define REGEN_FALSE 0 

#define ECO_ON 1
#define POWER_ON 0

#define UINT10_MAX 0x3FF


//Limit voltage out into the motor 
#define MAX_VOLTAGE_OUT 0x3C1 //Add comment indicating what physical value this hex value corresponds to


typedef struct
{
	uint16_t id;
	int32_t  velocity;
	uint32_t acceleration; 
	bool direction; 
	bool power_or_eco; 
	bool regen; 
} CAN_msg_t;

void CANInit(void);
void CAN_Set_Filters(CAN_FilterTypeDef* fltr);

uint16_t Parse_Acc(uint32_t pedal_data);
void Send_Voltage(uint16_t parsed_voltage, uint8_t DAC_ADDR, I2C_HandleTypeDef *hi2c1);
void CAN_process(CAN_msg_t *msg1);  
void decode_CAN_velocity_message(uint8_t RxData[], CAN_msg_t* CAN_msg); 
void Send_Regen(float regen, uint8_t DAC_REGEN_ADDR, I2C_HandleTypeDef *hi2c1);
void send_test_message(uint8_t* TxData, int32_t velocity, uint32_t acceleration); 

#endif
