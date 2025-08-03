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

#define REGEN_DAC_ON  1023
#define REGEN_DAC_OFF 0
#define ACCEL_DAC_OFF 0

#define ADC_LOWER_DEADZONE 10 
#define ADC_NO_THROTTLE_MAX 630                 // https://ubcsolar26.monday.com/boards/7524367653/pulses/8891936447/posts/4032506875
#define ADC_FULL_THROTTLE_MIN 1350      

#define HIGHEST 1950     
#define LOWEST  800


#define ADC_UPPER_DEADZONE 4000 
#define ADC_MAX_DIFFERENCE 99999          // change when we have a better idea

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
extern volatile drive_state_t g_drive_state;
extern volatile bool g_lcd_eco_mode_on;
extern volatile uint32_t g_velocity_kmh;


/*	Functions	*/
void Drive_State_interrupt_handler(uint16_t pin);
void Vehicle_State_CAN_rx_handle(uint32_t msg_id, uint8_t* data);
void Drive_State_Machine_handler();
void Motor_Controller_query_data();



#endif /* INC_DRIVE_STATE_H_ */
