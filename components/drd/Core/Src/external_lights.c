/*
 * external_lights.c
 *
 *	@brief   Contains functions to handle left and right turn signals, hazard lights, and
 *      day time running lights.
 *
 *  Created on: Feb 20, 2025
 *      Author: Evan Owens
 *
 *
 */

/*	Includes	*/
#include <stdint.h>
#include "external_lights.h"

/*	Local Function Prototypes	*/
static void Set_ExternalLights(uint8_t dtr, uint8_t lts, uint8_t rts);

/*	Symbolic Constants		*/
#define DTR_STATE		0
#define HAZARD_STATE 	1
#define LTS_STATE 		2
#define RTS_STATE 		3

/*	Global Variables	*/
volatile uint8_t left_turn_signal = 0;
volatile uint8_t right_turn_signal = 0;



/*
 * @brief State Machine to handle Vehicles External lights
 * 		Manages left and right turn signals, hazard signals, and
 * 		daytime running lights.
 *
 */
void ExternalLights_state_machine()
{
	uint32_t dtr = HAL_GPIO_ReadPin(DTR_IN_Pin, DTR_IN_Pin);
	uint32_t hazard_on = HAL_GPIO_ReadPin(HAZARD_GPIO_Port, HAZARD_Pin);
	static uint8_t prev_state = DTR_STATE;
	static uint8_t count = 0;
	static uint8_t lts = 0;
	static uint8_t rts = 0;

	if (hazard_on)
	{
		if (prev_state != HAZARD_STATE)
		{
			count = 0;
		}

		count++;

		if (count >= 10)
		{
			count = 0;
			lts = !lts;
			rts = !rts;
			dtr = 0;
		}

		prev_state = HAZARD_STATE;
	}
	else if (left_turn_signal)
	{
		if(prev_state != LTS_STATE)
		{
			count = 0;
		}

		count++;

		if (count >= 10)
		{
			lts = !lts;
		}

		rts = 0;
		dtr = 0;
		prev_state = LTS_STATE;
	}
	else if (right_turn_signal)
	{
		if (prev_state != RTS_STATE)
		{
			count = 0;
		}

		count++;

		if (count >= 10)
		{
			rts = !rts;
		}

		lts = 0;
		dtr = 0;
		prev_state = RTS_STATE;
	}
	else
	{
		lts = 0;
		rts = 0;
		prev_state = DTR_STATE;
	}

	Set_ExternalLights(dtr, lts, rts);
}


/*
 * @brief Outputs a given external lights state
 *
 * @param dtr, the state of the daytime running lights
 * @param lts, the state of the left turn signal
 * @param rts, the state of the right turn signal
 *
 * Each parameter should either be a 1 or a 0.
 */
void Set_ExternalLights(uint8_t dtr, uint8_t lts, uint8_t rts)
{
	HAL_GPIO_WritePin(LTS_OUT_GPIO_Port, LTS_OUT_Pin, lts ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(RTS_OUT_GPIO_Port, RTS_OUT_Pin, rts ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(DTR_OUT_GPIO_Port, DTR_OUT_Pin, dtr ? GPIO_PIN_SET : GPIO_PIN_RESET);
}


/*
 * @brief Sets turn signal global variables based on turn signal value passed in
 *
 * @param Lights_turn_signal_t signal indicating turn signal state
 */
//call this from CAN rx callback
void External_Lights_set_turn_signals(Lights_turn_signal_t signal)
{
	switch (signal)
	{
		case TURN_SIGNAL_OFF:
			left_turn_signal = 0;
			right_turn_signal = 0;
			break;

		case TURN_SIGNAL_LEFT:
			left_turn_signal = 1;
			right_turn_signal = 0;
			break;

		case TURN_SIGNAL_RIGHT:
			right_turn_signal = 1;
			left_turn_signal = 0;
			break;
	}


}
