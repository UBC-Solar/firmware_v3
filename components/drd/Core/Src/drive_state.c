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
#include "diagnostic.h"
#include "cyclic_data.h"


/*	Local Function Declarations	*/
static void motor_command_package_and_send(motor_command_t* motor_command, bool from_ISR);
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
static void velocity_CAN_msg_handle(uint8_t* data);
static void steering_CAN_msg_handle(uint8_t* data);

#ifdef DEBUG
    void state_request_CAN_msg_handle(uint8_t* data);
#endif


/*	Global Variables	*/
volatile uint32_t g_velocity_kmh = 0;
volatile bool g_eco_mode = true;
volatile drive_state_t g_drive_state = PARK;

uint16_t g_throttle_DAC = 0;
input_flags_t g_input_flags;


/*		DRIVE STATE MACHINE HANDLING	*/

/**
 * @brief Drive State Machine. Uses drive state to determine motor command
 * 		to be sent to the MDI.
 */
void Drive_State_Machine_handler()
{
	motor_command_t motor_command;
	update_input_flags();

	switch (g_drive_state)
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

	motor_command_package_and_send(&motor_command, false);
	handle_state_transition();
}

/*
 * @brief Handles the motor command when the car is in the forward drive state
 */
motor_command_t forward_state_handle()
{
	if (g_input_flags.mech_brake_pressed)
	{
		return get_motor_command(ACCEL_DAC_OFF, REGEN_DAC_OFF);
	}

	if (g_input_flags.regen_enabled)
	{
		return get_motor_command(g_throttle_DAC, REGEN_DAC_ON);
	}
	else
	{
		return get_motor_command(g_throttle_DAC, REGEN_DAC_OFF);
	}
}

/*
 * @brief Handles the motor command when the car is in the reverse drive state
 */
motor_command_t reverse_state_handle()
{
	if (g_input_flags.mech_brake_pressed)
	{
		return get_motor_command(ACCEL_DAC_OFF, REGEN_DAC_OFF);
	}

	//disable regen in reverse
	return get_motor_command(g_throttle_DAC, REGEN_DAC_OFF);
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
	bool park_request = g_input_flags.park_state_request;
	bool forward_request = g_input_flags.forward_state_request;
	bool reverse_request = g_input_flags.reverse_state_request;

	//too many requests have been triggered
	if ((park_request + forward_request + reverse_request) > 1)
	{
		clear_request_flags();
		return;
	}

	switch (g_drive_state)
		{

			case FORWARD:
				if(reverse_request && g_input_flags.velocity_under_threshold && g_input_flags.mech_brake_pressed)
				{
					g_drive_state = REVERSE;
				}
				else if(park_request && g_input_flags.velocity_under_threshold && g_input_flags.mech_brake_pressed)
				{
					g_drive_state = PARK;
				}
			break;

			case REVERSE:
				if(forward_request && g_input_flags.velocity_under_threshold && g_input_flags.mech_brake_pressed)
				{
					g_drive_state = FORWARD;
				}
				else if(park_request && g_input_flags.velocity_under_threshold && g_input_flags.mech_brake_pressed)
				{
					g_drive_state = PARK;
				}
			break;

			case PARK:
				if (forward_request && g_input_flags.velocity_under_threshold && g_input_flags.mech_brake_pressed)
				{
					g_drive_state = FORWARD;
				}
				else if(reverse_request && g_input_flags.velocity_under_threshold && g_input_flags.mech_brake_pressed)
				{
					g_drive_state = REVERSE;
				}
			break;

			default:
				g_drive_state = PARK;
		}
	//reset state transition requests
	clear_request_flags();
}


/*	DATA TRANSMISSION	*/

/*
 * @brief packages motor_command_t structs into motor command messages to be sent to the MDI
 *
 * @param motor_command, struct containing all motor command information to be sent over CAN
 */
void motor_command_package_and_send(motor_command_t* motor_command, bool from_ISR)
{
	CAN_comms_Tx_msg_t msg;
	msg.header = drive_command_header;
	uint8_t data[8] = {0};

	uint8_t accel_first_byte = (uint8_t) (motor_command->accel_DAC_value & 255);
	uint8_t accel_second_byte = (uint8_t) (motor_command->accel_DAC_value >> 8);
	uint8_t regen_first_byte = (uint8_t) (motor_command->regen_DAC_value & 255);
	uint8_t regen_second_byte = (uint8_t) (motor_command->regen_DAC_value >> 8);

	data[0] = accel_first_byte;
	data[1] = accel_second_byte;
	data[2] = regen_first_byte;
	data[3] = regen_second_byte;
	data[4] = motor_command->motor_command_flags;

	memcpy(msg.data, data, CAN_DATA_SIZE);

	if (from_ISR)
	{
		CAN_comms_Add_Tx_messageISR(&msg);
	}
	else
	{
		CAN_comms_Add_Tx_message(&msg);
	}

}


/*
 * @brief Handles Received CAN Messages relevant to drive state
 */
