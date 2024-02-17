#include <stdint.h>
#include "cyclic_data.h"

/*
 *	Defines
 */
#define CAN_ID_DID_NEXT_PAGE    0x400
#define CAN_ID_FAULTS		 	0x622
#define CAN_ID_SIMULATION_SPEED 0x750
#define CAN_ID_BATTERY_SOC		0x624
#define CAD_ID_MOTOR_COMMAND 	0x401
#define CAN_ID_VEHICLE_VELOCITY 0x503
#define CAN_ID_MCB_DRIVE_STATE 	0x403
#define CAN_ID_MOTOR_CURRENT    0x506
#define CAN_ID_ARRAY_CURRENT	0x702
#define CAN_ID_PACK_CURRENT		0x450
#define CAN_ID_PACK_VOLTAGE		0x623
#define CAN_ID_PACK_TEMPERATURE 0x625

#define DID_REFRESH_DELAY		100	    // How often the DID will refresh the screen (ms)
#define DID_PAGE_TIMEOUT        10000   // Time in ms until the DID changes back to the main page when on another page

#define NUM_PAGES 4
#define PAGE_0 0
#define PAGE_1 1
#define PAGE_2 2
#define PAGE_3 3

#define TRUE 1
#define FALSE 0

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