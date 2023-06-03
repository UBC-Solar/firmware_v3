/*
 * MCB.h
 *
 *  Created on: Jun. 1, 2023
 *      Author: kcgro
 */

#ifndef INC_MCB_H_
#define INC_MCB_H_

#include "can.h"

#define CAN_DATA_LENGTH 8
#define ADC_POLL_DELAY 10
#define SEND_MOTOR_COMMAND_DELAY 10
#define ADC_DEADZONE 500
#define ADC_MAX 4096
#define TRUE 1
#define FALSE 0

#define EVENT_FLAG_UPDATE_DELAY 5
#define MIN_REVERSE_VELOCITY 3
#define GET_CAN_VELOCITY_DELAY 500
#define READ_BATTERY_SOC_DELAY 5000
#define DELAY_MOTOR_STATE_MACHINE 10

#define CRUISE_INCREMENT_VAL 1 // How much pressing the cruise up/down buttons increase or decrease the cruise velocity
#define CRUISE_MAX 30 // Max cruise speed
#define CRUISE_MIN 5 // Min cruise speed

#define BATTERY_SOC_THRESHHOLD 90


// Typedefs
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

// Function definitions
void SendCANMotorCommand(float current, float velocity);
float NormalizeValue(float value);


#endif /* INC_MCB_H_ */
