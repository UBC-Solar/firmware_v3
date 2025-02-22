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


/*	Symbolic Constants	*/
#define LIGHTS_STATE_MACHINE_DELAY 50


/*	Typedefs 	*/
typedef enum {
	TURN_SIGNAL_OFF  = 0,
	TURN_SIGNAL_LEFT = 1,
	TURN_SIGNAL_RIGHT = 2
}Lights_turn_signal_t;

/*	Function Prototypes	*/
void ExternalLights_state_machine();
void External_Lights_set_turn_signals(Lights_turn_signal_t signal);


#endif /* INC_EXTERNAL_LIGHTS_H_ */
