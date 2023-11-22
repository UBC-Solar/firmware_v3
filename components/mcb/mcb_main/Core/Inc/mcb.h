/*
 * 	mcb.h
 *  Created on: Jun. 1, 2023
 *  Author: Kyle Groulx
 */

#ifndef INC_MCB_H_
#define INC_MCB_H_

#include "can.h"
#include "adc.h"
#include "can.h"
#include "stdbool.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"

#define CAN_DATA_LENGTH 8			  // Length of a CAN message in bytes



#define DELAY_MCB_STATE_MACHINE 10	  // Main mcb state machine delay time in ms

#define GET_BATTERY_SOC_DELAY 5000	  // getBatterySOC rtos task delay time in ms
#define GET_VELOCITY_DELAY 500		  // getVelocity rtos task delay time in ms

#define ADC_DEADZONE 500			  // Deadzone value for ADC
#define ADC_MAX 4096				  // Max possible value for ADC

#define MIN_REVERSE_VELOCITY 3		  // Minimum forward velocity before switching to the reverse state
#define CRUISE_INCREMENT_VAL 1 		  // Increment value for cruise up/down buttons
#define CRUISE_MAX 30 				  // Max cruise speed in m/s
#define CRUISE_MIN 5 				  // Min cruise speed	in m/s
#define CRUISE_CURRENT 0.3            // Base line current in cruise control (0-1)

#define BATTERY_SOC_THRESHOLD 90	  // Max battery state of charge for regenerative braking to be enabled.
#define BATTERY_SOC_FULL 100		  // Full battery
#define BATTERY_SOC_EMPTY 0			  // Empty battery


#define SETBIT(x, bitpos) (x |= (1 << bitpos))
#define GETBIT(x, bitpos) ((x >> bitpos) & 1)

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

typedef union IntBytes {
	int int_value;
	uint8_t bytes[4];
} IntBytes;

/*
 *  Input flags used to decide what state to be in
 */
typedef struct InputFlags {
  volatile bool regen_pressed;
  volatile bool throttle_pressed;
  volatile bool mech_brake_pressed;

  volatile bool reverse_enabled;
  volatile bool cruise_enabled;
  volatile bool park_enabled;
  volatile bool cruise_accelerate_enabled;
  volatile bool regen_switch_enabled;


  volatile bool velocity_under_threshold;
  volatile bool charge_under_threshold;
  volatile bool regen_disable;
} InputFlags;

/*
 * Used to store the value of the drive state
 * See mcb specs doc/wiki for specifications of each state.
*/
typedef enum {
	INVALID = (uint8_t) 0x00,
	IDLE = (uint8_t) 0x01,
	DRIVE = (uint8_t) 0x02,
	REGEN = (uint8_t) 0x03,
	CRUISE = (uint8_t) 0x04,
	CRUISE_ACCELERATE = (uint8_t) 0x05,
	PARK = (uint8_t) 0x06,
	REVERSE = (uint8_t) 0x07
}DriveState;

/*
 *  Variables
 */
extern InputFlags input_flags;
extern DriveState state;
extern float gCruiseVelocity;
extern float gVelocityOfCar;
extern uint8_t gBatterySOC;

/*
 *  Functions
 */
void TaskMCBStateMachine();

void SendCANMotorCommand(float current, float velocity);
float NormalizeADCValue(uint16_t value);
void SendCANDIDNextPage();
void SendCANDIDDriveState(DriveState state);

bool isBitSet(int num, int pos);


#endif /* INC_MCB_H_ */
