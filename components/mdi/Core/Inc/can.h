/**
 * This is the header file for the MDI CAN driver.
 *
 **/

#ifndef CAN_H
#define CAN_H 

#include "stm32f1xx_hal.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#define BUFFER_SIZE 2

#define TRUE 1
#define FALSE 0

#define FORWARD_TRUE 1
#define FORWARD_FALSE 0


#define REGEN_TRUE 1 
#define REGEN_FALSE 0 

//Eco mode would be standard operation and maximum efficiency
//Power mode would be for passing or when we need a boost up a hill
#define ECO_MODE_ON 0
#define POWER_MODE_ON 1


#define UINT10_MAX 0x3FF

#define WHEEL_RADIUS 0.283 //units in m

//Limit voltage out into the motor 
#define MAX_VOLTAGE_OUT 0x3C1 //Corresponds to 4.7V (0.938*10'b1_111_111_111)

typedef struct
{
	uint16_t id;
	float  velocity;
	float acceleration;
	bool FWD_direction;
	bool PWR_mode_on;
	bool regen;
	bool cruise_control_enable;

	//0x501
	uint8_t motorCurrentFlag;
	uint8_t velocityFlag;
	uint8_t busCurrentFlag;
	uint8_t busVoltageUpperLimitFlag;
	uint8_t busVoltageLowerLimitFlag;
	uint8_t heatsinkTemp;
	uint8_t hardwareOverCurrent;
	uint8_t softwareOverCurrent;
	uint8_t DCBusOverVoltage;

	//0x502
	float busVoltage;
	float busCurrent;

	//0x503
	float motorVelocity;
	float vehicleVelocity;
	uint32_t  CAN_Velocity;

	//0x50B
	float motorTemp;
	float controllerHeatsinkTemp;

} CAN_message_t;

void CAN_Decode_Velocity_Message(uint8_t localRxData[], CAN_message_t* CAN_msg);

void Send_Test_Message(uint8_t* TxData, int32_t velocity, uint32_t acceleration);

void Decode_Frame0(uint8_t localRxData[], CAN_message_t* CAN_msg);

void Decode_Frame2(uint8_t localRxData[], CAN_message_t* CAN_msg);

uint8_t getBit(uint8_t msb, uint8_t two, uint8_t three, uint8_t four, uint8_t five, uint8_t six, uint8_t seven, uint8_t lsb);

void split_32_bit_number(uint32_t number, uint8_t *bytes);

void get501(uint8_t* message501, CAN_message_t CanMessage);

void get502(uint8_t* message502, CAN_message_t CanMessage);

void get503(uint8_t* message503, CAN_message_t CanMessage);

void get50B(uint8_t* message50B, CAN_message_t CanMessage);

void Send_Test_Message(uint8_t* TxData, int32_t velocity, uint32_t acceleration);

void CopyRxData(uint8_t* globalRxData, uint8_t* localRxData);

#endif
