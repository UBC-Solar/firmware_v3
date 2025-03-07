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
#include <stdlib.h>

/*	Symbolic Constants	*/


/*	Local Function Declarations	*/
static void motor_command_package_and_send(motor_command_t motor_command);
static void brake_press_handle();
static void update_input_flags();
static void get_accel_readings();
static void normalize_adc_values(uint16_t adc1, uint16_t adc2);
static bool accel_check_validity(uint16_t adc1, uint16_t adc2);
static uint8_t get_command_flags();


/*	Global Variables	*/
input_flags_t input_flags;
drive_state_t state = PARK;
uint16_t throttle_DAC = 0;


void Drive_State_Machine_handler()
{
	motor_command_t motor_command;
	update_input_flags();

	switch (state)
	{
		case DRIVE:
			//motor_command =
	}

	//handle_state_transition();
}






/*	DATA TRANSMISSION	*/

void motor_command_package_and_send(motor_command_t motor_command)
{
//	CAN_comms_Tx_msg_t msg;
//	CAN_TxHeaderTypeDef header;
	//Todo: finsish writting function
}

void Drive_State_can_rx_handle(CAN_comms_Rx_msg_t* can_msg)
{

	//Todo: write helper functions for parsing these messages
	// Velocity from MDU 0x08850225
	//BMS Regen Disabled (BMS Status 0x622)




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


void brake_press_handle()
{
	motor_command_t motor_command;
	motor_command.accel_DAC_value = 0;
	motor_command.regen_DAC_value = REGEN_DAC_DEFAULT;
	motor_command.motor_command_flags = get_command_flags();
	motor_command_package_and_send(motor_command);

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



