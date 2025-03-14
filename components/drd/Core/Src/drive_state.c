/*
 * drive_state.c
 *
 *	@brief	Handles the drive state for the car. Takes ADC, GPIO, and CAN Inputs, and outputs a DAC values
 *		as well as flags for the MDI.
 *
 *  Created on: March 1, 2025
 *      Author: Evan Owens and Mridul Singh
 *
 *
 */

/*	Includes	*/
#include "drive_state.h"
#include "adc.h"
#include "CAN_comms.h"
#include <string.h>
#include <stdlib.h>

/*	Symbolic Constants	*/


/*	Local Function Declarations	*/
static void motor_command_package_and_send(motor_command_t* motor_command);
motor_command_t get_motor_command(uint16_t accel_DAC, uint16_t regen_DAC);

static void normalize_adc_values(uint16_t adc1, uint16_t adc2);
static bool accel_check_validity(uint16_t adc1, uint16_t adc2);

static motor_command_t forward_state_handle();
static motor_command_t reverse_state_handle();
static motor_command_t park_state_handle();
static void handle_state_transition();

static void brake_press_handle();
static void update_input_flags();
static void get_accel_readings();
static uint8_t get_command_flags();
static void clear_request_flags();


/*	Global Variables	*/
volatile uint32_t velocity = 0;
input_flags_t input_flags;
drive_state_t state = PARK;
uint16_t throttle_DAC = 0;


/*		DRIVE STATE MACHINE HANDLING	*/

/**
 * @brief Drive State Machine. Uses drive state to determine motor command
 * 		to be sent to the MDI.
 */
void Drive_State_Machine_handler()
{
	motor_command_t motor_command;
	update_input_flags();

	switch (state)
	{
		case FORWARD:
			motor_command = forward_state_handle();
		break;

		case REVERSE:
			motor_command = reverse_state_handle();
		break;

		case PARK:
			motor_command = park_state_handle();
		break;

		default:
			motor_command = get_motor_command(ACCEL_DAC_OFF, REGEN_DAC_OFF);
	}

	motor_command_package_and_send(&motor_command);
	handle_state_transition();
}

/*
 * @brief Handles the motor command when the car is in the forward drive state
 */
motor_command_t forward_state_handle()
{
	if (input_flags.mech_brake_pressed)
	{
		return get_motor_command(ACCEL_DAC_OFF, REGEN_DAC_OFF);
	}

	if (input_flags.regen_enabled)
	{
		return get_motor_command(throttle_DAC, REGEN_DAC_ON);
	}
	else
	{
		return get_motor_command(throttle_DAC, REGEN_DAC_OFF);
	}
}

/*
 * @brief Handles the motor command when the car is in the reverse drive state
 */
motor_command_t reverse_state_handle()
{
	if (input_flags.mech_brake_pressed)
	{
		return get_motor_command(ACCEL_DAC_OFF, REGEN_DAC_OFF);
	}

	//disable regen in reverse
	return get_motor_command(throttle_DAC, REGEN_DAC_OFF);
}

/*
 * @brief Handles the motor command when the car is in the park state
 */
motor_command_t park_state_handle()
{
	return get_motor_command(ACCEL_DAC_OFF, REGEN_DAC_OFF);
}

/*
 * @brief Handles the drive state tarnsitions
 */
void handle_state_transition()
{
	bool park_request = input_flags.park_state_request;
	bool forward_request = input_flags.forward_state_request;
	bool reverse_request = input_flags.reverse_state_request;

	//too many requests have been triggered
	if ((park_request + forward_request + reverse_request) > 1)
	{
		clear_request_flags();
		return;
	}

	switch (state)
		{
			case FORWARD:
				if(reverse_request && input_flags.velocity_under_threshold && input_flags.mech_brake_pressed)
				{
					state = REVERSE;
				}
				else if(park_request && input_flags.velocity_under_threshold && input_flags.mech_brake_pressed)
				{
					state = PARK;
				}
			break;

			case REVERSE:
				if(forward_request && input_flags.velocity_under_threshold && input_flags.mech_brake_pressed)
				{
					state = FORWARD;
				}
				else if(park_request && input_flags.velocity_under_threshold && input_flags.mech_brake_pressed)
				{
					state = PARK;
				}
			break;

			case PARK:
				if (forward_request && input_flags.velocity_under_threshold && input_flags.mech_brake_pressed)
				{
					state = FORWARD;
				}
				else if(reverse_request && input_flags.velocity_under_threshold && input_flags.mech_brake_pressed)
				{
					state = REVERSE;
				}
			break;

			default:
				state = PARK;
		}
	//reset state transition requests
	input_flags.forward_state_request = 0;
	input_flags.reverse_state_request = 0;
	input_flags.park_state_request = 0;
}


/*	DATA TRANSMISSION	*/

/*
 * @brief packages motor_command_t structs into motor command messages to be sent to the MDI
 *
 * @param motor_command, struct containing all motor command information to be sent over CAN
 */
void motor_command_package_and_send(motor_command_t* motor_command)
{
	CAN_comms_Tx_msg_t msg;
	msg.header = drive_command_header;
	uint8_t data[8] = {0};

	uint8_t accel_first_byte = (uint8_t) (motor_command->accel_DAC_value & 255);
	uint8_t accel_second_byte = (uint8_t) (motor_command->accel_DAC_value >> 8);
	uint8_t regen_first_byte = (uint8_t) (motor_command->accel_DAC_value & 255);
	uint8_t regen_second_byte = (uint8_t) (motor_command->accel_DAC_value >> 8);

	data[0] = accel_first_byte;
	data[1] = accel_second_byte;
	data[2] = regen_first_byte;
	data[3] = regen_second_byte;
	data[4] = motor_command->motor_command_flags;

	memcpy(msg.data, data, CAN_DATA_SIZE);

	CAN_comms_Add_Tx_message(&msg);
}


