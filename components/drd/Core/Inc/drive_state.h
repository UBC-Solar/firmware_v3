/*
 * drive_state.h
 *
 *  Created on: Mar 1, 2025
 *
 */

#ifndef INC_DRIVE_STATE_H_
#define INC_DRIVE_STATE_H_

/*	Includes	*/
#include <stdbool.h>
#include "can.h"
#include "adc.h"
#include "CAN_comms.h"


/*	Symbolic Constants	*/
#define DRIVE_STATE_MACHINE_DELAY 25


#define ADC_MAX 2880		  // Max possible value for ADC
#define ADC_MIN 1200      // Min possible value for ADC

#define THROTTLE_ADC_MIN_VALUE 200
#define THROTTLE_ADC_MAX_VALUE 3896
#define ADC_MAX_DIFFERENCE 100 // change when we have a better idea


#define REGEN_DAC_ON 1023
#define REGEN_DAC_OFF 0
#define ACCEL_DAC_OFF 0

#define ADC_NO_THROTTLE_MAX 0 //figure out
#define ADC_FULL_THROTTLE_MIN 0 //figure out
#define ADC_THROTTLE_MAX	0 //figure out
#define ADC_LOWER_DEADZONE 0 //tbd
#define ADC_UPPER_DEADZONE 0 //tbd

#define MOTOR_DRIVE_COMMAND_ADDRESS 0x401

/*	Data Types	*/
typedef struct{
	volatile bool mech_brake_pressed;
	volatile bool regen_enabled;
	volatile bool battery_regen_disabled;
	volatile bool velocity_under_threshold;
	volatile bool forward_state_request;
	volatile bool reverse_state_request;
	volatile bool park_state_request;
	volatile bool throttle_ADC_out_of_range;
	volatile bool eco_mode_on;
	volatile uint16_t accel_DAC_value;
	volatile uint16_t regen_DAC_value; //currently unused, but we could implemented down the line
} input_flags_t;

typedef struct {
	uint16_t accel_DAC_value;
	uint16_t regen_DAC_value;
	uint8_t motor_command_flags; //direction value first bit and eco mode value second bit.
} motor_command_t;

typedef enum {
	INVALID = (uint8_t) 0x00,
	FORWARD = (uint8_t) 0x01,
	CRUISE = (uint8_t) 0x02,
	PARK = (uint8_t) 0x03,
	REVERSE = (uint8_t) 0x04
} drive_state_t;

/*	Global Variables	*/

extern input_flags_t input_flags;
extern drive_state_t state;


/*	Functions	*/
void drive_state_interrupt_handler(uint16_t pin);
void Drive_State_can_rx_handle(CAN_comms_Rx_msg_t* can_msg);
void Drive_State_Machine_handler();



#endif /* INC_DRIVE_STATE_H_ */
