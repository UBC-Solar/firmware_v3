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



typedef union FloatBytes {
	float float_value;			/**< Float value member of the union. */
	uint8_t bytes[4];			/**< Array of 4 bytes member of union. */
} FloatBytes;

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

enum DriveState {
	INVALID = (uint32_t) 0x0000, // Same behavior as IDLE
    IDLE = (uint32_t) 0x0001,
    DRIVE = (uint32_t) 0x0002,
    REGEN = (uint32_t) 0x0004,
	CRUISE = (uint32_t) 0x0008,
	MOTOR_OVERHEAT = (uint32_t) 0x0010, // Not implimented
	PARK = (uint32_t) 0x0012,
	REVERSE = (uint32_t) 0x0014
} state;


void SendCANMotorCommand(float current, float velocity)
{
	union FloatBytes c;
	union FloatBytes v;

	c.float_value = current;
	v.float_value = velocity;

	uint8_t data_send[CAN_DATA_LENGTH];
	for (int i = 0; i < (uint8_t) CAN_DATA_LENGTH / 2; i++)
	{
		data_send[i] = v.bytes[i];
	    data_send[4 + i] = c.bytes[i];
	}
	HAL_CAN_AddTxMessage(&hcan, &drive_command_header, data_send, &can_mailbox);
}

float NormalizeValue(float value)
{
	return (value - ADC_DEADZONE >= 0 ? ((float)(value - ADC_DEADZONE))/(ADC_MAX - ADC_DEADZONE) : 0.0);
}


#endif /* INC_MCB_H_ */
