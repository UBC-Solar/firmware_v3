/*
 * MCB.h
 *
 *  Created on: Jun. 1, 2023
 *      Author: Kyle Groulx
 */

#ifndef INC_MCB_H_
#define INC_MCB_H_

#include "can.h"

#define CAN_DATA_LENGTH 8			// Length of a CAN message in bytes

#define UPDATE_FLAGS_DELAY 5		// updateFlags rtos task delay time in ms
#define UPDATE_STATE_DELAY 5		// updateState rtos task delay time in ms
#define SEND_MOTOR_COMMAND_DELAY 10	// sendMotorCommand rtos task delay time in ms
#define GET_ADC_VALUES_DELAY 10		// getADCValues rtos task delay time in ms
#define GET_BATTERY_SOC_DELAY 5000	// getBatterySOC rtos task delay time in ms
#define GET_VELOCITY_DELAY 500		// getVelocity rtos task delay time in ms

#define ADC_DEADZONE 500			// Deadzone value for ADC
#define ADC_MAX 4096				// Max possible value for ADC

#define TRUE 1
#define FALSE 0

#define MIN_REVERSE_VELOCITY 3		// Minimum forward velocity before switching to the reverse state
#define CRUISE_INCREMENT_VAL 1 		// Increment value for cruise up/down buttons
#define CRUISE_MAX 30 				// Max cruise speed in m/s
#define CRUISE_MIN 5 				// Min cruise speed	in m/s

#define BATTERY_SOC_THRESHOLD 90	// Max battery state of charge for regenerative braking to be enabled.
#define BATTERY_SOC_FULL 100		// Full battery
#define BATTERY_SOC_EMPTY 0			// Empty battery


/*
 * Typedefs
 */
/*
 *  Union used for send floats as array for 4 bytes in CAN messages
 */
typedef union FloatBytes {
	float float_value;
	uint8_t bytes[4];
} FloatBytes;
/*
 *  Input flags used to decide what state to be in
 */
typedef struct InputFlags {
  volatile uint8_t regen_pressed;
  volatile uint8_t throttle_pressed;
  volatile uint8_t mech_brake_pressed;

  volatile uint8_t reverse_enabled;
  volatile uint8_t cruise_enabled;
  volatile uint8_t park_enabled;

  volatile uint8_t velocity_under_threshold;
  volatile uint8_t charge_under_threshold;
} InputFlags;

/*
 * Used to store the value of the drive state
 * Possibly replace with bools instead of uint32_t?
*/
enum DriveState {
	INVALID = (uint32_t) 0x0000, // Same behavior as IDLE
    IDLE = (uint32_t) 0x0001,
    DRIVE = (uint32_t) 0x0002,
    REGEN = (uint32_t) 0x0004,
	CRUISE = (uint32_t) 0x0008,
	MOTOR_OVERHEAT = (uint32_t) 0x0010, // Not implimented
	PARK = (uint32_t) 0x0012,
	REVERSE = (uint32_t) 0x0014
};
/*
 *
 */

/*
 *  Variables
 */
extern struct InputFlags event_flags; // Event flags for deciding what state to be in.
extern enum DriveState state;
extern float cruise_velocity; // Velocity for cruise control
extern float velocity_of_car; // Current velocity of the car will be stored here.

/*
 *  Functions
 */
void SendCANMotorCommand(float current, float velocity);
float NormalizeADCValue(float value);
void SendCANDIDNextPage();


#endif /* INC_MCB_H_ */
