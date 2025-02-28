/*
 * external_lights.h
 *
 *  Created on: Feb 20, 2025
 *      Author: Evan Owens
 */

#ifndef INC_EXTERNAL_LIGHTS_H_
#define INC_EXTERNAL_LIGHTS_H_

/*	Includes	*/
#include "main.h"
#include "CAN_comms.h"

/*	Symbolic Constants	*/
#define LIGHTS_STATE_MACHINE_DELAY 50
#define LIGHTS_STATE_PERIOD 1000 //lights are on or off for 1 second at a time
#define LIGHTS_FLIP_COUNT  LIGHTS_STATE_PERIOD/LIGHTS_STATE_MACHINE_DELAY


/*	Typedefs 	*/


/*	Function Prototypes	*/
void ExternalLights_state_machine();
void External_Lights_set_turn_signals(CAN_comms_Rx_msg_t*  msg);


#endif /* INC_EXTERNAL_LIGHTS_H_ */
