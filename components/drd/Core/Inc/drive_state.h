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
#define DRIVE_STATE_MACHINE_DELAY 	25
#define DRIVE_COMMAND_SIZE 			5
#define MC_FRAME_REQUEST_DELAY 		100


#define ADC_MAX 2880		  // Max possible value for ADC
#define ADC_MIN 1200      // Min possible value for ADC

#define THROTTLE_ADC_MIN_VALUE 200
#define THROTTLE_ADC_MAX_VALUE 3896
#define ADC_MAX_DIFFERENCE 600 // change when we have a better idea


#define REGEN_DAC_ON 1023
#define REGEN_DAC_OFF 0
#define ACCEL_DAC_OFF 0

#define ADC_NO_THROTTLE_MAX 1000 //figure out
#define ADC_FULL_THROTTLE_MIN 1850 //figure out
#define ADC_THROTTLE_MAX	0 //figure out
#define ADC_LOWER_DEADZONE 900 //tbd
#define ADC_UPPER_DEADZONE 4000 //tbd


#define WHEEL_RADIUS 0.283
#define M_PI 3.14159
#define VELOCITY_THRESHOLD 0.5     	  // Max velocity allowed during a drive state change (m/s)

/*	Data Types	*/
typedef struct{
	volatile bool mech_brake_pressed;
	volatile bool regen_enabled;
	volatile bool velocity_under_threshold;
	volatile bool forward_state_request;
	volatile bool reverse_state_request;
	volatile bool park_state_request;
	volatile bool eco_mode_on;
} input_flags_t;

typedef union {
	struct {
		volatile bool mech_brake_pressed 		: 1;
		volatile bool regen_enabled 			: 1;
		volatile bool throttle_ADC_out_of_range : 1;
		volatile bool throttle_ADC_mismatch 	: 1;
		volatile bool watchdog_reset 			: 1;
		volatile bool motor_comm_fault 			: 1;
	};
	uint8_t all_flags;
} DRD_flags_t;

typedef struct {
	volatile uint16_t raw_adc1;
	volatile uint16_t raw_adc2;
	DRD_flags_t flags;
} DRD_diagnostic_t;

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

extern input_flags_t g_input_flags;
extern DRD_diagnostic_t g_diagnostics;
extern volatile drive_state_t g_drive_state;
extern volatile bool g_eco_mode;
extern volatile uint32_t g_velocity_kmh;


/*	Functions	*/
void Drive_State_interrupt_handler(uint16_t pin);
void Drive_State_CAN_rx_handle(uint32_t msg_id, uint8_t* data);
void Drive_State_Machine_handler();
void Motor_Controller_query_data();
void DRD_diagnostics_transmit(DRD_diagnostic_t* diagnostics, bool from_ISR);



#endif /* INC_DRIVE_STATE_H_ */
