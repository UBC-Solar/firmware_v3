/*
 * mcb.c
 *
 *  Created on: Jun. 1, 2023
 *      Author: Kyle Groulx
 */
#include <string.h>

#include "mcb.h"
#include "can.h"
#include "cyclic_data.h"

// githash.h is autogenerated as a prebuild step.
// If you are getting build errors here, manually create a githash.h file in the same directory and add #define GITHASH "********"
#include "../../../../libraries/githash/githash.h" // TODO: Add this library to the project instead of using the relative path


#define MAX_CYCLE_TIME_VEHICLE_VELOCITY 1000 // Max time(ms) of not receive vehicle velocity from MDI before setting a fault in the mcb diagnosics MCB message

float cruiseVelocity = 0.0;		// Velocity for cruise control

CREATE_CYCLIC_DATA_FLOAT(vehicle_velocity, MAX_CYCLE_TIME_VEHICLE_VELOCITY);

InputFlags input_flags;
DriveState state = PARK;
MotorCommand motorCommand;
float g_throttle = 0.0;
uint16_t g_throttle_ADC = 0;
/*
 *	Main state machine task
 */
void drive_state_machine_handler()
{
	UpdateInputFlags(&input_flags);

	g_throttle_ADC = ReadADC(&hadc1);
	g_throttle = normalize_adc_value(g_throttle_ADC);

	switch(state)
	{
		case DRIVE:
			motorCommand = DoStateDRIVE(input_flags);
			TransitionDRIVEstate(input_flags, &state);
		break;

		case REVERSE:
			motorCommand = DoStateREVERSE(input_flags);
			TransitionREVERSEstate(input_flags, &state);
		break;

		case PARK:
			motorCommand = DoStatePARK(input_flags);
			TransitionPARKstate(input_flags, &state);
		break;

		// TODO: Cruise disabled
		// case CRUISE:
		// 	motorCommand = DoStateCRUISE(input_flags);
		// 	TransitionCRUISEstate(input_flags, &state);
		// break;

		// Invalid state, send 0.0 throttle and 0.0
		default:
			motorCommand = GetMotorCommand(0.0, 0.0);
	}

	SendCANMotorCommand(motorCommand);
}



/*
 * Sends MCB diagnostics over CAN
 * Bits 0-15: pedal ADC reading
 * Bit 16: pedal ADC out of range
 * Bit 17: regen on
 * Bit 18: cruise on
 */
void send_mcb_diagnostics()
{
	uint8_t data_send[CAN_DATA_LENGTH] = {0};

	data_send[0] = g_throttle_ADC & 0xFF;
	data_send[1] = (g_throttle_ADC >> 8) & 0xFF;


	if (input_flags.throttle_ADC_out_of_range == true)	
	{
		SETBIT(data_send[2], 0);
	}

	if (input_flags.regen_enabled == true)
	{
		SETBIT(data_send[2], 1);
	}

	if (input_flags.cruise_enabled == true)
	{
		SETBIT(data_send[2], 2);
	}

	if (input_flags.mech_brake_pressed == true)
	{
		SETBIT(data_send[2], 3);
	}

	if (input_flags.MDI_communication_fault == true)
	{
		SETBIT(data_send[2], 4);
	}

	HAL_CAN_AddTxMessage(&hcan, &mcb_diagnostics, data_send, &can_mailbox);

	return;
}

/* 
 * Sends the MCB githash over CAN
 */
void send_mcb_githash()
{
	uint8_t data_send[CAN_DATA_LENGTH];
	strncpy((char*)data_send, GITHASH, CAN_DATA_LENGTH);
	HAL_CAN_AddTxMessage(&hcan, &mcb_githash, data_send, &can_mailbox);
	return;
}

MotorCommand DoStateDRIVE( InputFlags input_flags )
{
	// Check if mech brake is pressed
	if ( input_flags.mech_brake_pressed )
		return GetMotorCommand(0.0, VELOCITY_FORWARD);

	// Check if regen is enabled
	if (input_flags.regen_enabled) // regen switch on and battery isn't requesting regen to be turned off
	{
		return GetMotorCommand(g_throttle, VELOCITY_FORWARD);
	}
	else // Regen disable
	{
		return GetMotorCommand(g_throttle, VELOCITY_REGEN_DISABLED);
	}
}

