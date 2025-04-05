/*
 * fault_lights.c
 *
 *	@brief   Contains functions to handle fault lights
 *
 *  Created on: March 01, 2025
 *      Author: Mridul Singh
 */
#ifndef INC_FAULT_LIGHTS_H_
#define INC_FAULT_LIGHTS_H_

/*	Includes	*/
#include "main.h"
#include "CAN_comms.h"

/*
 *  Typedefs
 */
typedef union U16Bytes {
	uint16_t U16_value;
	uint8_t bytes[2];
} U16Bytes;

/*
 * User Defines
 */
#define HIGH 1
#define LOW  0

#define PACK_VOLTAGE_DIVISOR	468
#define MAX_PACK_VOLTAGE 	 	134.4
#define MIN_PACK_VOLTAGE 	 	86.72

/*
 *  Function prototypes
 */
void Fault_Lights_CAN_rx_handle(uint32_t CAN_ID, uint8_t* data);
void parse_batt_faults(uint8_t* can_rx_data);
void estop(uint8_t* can_rx_data);
void parse_mtr_faults(uint8_t* can_rx_data);
void Set_fault_lights(uint32_t CAN_ID, uint8_t* data);
void batt_pack_voltage_fault(uint8_t* can_rx_data);

#endif /* INC_EXTERNAL_LIGHTS_H_ */
