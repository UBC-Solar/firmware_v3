/*
 * mcb.c
 *
 *  Created on: Jun. 1, 2023
 *      Author: Kyle Groulx
 */

#include "mcb.h"

InputFlags input_flags; 	   // Event flags for deciding what state to be in.
DriveState state;		   // Holds the current the drive state.
float cruise_velocity = 0; 	   // Velocity for cruise control
float velocity_of_car = 0; 	   // Current velocity of the car will be stored here.
uint8_t battery_soc = 0; 	   // Stores the charge of the battery, updated in a task.

/*
 *   Takes current value and velocity float value and sends in via CAN as an array of bytes.
 */
void SendCANMotorCommand(float current, float velocity)
{
	union FloatBytes c;
	union FloatBytes v;

	c.float_value = current;
	v.float_value = velocity;

	uint8_t data_send[CAN_DATA_LENGTH];
	for (int i = 0; i < (uint8_t) CAN_DATA_LENGTH / 2; i++)
	{
		data_send[i] = v.bytes[i];
	    data_send[4 + i] = c.bytes[i];
	}
	HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
	HAL_CAN_AddTxMessage(&hcan, &drive_command_header, data_send, &can_mailbox);
}

/*
 *  Function used for normalizing(0-1) and accounting for deadzone of ADC inputs.
 */
float NormalizeADCValue(uint16_t value)
{
	return (value - ADC_DEADZONE >= 0 ? ((float)(value - ADC_DEADZONE))/(ADC_MAX - ADC_DEADZONE) : 0.0);
}

/*
 *  Function used for sending the Next Page command to DID.
 */
void SendCANDIDNextPage()
{
	uint8_t data_send[CAN_DATA_LENGTH] = {0};
	SETBIT(data_send[0], 0);
	HAL_CAN_AddTxMessage(&hcan, &DID_next_page_header, data_send, &can_mailbox);
}

/*
 * 	Sends a CAN message to the DID that contains the drive state of the MCB to display to the driver
 *  NOTE: The drive state displayed on the DID is a simplified version of the internal state of the MCB,
 *  IE there are only 5 states that can be shown on the DID: Drive, Regen, Cruise, Park and Reverse.
 *  INVALID = 0x00	// Should never be in INVALID state
 *  DRIVE   = 0x02
 *	CRUISE  = 0x04
 * 	PARK    = 0x06
 *	REVERSE = 0x07
 */

void SendCANDIDDriveState(DriveState state)
{
	static DriveState lastState = PARK;
	uint8_t data_send[CAN_DATA_LENGTH] = {0};

	if( state == DRIVE || state == CRUISE || state == PARK || state == REVERSE )
	{
		data_send[1] = state;
		lastState = state;
	}
	else if( state == IDLE && (lastState == DRIVE || lastState == REGEN || lastState == REVERSE) )
	{
		data_send[1] = lastState; // If in the IDLE state, use the last used state
	}
	else if( state == REGEN && (lastState == DRIVE || lastState == REGEN || lastState == REVERSE) )
	{
		data_send[1] = lastState; // If in the IDLE state, use the last used state
	}
	else if (state == CRUISE_ACCELERATE )
	{
		data_send[1] = CRUISE; // If in the CRUISE_ACCELERATE state, send the CRUISE state
	}
	else
	{
		data_send[1] = INVALID; // Should never be in this state.
	}

	HAL_CAN_AddTxMessage(&hcan, &DID_next_page_header, data_send, &can_mailbox);
}

void UpdateInputFlags(InputFlags* flags)
{
	flags->mech_brake_pressed = HAL_GPIO_ReadPin(BRK_IN_GPIO_Port, BRK_IN_Pin);
	flags->park_enabled = HAL_GPIO_ReadPin(PARK_EN_GPIO_Port, PARK_EN_Pin);
	flags->reverse_enabled = HAL_GPIO_ReadPin(RVRS_EN_GPIO_Port, RVRS_EN_Pin);
	flags->regen_switch_enabled = HAL_GPIO_ReadPin(REGEN_EN_GPIO_Port, REGEN_EN_Pin);
	flags->velocity_under_threshold = (velocity_of_car < MIN_REVERSE_VELOCITY);
	flags->charge_under_threshold = (battery_soc < BATTERY_SOC_THRESHOLD);
}

bool isBitSet(int num, int pos)
{
	return (num & (1 << pos)) != 0;
}


