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

// Build configuration macros
// #define CRUISE_ENABLED
// #define REGEN_ENABLED

#define CAN_DATA_LENGTH 8			  // Length of a CAN message in bytes

#define DELAY_MCB_STATE_MACHINE 50	  // Main mcb state machine delay time in ms

#define DELAY_GET_CAN_MESSAGES 10	  // Delay for getting CAN messages task

#define ADC_MAX 2880		  // Max possible value for ADC
#define ADC_MIN 1200      // Min possible value for ADC

#define THROTTLE_ADC_MIN_VALUE 200 
#define THROTTLE_ADC_MAX_VALUE 3896

#define VELOCITY_THRESHOLD 0.5     	  // Max velocity allowed during a drive state change (m/s)
#define CRUISE_INCREMENT_VAL 1 		  // Increment value for cruise up/down buttons
#define CRUISE_MAX 30 				  // Max cruise speed in m/s
#define CRUISE_MIN 5 				  // Min cruise speed	in m/s
#define CRUISE_THROTTLE 0.3            // Base line throttle in cruise control (0-1)

// TODO: refactor
#define VELOCITY_FORWARD 100.0
#define VELOCITY_REVERSE -100.0
#define VELOCITY_REGEN_DISABLED 0.0

/**
 * Zones of Operation:
 * 1. No foot on throttle/not moved from rest
 * 	  	In this Zone the ADC values were between 1150 to 1250. We dont want any motor spinnning
 * 		Thus, ADC_FOR_NO_SPIN is defined below. normalized adc value = 0
 * 2. Foot on throttle
 * 		This zone is > ADC_FOR_NO_SPIN up to ADC_MIN_FOR_FULL_THROTTLE. Based on experiment we found that putting the pedal so that its tip intersects 
 * 		with the brake cable give us a highest ADC value of 1830. Thus, ADC_MIN_FOR_FULL_THROTTLE is defined below
 * 		normalized adc value scales linearly from 0 to 1.
 * 3. Full Throttle:
 * 		This zone is > ADC_MIN_FOR_FULL_THROTTLE up to ADC_MAX_FOR_FULL_THROTTLE. This is at 1 inch past the intersection of the brake cable. 
 * 		Experiment shows 1 inch past brake cable is 1930 to 1966. Thus, ADC_MAX_FOR_FULL_THROTTLE is defined below
 * 		normalized adc value = 1.0
 * 4. Out of Range:
 * 		To protect against shorts, we will consider any value above ADC_MAX_FOR_FULL_THROTTLE as out of range. 
 * 		This means normalized adc values = 0.
 * Note: ADC_LOWER_DEADZONE is defined based on an ADC reading that is lower than anything we have seen. This is to protect against short to GND.
 */
#define ADC_LOWER_DEADZONE            900
#define ADC_FOR_NO_SPIN               1300    
#define ADC_MIN_FOR_FULL_THROTTLE     1830
#define ADC_MAX_FOR_FULL_THROTTLE     2000


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
  volatile bool battery_regen_disabled;
  volatile bool velocity_under_threshold;
  volatile bool switch_pos_drive;
  volatile bool switch_pos_reverse;
  volatile bool switch_pos_park;
  volatile bool throttle_ADC_out_of_range;
  volatile bool MDI_communication_fault;
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
void drive_state_machine_handler();
void TaskGetCANMessage();

void SendCANMotorCommand(MotorCommand motorCommand);
void SendCANDIDNextPage();
void SendCANDIDDriveState();
void send_mcb_diagnostics();
void send_mcb_githash();
float normalize_adc_value(uint16_t value);
void GetSwitchState(InputFlags * input_flags);

MotorCommand GetMotorCommand(float throttle, float velocity);

MotorCommand DoStateDRIVE(InputFlags input_flags);
MotorCommand DoStateCRUISE();
MotorCommand DoStateREVERSE(InputFlags input_flags);
MotorCommand DoStatePARK();

void TransitionDRIVEstate( InputFlags input_flags, DriveState * state);
void TransitionREVERSEstate(InputFlags input_flags, DriveState * state);
void TransitionCRUISEstate(InputFlags input_flags, DriveState * state);
void TransitionPARKstate(InputFlags input_flags, DriveState * state);

void UpdateInputFlags(InputFlags * input_flags);


bool isBitSet(int num, int pos);
bool isBitSetFromArray(uint8_t * num, int pos);


#endif /* INC_MCB_H_ */
