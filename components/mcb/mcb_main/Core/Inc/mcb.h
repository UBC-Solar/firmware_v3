/*
 * 	mcb.h
 *  Created on: Jun. 1, 2023
 *  Author: Kyle Groulx
 */

#ifndef INC_MCB_H_
#define INC_MCB_H_

#include <stdbool.h>
#include "can.h"
#include "adc.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"

#define CAN_DATA_LENGTH 8			  // Length of a CAN message in bytes

#define P1 0.3						  // TODO find proper value for this

#define DELAY_MCB_STATE_MACHINE 10	  // Main mcb state machine delay time in ms

#define DELAY_GET_CAN_MESSAGES 10	  // Delay for getting CAN messages task

#define ADC_DEADZONE 500			  // Deadzone value for ADC
#define ADC_MAX 4096				  // Max possible value for ADC

#define VELOCITY_THRESHOLD 0.5     	  // Max velocity allowed during a drive state change (m/s)
#define CRUISE_INCREMENT_VAL 1 		  // Increment value for cruise up/down buttons
#define CRUISE_MAX 30 				  // Max cruise speed in m/s
#define CRUISE_MIN 5 				  // Min cruise speed	in m/s
#define CRUISE_THROTTLE 0.3            // Base line throttle in cruise control (0-1)

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
  volatile bool cruise_enabled;
  volatile bool mech_brake_pressed;
  volatile bool regen_enabled;
  volatile bool battery_SOC_under_threshold;
  volatile bool battery_temp_under_threshold;
  volatile bool velocity_under_threshold;
  volatile bool switch_pos_drive;
  volatile bool switch_pos_reverse;
  volatile bool switch_pos_park;
} InputFlags;

/*
 *  Struct that contains all information to send a motor command
 */
typedef struct MotorCommand {
  float throttle;
  float velocity;
} MotorCommand;


/*
 * Used to store the value of the drive state
*/
typedef enum {
	INVALID = (uint8_t) 0x00,
	DRIVE = (uint8_t) 0x01,
	CRUISE = (uint8_t) 0x02,
	PARK = (uint8_t) 0x03,
	REVERSE = (uint8_t) 0x04
}DriveState;


/*
 *  Variables
 */
extern InputFlags input_flags;
extern DriveState state;
extern float cruiseVelocity;
extern float velocityOfCar;
extern uint8_t gBatterySOC;

/*
 *  Functions
 */
void TaskMCBStateMachine();
void TaskGetCANMessage();

void SendCANMotorCommand(MotorCommand motorCommand);
float NormalizeADCValue(uint16_t value);
void SendCANDIDNextPage();
void SendCANDIDDriveState(DriveState state);

void GetSwitchState(InputFlags * input_flags);

MotorCommand GetMotorCommand(float throttle, float velocity);

MotorCommand DoStateDRIVE(InputFlags input_flags);
MotorCommand DoStateCRUISE(InputFlags input_flags);
MotorCommand DoStateREVERSE(InputFlags input_flags);
MotorCommand DoStatePARK(InputFlags input_flags);

void TransitionDRIVEstate( InputFlags input_flags, DriveState * state);
void TransitionREVERSEstate(InputFlags input_flags, DriveState * state);
void TransitionCRUISEstate(InputFlags input_flags, DriveState * state);
void TransitionPARKstate(InputFlags input_flags, DriveState * state);

void UpdateInputFlags(InputFlags * input_flags);


bool isBitSet(int num, int pos);
bool isBitSetFromArray(uint8_t * num, int pos);


#endif /* INC_MCB_H_ */