void Drive_State_CAN_rx_handle(uint32_t msg_id, uint8_t* data)
{
	switch(msg_id)
	{
		case(FRAME0):
			velocity_CAN_msg_handle(data);
		break;

		case(STR_CAN_MSG_ID):
			steering_CAN_msg_handle(data);
		break;

		#ifdef DEBUG
		case(0x500):
			state_request_CAN_msg_handle(data);
		break;
		#endif
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
	msg.data[0] = MDU_REQUEST_FRAME; //request frame 0,1,2
	CAN_comms_Add_Tx_message(&msg);
}


/*		DATA COLLECTION			*/


/**
 * @brief Collects mech brake, regen switch, and accelerator inputs.
 */
void update_input_flags()
{
	bool mech_brake_pressed = HAL_GPIO_ReadPin(BRK_IN_GPIO_Port, BRK_IN_Pin);
	bool regen_enabled =  HAL_GPIO_ReadPin(REGEN_EN_GPIO_Port, REGEN_EN_Pin);

	g_input_flags.mech_brake_pressed = mech_brake_pressed;
	g_input_flags.regen_enabled = regen_enabled;

	g_diagnostics.flags.mech_brake_pressed = mech_brake_pressed;
	g_diagnostics.flags.regen_enabled  = regen_enabled;

	get_accel_readings();
}


/**
 * @brief Interrupt handler for any drive state interrupts. This includes mech brake press,
 * 		and drive, reverse, and park state requests.
 */
void Drive_State_interrupt_handler(uint16_t pin)
{
	HAL_GPIO_TogglePin(DEBUG_LED_1_GPIO_Port, DEBUG_LED_1_Pin);
	if (pin == BRK_IN_Pin)
	{
		brake_press_handle();
	}
	if (pin == FORWARD_EN_Pin)
	{
		g_input_flags.forward_state_request = true;
		return;
	}
	if (pin == PARK_EN_Pin)
	{
		g_input_flags.park_state_request = true;
		return;
	}
	if (pin == REVERSE_EN_Pin)
	{
		g_input_flags.reverse_state_request = true;
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

	g_diagnostics.raw_adc1 = adc1;
	g_diagnostics.raw_adc2 = adc2;

	if (!accel_check_validity(adc1, adc2))
	{
		g_throttle_DAC = 0;
		return;
	}

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
	uint16_t average_adc = ((adc1 + adc2)/2);

	if (average_adc < ADC_NO_THROTTLE_MAX)
	{
		g_throttle_DAC = 0;
		return;
	}

	if (average_adc > ADC_FULL_THROTTLE_MIN)
	{
		g_throttle_DAC = ADC_THROTTLE_MAX;
		return;
	}

	g_throttle_DAC = average_adc >> 2; //get rid of least 2 bits for 10 bit DAC on MC

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
		g_diagnostics.flags.throttle_ADC_out_of_range = true;
		return false;
	}
	if (adc2 < ADC_LOWER_DEADZONE || adc2 > ADC_UPPER_DEADZONE)
	{
		g_diagnostics.flags.throttle_ADC_out_of_range = true;
		return false;
	}
	if (abs(adc1 - adc2) > ADC_MAX_DIFFERENCE)
	{
		g_diagnostics.flags.throttle_ADC_mismatch = true;
		return false;
	}

	g_diagnostics.flags.throttle_ADC_mismatch = false;
	g_diagnostics.flags.throttle_ADC_out_of_range = false;

	return true;
}



/*		HELPER FUNCTIONS 	*/

/*
 * @brief function to send a motor command on a brake press interrupt
 */
void brake_press_handle()
{
	g_input_flags.mech_brake_pressed = true;
	g_diagnostics.flags.mech_brake_pressed = true;
	motor_command_t motor_command = get_motor_command(ACCEL_DAC_OFF, REGEN_DAC_OFF);
	motor_command_package_and_send(&motor_command, true);
	DRD_diagnostics_transmit(&g_diagnostics, true);
}

/*
 * @brief helper functions to return the flags in the motor command message.
 */
uint8_t get_command_flags()
{
	uint8_t flags = 0;
	flags |= (g_drive_state == REVERSE  ? 0: 1); //direction value: 0 for reverse, 1 for forward or Park
	flags |= (g_input_flags.eco_mode_on ? 1 << 1: 0);
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

/*
 * @brief Helper function which clears drive state transition request flags
 *
 */
void clear_request_flags()
{
	g_input_flags.park_state_request = false;
	g_input_flags.forward_state_request = false;
	g_input_flags.reverse_state_request = false;
}


/*
 * @brief helper function which handles RPM to velocity conversions when FRAME 0 is received
 * 			from the motor controller
 *
 * 	@param Data	Data from recieved CAN message
 */
void velocity_CAN_msg_handle(uint8_t* data)
{
	uint32_t rpm = (data[4] >> 3) | ((data[5] & 0x7f) << 5); //35th to 46th bit
	float velocity = (WHEEL_RADIUS * 2.0 * M_PI * rpm) / 60.0;
	g_velocity_kmh = velocity * 3.6;

	if (velocity < VELOCITY_THRESHOLD)
	{
		g_input_flags.velocity_under_threshold = true;
	}

	else
	{
		g_input_flags.velocity_under_threshold = false;
	}
}

/*
 * @brief helper function which sets eco mode flags based on data receievd from
 * 		the steering wheel CAN message.
 */
void steering_CAN_msg_handle(uint8_t* data)
{
	g_input_flags.eco_mode_on = (data[0] >> 2); //third bit of steering CAN message
	g_eco_mode = g_input_flags.eco_mode_on; //global variable for LCD
}


#ifdef DEBUG

/*
 * @brief handler function for state transition CAN messages
 *
 * These CAN messages are only used to automate state transitions without button presses and don't
 * 		exist in the firmware in production. Used only for testing in debug.
 *
 */
void state_request_CAN_msg_handle(uint8_t* data)
{
	int value = data[0];
	if (value == 0)
	{
		g_input_flags.park_state_request = true;
	}
	else if (value == 1)
	{
		g_input_flags.reverse_state_request = true;
	}
	else if (value == 2)
	{
		g_input_flags.forward_state_request = true;
	}
}
#endif