void TransitionDRIVEstate( InputFlags input_flags, DriveState * state)
{
	if( input_flags.switch_pos_reverse && input_flags.velocity_under_threshold && input_flags.mech_brake_pressed)
		*state = REVERSE;
	else if( input_flags.switch_pos_park && input_flags.velocity_under_threshold && input_flags.mech_brake_pressed)
		*state = PARK;
}


MotorCommand DoStateREVERSE(InputFlags input_flags)
{
	// Check if mech brake is pressed
	if ( input_flags.mech_brake_pressed )
		return GetMotorCommand(0.0, VELOCITY_REGEN_DISABLED);

	// Regen disabled in reverse on MDI, dont need to check input_flags.regen_enabled
	return GetMotorCommand(g_throttle, VELOCITY_REVERSE);
}

void TransitionREVERSEstate(InputFlags input_flags, DriveState * state)
{
	if ( input_flags.switch_pos_drive && input_flags.velocity_under_threshold && input_flags.mech_brake_pressed)
		*state = DRIVE;
	else if ( input_flags.switch_pos_park && input_flags.velocity_under_threshold && input_flags.mech_brake_pressed)
		*state = PARK;
}

MotorCommand DoStatePARK()
{
	return GetMotorCommand(0.0, VELOCITY_REGEN_DISABLED);		// COASTING
}

void TransitionPARKstate(InputFlags input_flags, DriveState * state)
{
	if ( input_flags.switch_pos_drive && input_flags.velocity_under_threshold && input_flags.mech_brake_pressed )
		*state = DRIVE;
	else if ( input_flags.switch_pos_reverse && input_flags.velocity_under_threshold && input_flags.mech_brake_pressed)
		*state = REVERSE;
}

// TODO: Cruise not implimented
MotorCommand DoStateCRUISE()
{
	return GetMotorCommand(CRUISE_THROTTLE, cruiseVelocity);
}
void TransitionCRUISEstate(InputFlags input_flags, DriveState * state)
{
	if ( input_flags.mech_brake_pressed || !input_flags.cruise_enabled )
		*state = DRIVE;
}

/*
 *  Updates input flags
 */
void UpdateInputFlags(InputFlags * input_flags)
{
	input_flags->mech_brake_pressed = HAL_GPIO_ReadPin(BRK_IN_GPIO_Port, BRK_IN_Pin);
	input_flags->regen_enabled = HAL_GPIO_ReadPin(REGEN_EN_GPIO_Port, REGEN_EN_Pin) && !input_flags->battery_regen_disabled;

	float* velocity = GET_CYCLIC_DATA(vehicle_velocity);
	if (velocity != NULL)
	{
		input_flags->velocity_under_threshold = *velocity < VELOCITY_THRESHOLD;
		input_flags->MDI_communication_fault = false;
	}
	else
	{
		// If NULL, then we haven't received a vehicle velocity msg from the MDI for MAX_CYCLE_TIME_VEHICLE_VELOCITY.
		// Set 
		input_flags->velocity_under_threshold = false;
		input_flags->MDI_communication_fault = true;
	}
	GetSwitchState(input_flags);
}

/*
 *  Parses CAN message for velocity
 */
void ParseCANVelocity(uint8_t * CANMessageData)
{
	FloatBytes velocity;
	for(int i = 0; i < (sizeof(float)/sizeof(uint8_t)); i++)
	{
		velocity.bytes[i] = CANMessageData[i + 4]; // Vehicle Velocity is stored in bits 32-63.
	}
	SET_CYCLIC_DATA(vehicle_velocity, velocity.float_value);
}

/*
 *  Parse battery status CAN message
 */
void ParseCANBatteryStatus(uint8_t * CANMessageData)
{
	input_flags.battery_regen_disabled = isBitSetFromArray(CANMessageData, 17); // regen_disable bit is stored in bit 17
}

/*
 *  Returns a MotorCommand struct based on the input throttle and velocity arguments
 */
MotorCommand GetMotorCommand(float throttle, float velocity)
{
	MotorCommand motorCommand;
	motorCommand.throttle = throttle;
	motorCommand.velocity = velocity;
	return motorCommand;
}

/*
 *  RTOS task for receiving CAN messages
 */
