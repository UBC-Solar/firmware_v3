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


/*	Symbolic Constants	*/


/*	Local Function Declarations	*/


/*	Global Variables	*/
input_flags_t input_flags;
drive_state_t state = PARK;


void Drive_State_Machine_handler()
{
	motor_command_t motor_command;
	update_input_flags();

	switch (state)
	{
		case DRIVE:
			//motor_command =
	}

	handle_state_transition();
}


void Drive_State_handle_can_rx(CAN_comms_Rx_msg_t* can_msg)
{

	//Todo: write helper functions for parsing these messages
	// Velocity from MDU 0x08850225
	//BMS Regen Disabled (BMS Status 0x622)




}


void update_input_flags()
{
	//read ADC's

}