/*
 * @brief Handles Received CAN Messages relevant
 */
void Drive_State_can_rx_handle(uint8_t* data, uint32_t msg_id)
{
	switch(msg_id)
	{
		case(MDU_REQUEST_COMMAND_ID):
			velocity = (data[4] >> 2) | (data[5] & 0x3f); //35th to 46th bit
		break;

		case(STR_CAN_MSG_ID):
			input_flags.eco_mode_on = (data[0] >> 2); //third bit of steering CAN message
		break;
	}

}

/*
* @brief function which transmits a CAN message to the Mitsuba Motor to query their CAN Data Frames
* 		Should be called from the default task at a regular interval.
 */
void Motor_Controller_query_data()
{
	CAN_comms_Tx_msg_t msg;
	msg.header = mdu_request_header;
	msg.data[0] = MDU_REQUEST_FRAME_2_0; //request frame 2 and 0
	CAN_comms_Add_Tx_message(&msg);
}


/*		DATA COLLECTION			*/


/**
 * @brief Collects mech brake, regen switch, and accelerator inputs.
 */
void update_input_flags()
{
	input_flags.mech_brake_pressed = HAL_GPIO_ReadPin(BRK_IN_GPIO_Port, BRK_IN_Pin);
	input_flags.regen_enabled = HAL_GPIO_ReadPin(REGEN_EN_GPIO_Port, REGEN_EN_Pin);

	get_accel_readings();
}


/**
 * @brief Interrupt handler for any drive state interrupts. This includes mech brake press,
 * 		and drive, reverse, and park state requests.
 */
void drive_state_interrupt_handler(uint16_t pin)
{
	if (pin == BRK_IN_Pin)
	{
		brake_press_handle();
	}
	if (pin == FORWARD_EN_Pin)
	{
		input_flags.forward_state_request = true;
		return;
	}
	if (pin == PARK_EN_Pin)
	{
		input_flags.park_state_request = true;
		return;
	}
	if (pin == REVERSE_EN_Pin)
	{
		input_flags.reverse_state_request = true;
		return;
	}

}


/*		Handle Acceleration Readings	*/


/*
 * @brief sets throttle_DAC with DAC accel value, generated from
 * 		reading adc1 and adc2, and normalizing them.
 */
void get_accel_readings()
{
	uint16_t adc1 = Read_ADC(&hadc1);
	uint16_t adc2 = Read_ADC(&hadc2);

	if (!accel_check_validity(adc1, adc2))
	{
		input_flags.throttle_ADC_out_of_range = true;
		throttle_DAC = 0;
		return;
	}

	input_flags.throttle_ADC_out_of_range = false;
	normalize_adc_values(adc1, adc2);

}


/*
 * @brief Normalizes adc1 and adc2 into a single accel DAC value.
 *
 * @param adc1 - 12 bit ADC reading of first accelerator potentiometer
 * @param adc2 - 12 bit ADC reading of second accelerator potentiometer
 */
void normalize_adc_values(uint16_t adc1, uint16_t adc2)
{
	//Todo: confirm ADC orientation
	uint16_t uniform_adc = ((adc1 + adc2)/2) >> 2; //get rid of lowest 2 bits

	if (uniform_adc < ADC_NO_THROTTLE_MAX)
	{
		throttle_DAC = 0;
		return;
	}

	if (uniform_adc > ADC_FULL_THROTTLE_MIN)
	{
		throttle_DAC = ADC_THROTTLE_MAX;
		return;
	}

	throttle_DAC = uniform_adc;

}


/**
 * @brief Helper function to ensure that the ADC readings are valid. Validity is based on if
 * 		the readings are in the given range, and the difference between them is not too large.
 *
 *
 * @returns False if any of the ADC readings are invalid.
 *
 */
bool accel_check_validity(uint16_t adc1, uint16_t adc2)
{
	if (adc1 < ADC_LOWER_DEADZONE || adc1 > ADC_UPPER_DEADZONE)
	{
		return false;
	}
	if (adc2 < ADC_LOWER_DEADZONE || adc2 > ADC_UPPER_DEADZONE)
	{
		return false;
	}
	if (abs(adc1 - adc2) > ADC_MAX_DIFFERENCE)
	{
		return false;
	}

	return true;
}



/*		HELPER FUNCTIONS 	*/

/*
 * @brief function to send a motor command on a brake press interrupt
 */
void brake_press_handle()
{
	motor_command_t motor_command = get_motor_command(ACCEL_DAC_OFF, REGEN_DAC_OFF);
	motor_command_package_and_send(&motor_command);
}

/*
 * @brief helper functions to return the flags in the motor command message.
 */
uint8_t get_command_flags()
{
	uint8_t flags = 0;
	flags |= (input_flags.eco_mode_on ? 1: 0);
	flags |= (input_flags.eco_mode_on ? 1 << 1: 0);
	return flags;
}

/*
 * @brief helper function that returns a motor command given accelerator and regen DAC values
 *
 * @param accel_DAC 10 bit accelerator DAC value
 * @param regen_DAC 10 bit regen DAC value
 *
 * @returns motor_command_t motor_command type
 *
 */
motor_command_t get_motor_command(uint16_t accel_DAC, uint16_t regen_DAC)
{
	motor_command_t motor_command;
	motor_command.accel_DAC_value = accel_DAC;
	motor_command.regen_DAC_value = regen_DAC;
	motor_command.motor_command_flags = get_command_flags();
	return motor_command;
}

void clear_request_flags()
{
	input_flags.park_state_request = false;
	input_flags.forward_state_request = false;
	input_flags.reverse_state_request = false;
}