void TaskGetCANMessage()
{
	/* Infinite loop */
	CAN_msg_t CAN_msg;
	for(;;)
	{
		
		if (xQueueReceive(CAN_rx_queue, &CAN_msg, portMAX_DELAY) == pdTRUE)
		{
			if (CAN_msg.header.StdId == CAN_ID_VELOCITY)
			{
				ParseCANVelocity(CAN_msg.data);
			}
			else if (CAN_msg.header.StdId == CAN_ID_BATTERY_TEMP)
			{
				ParseCANBatteryStatus(CAN_msg.data);
			}
		}
	}
	/* USER CODE END getBatterySOC */
}




/*
 *   Takes current value and velocity float value and sends in via CAN as an array of bytes.
 */
void SendCANMotorCommand(MotorCommand motorCommand)
{
	union FloatBytes throttle;
	union FloatBytes velocity;

	throttle.float_value = motorCommand.throttle;
	velocity.float_value = motorCommand.velocity;

	uint8_t data_send[CAN_DATA_LENGTH];
	for (int i = 0; i < (uint8_t) CAN_DATA_LENGTH / 2; i++)
	{
		data_send[i] = velocity.bytes[i];
	    data_send[4 + i] = throttle.bytes[i];
	}
	
	if (HAL_CAN_AddTxMessage(&hcan, &drive_command_header, data_send, &can_mailbox) == HAL_OK) {
		HAL_GPIO_TogglePin(LED_OUT1_GPIO_Port, LED_OUT1_Pin);
		HAL_GPIO_TogglePin(LED_OUT2_GPIO_Port, LED_OUT2_Pin);
	}
	
}

/*
 *  Function used for normalizing(0-1) and accounting for deadzone of ADC inputs.
 */
float normalize_adc_value(uint16_t value)
{
	// TODO: Use a case statement + clean up
	if (value <= ADC_LOWER_DEADZONE || value > ADC_MAX_FOR_FULL_THROTTLE) {					// Shorted to ground or beyond pedal compression
		input_flags.throttle_ADC_out_of_range = true;
		return 0.0;
	} else if (value > ADC_FOR_NO_SPIN && value < ADC_MIN_FOR_FULL_THROTTLE) {				// Pedal compressed between initial and at brake cable
		input_flags.throttle_ADC_out_of_range = false;
		float normalized_value = (float)((float)(value - ADC_FOR_NO_SPIN) / (float)(ADC_MIN_FOR_FULL_THROTTLE - ADC_FOR_NO_SPIN));
		return normalized_value;
	} else if (value >= ADC_MIN_FOR_FULL_THROTTLE && value <= ADC_MAX_FOR_FULL_THROTTLE) {	// Pedal compressed between brake cable to 1 inch past cable
		input_flags.throttle_ADC_out_of_range = false;
		return 1.0;
	} else {																				// Default If pedal is not pressed
		input_flags.throttle_ADC_out_of_range = false;
		return 0.0;
	}
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
 */
void SendCANDIDDriveState()
{
	uint8_t data_send[CAN_DATA_LENGTH] = {0};
	data_send[0] = state;
	HAL_CAN_AddTxMessage(&hcan, &drive_state_header, data_send, &can_mailbox);
}

void GetSwitchState(InputFlags * input_flags)
{
	if( !HAL_GPIO_ReadPin(SWITCH_IN1_GPIO_Port, SWITCH_IN1_Pin) && !HAL_GPIO_ReadPin(SWITCH_IN2_GPIO_Port, SWITCH_IN2_Pin) )
	{
		// Todo: Change to enum instead of a bool for each one
		input_flags->switch_pos_drive = false;
		input_flags->switch_pos_reverse = true;
		input_flags->switch_pos_park = false;
	}
	else if( HAL_GPIO_ReadPin(SWITCH_IN1_GPIO_Port, SWITCH_IN1_Pin) && HAL_GPIO_ReadPin(SWITCH_IN2_GPIO_Port, SWITCH_IN2_Pin) )
	{
		input_flags->switch_pos_drive = true;
		input_flags->switch_pos_reverse = false;
		input_flags->switch_pos_park = false;
	}
	else
	{
		input_flags->switch_pos_drive = false;
		input_flags->switch_pos_reverse = false;
		input_flags->switch_pos_park = true;
	}
}

bool isBitSet(int num, int pos)
{
	return (num & (1 << pos)) != 0;
}

bool isBitSetFromArray(uint8_t * num, int pos)
{
	int index = pos / 8;
	int remainder = pos % 8;
	return isBitSet(num[index], remainder);
}


