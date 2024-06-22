#include <stdint.h>
#include "cyclic_data.h"

/*
 *	CAN IDs
 */
#define CAN_ID_DID_NEXT_PAGE    0x400
#define CAD_ID_MOTOR_COMMAND 	0x401
#define CAN_ID_MCB_DRIVE_STATE 	0x403

#define CAN_ID_PACK_CURRENT		0x450

#define CAN_ID_VEHICLE_VELOCITY 0x503
#define CAN_ID_MOTOR_CURRENT    0x506

#define CAN_ID_FAULTS		 	0x622
#define CAN_ID_PACK_VOLTAGE		0x623
#define CAN_ID_BATTERY_SOC		0x624
#define CAN_ID_PACK_TEMPERATURE 0x625

#define CAN_ID_ARRAY_CURRENT	0x702

#define CAN_ID_SIMULATION_SPEED 0x750

#define CAN_ID_HEARTBEAT 		0x580

/*
 *	Cycle times (ms)
 */
#define MAX_CYCLE_TIME_INCREASE 1.2

#define CYCLE_TIME_MOTOR_COMMAND 	 	100
#define MAX_CYCLE_TIME_MOTOR_COMMAND 	CYCLE_TIME_MOTOR_COMMAND * MAX_CYCLE_TIME_INCREASE

#define CYCLE_TIME_MCB_DRIVE_STATE	 	100
#define MAX_CYCLE_TIME_MCB_DRIVE_STATE 	CYCLE_TIME_MCB_DRIVE_STATE * MAX_CYCLE_TIME_INCREASE

#define CYCLE_TIME_PACK_CURRENT			200
#define MAX_CYCLE_TIME_PACK_CURRENT		CYCLE_TIME_PACK_CURRENT * MAX_CYCLE_TIME_INCREASE

#define CYCLE_TIME_VEHICLE_VELOCITY		200
#define MAX_CYCLE_TIME_VEHICLE_VELOCITY	CYCLE_TIME_VEHICLE_VELOCITY * MAX_CYCLE_TIME_INCREASE

#define CYCLE_TIME_MOTOR_CURRENT		200
#define MAX_CYCLE_TIME_MOTOR_CURRENT	CYCLE_TIME_MOTOR_CURRENT * MAX_CYCLE_TIME_INCREASE

#define CYCLE_TIME_PACK_VOLTAGE			1000
#define MAX_CYCLE_TIME_PACK_VOLTAGE		CYCLE_TIME_PACK_VOLTAGE * MAX_CYCLE_TIME_INCREASE

#define CYCLE_TIME_BATTERY_SOC			1000
#define MAX_CYCLE_TIME_BATTERY_SOC		CYCLE_TIME_BATTERY_SOC * MAX_CYCLE_TIME_INCREASE

#define CYCLE_TIME_PACK_TEMPERATURE		1000
#define MAX_CYCLE_TIME_PACK_TEMPERATURE CYCLE_TIME_PACK_TEMPERATURE * MAX_CYCLE_TIME_INCREASE

#define CYCLE_TIME_ARRAY_CURRENT		1000
#define MAX_CYCLE_TIME_ARRAY_CURRENT	CYCLE_TIME_ARRAY_CURRENT * MAX_CYCLE_TIME_INCREASE

#define CYCLE_TIME_SIMULATION_SPEED 	300000
#define MAX_CYCLE_TIME_SIMULATION_SPEED CYCLE_TIME_SIMULATION_SPEED * MAX_CYCLE_TIME_INCREASE


#define DID_REFRESH_DELAY		100	    // How often the DID will refresh the screen (ms)
#define DID_PAGE_TIMEOUT        10000   // Time in ms until the DID changes back to the main page when on another page
#define DID_HEARTBEAT_CYCLETIME 1000    // Time in ms between DID heartbeats

#define NUM_PAGES 4
#define PAGE_0 0
#define PAGE_1 1
#define PAGE_2 2
#define PAGE_3 3

#define TRUE 1
#define FALSE 0

#define MS_TO_KM(ms) (ms * 3.6)
#define MS_TO_MPH(ms) (ms * 2.23694)
/*
 *  Typedefs
 */
typedef union FloatBytes {
	float float_value;
	uint8_t bytes[4];
} FloatBytes;

typedef union U16Bytes {
	uint16_t U16_value;
	uint8_t bytes[2];
} U16Bytes;

typedef union S8Byte {
	int8_t S8_value;
	uint8_t byte;
} S8Byte;

/*
 *  Function prototypes
 */
void parse_can_message( uint8_t* CAN_rx_data, uint32_t CAN_ID );
void update_DID_screen();
void parse_warnings(uint8_t* CAN_rx_data);
void DID_timeout();
void send_did_heartbeat();
