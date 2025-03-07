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
 *  Function prototypes
 */
void parse_batt_faults(uint8_t* can_rx_data, uint8_t current_sign);
void estop(uint8_t* can_rx_data);
void Set_fault_lights(uint32_t CAN_ID, uint8_t* data);

#endif /* INC_EXTERNAL_LIGHTS_H_ */
