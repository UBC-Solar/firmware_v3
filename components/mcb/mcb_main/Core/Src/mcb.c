/*
 * mcb.c
 *
 *  Created on: Jun. 1, 2023
 *      Author: Kyle Groulx
 */

#include "mcb.h"

float cruiseVelocity = 0.0;		// Velocity for cruise control
float velocityOfCar = 0.0;		// Current velocity of the car will be stored here.
InputFlags input_flags;
DriveState state = PARK;

/*
 *	Main state machine task
 */
void TaskMCBStateMachine()
{
	for(;;)
	{
		taskENTER_CRITICAL();
		MotorCommand motorCommand;
		UpdateInputFlags(&input_flags);

		switch(state)
		{
		case DRIVE:
			motorCommand = DoStateDRIVE(input_flags);
		break;

		case CRUISE:
			motorCommand = DoStateCRUISE(input_flags);
		break;

		case REVERSE:
			motorCommand = DoStateREVERSE(input_flags);
		break;

		case PARK:
			motorCommand = DoStatePARK(input_flags);
		break;

		default:
			motorCommand.velocity = 0.0;
			motorCommand.throttle = 0.0;
		}

		SendCANMotorCommand(motorCommand);

		taskEXIT_CRITICAL();
		osDelay(DELAY_MCB_STATE_MACHINE);
	}
}


MotorCommand DoStateDRIVE( InputFlags input_flags )
{
	MotorCommand motorCommand;

	uint16_t ADC_throttle_val = ReadADC(&hadc1);
	motorCommand.throttle = NormalizeADCValue(ADC_throttle_val);

	if ( input_flags.mech_brake_pressed )
	{
		motorCommand.velocity = 0.0;
		motorCommand.throttle = 0.0;
	}
	else if ( input_flags.regen_enabled && input_flags.battery_SOC_under_threshold && input_flags.battery_temp_under_threshold )
	{
		if( motorCommand.throttle == 0.0 )
		{
			motorCommand.velocity = 0.0;
		}
		else if( motorCommand.throttle > 0.0 )
		{
			motorCommand.velocity = 100.0;
		}
	}
	else // Regen disabled
	{
		#ifdef MITSUBA
			if(motorCommand.throttle == 0.0)
			{
				motorCommand.velocity = 0.0;
				motorCommand.throttle = 0.0;
			}
			else if(motorCommand.throttle < P1)
			{
				motorCommand.velocity = 100.0;
				motorCommand.throttle = P1;
			}
			else if(motorCommand.throttle > P1)
			{
				motorCommand.velocity = 100.0;
			}
		#endif
		#ifdef TRITIUM
			if(motorCommand.throttle == 0.0)
			{
				motorCommand.velocity = 0.0;
			}
			else if(motorCommand.throttle > 0.0)
			{
				motorCommand.velocity = 100.0;
			}
		#endif
	}

	/*
	 *  State transitions
	 */
	if( input_flags.reverse_enabled && input_flags.velocity_under_threshold )
	{
		state = REVERSE;
	}
	else if( input_flags.park_enabled && input_flags.velocity_under_threshold )
	{
		state = PARK;
	}
	else if( input_flags.cruise_enabled )
	{
		state = CRUISE;
	}

	return motorCommand;
}


MotorCommand DoStateCRUISE(InputFlags input_flags)
{
	MotorCommand motorCommand;

	motorCommand.velocity = cruiseVelocity;
	motorCommand.throttle = CRUISE_THROTTLE;
	/*
	 *  State transitions
	 */
	if ( input_flags.mech_brake_pressed || !input_flags.cruise_enabled)
	{
		state = DRIVE;
	}
	return motorCommand;
}

MotorCommand DoStateREVERSE(InputFlags input_flags)
{
	MotorCommand motorCommand;
	#ifdef MITSUBA
		if(motorCommand.throttle == 0.0)
		{
			motorCommand.velocity = 0.0;
			motorCommand.throttle = 0.0;
		}
		else if(motorCommand.throttle < P1)
		{
			motorCommand.velocity = -100.0;
			motorCommand.throttle = P1;
		}
		else if(motorCommand.throttle > P1)
		{
			motorCommand.velocity = -100.0;
		}
	#endif
	#ifdef TRITIUM
		if(motorCommand.throttle == 0.0)
		{
			motorCommand.velocity = 0.0;
		}
		else if(motorCommand.throttle > 0.0)
		{
			motorCommand.velocity = -100.0;
		}
	#endif

	/*
	 *  State transitions
	 */
	if ( input_flags.drive_enabled && input_flags.mech_brake_pressed && input_flags.velocity_under_threshold )
	{
		state = DRIVE;
	}
	else if ( input_flags.park_enabled && input_flags.mech_brake_pressed && input_flags.velocity_under_threshold )
	{
		state = PARK;
	}
	return motorCommand;
}

MotorCommand DoStatePARK(InputFlags input_flags)
{
	MotorCommand motorCommand;
	motorCommand.velocity = 0.0;
	motorCommand.throttle = 1.0;

	/*
	 *  State transitions
	 */
	if ( input_flags.drive_enabled && input_flags.mech_brake_pressed && input_flags.velocity_under_threshold )
	{
		state = DRIVE;
	}
	else if ( input_flags.reverse_enabled && input_flags.mech_brake_pressed && input_flags.velocity_under_threshold )
	{
		state = REVERSE;
	}
	return motorCommand;
}

void UpdateInputFlags(InputFlags * input_flags)
{
	input_flags->drive_enabled = !HAL_GPIO_ReadPin(FWRD_EN_GPIO_Port, FWRD_EN_Pin);
	input_flags->drive_enabled = !HAL_GPIO_ReadPin(RVRS_EN_GPIO_Port, RVRS_EN_Pin);
	input_flags->drive_enabled = !HAL_GPIO_ReadPin(PARK_EN_GPIO_Port, PARK_EN_Pin);
	input_flags->mech_brake_pressed = !HAL_GPIO_ReadPin(BRK_IN_GPIO_Port, BRK_IN_Pin);
	input_flags->regen_enabled = !HAL_GPIO_ReadPin(REGEN_EN_GPIO_Port, REGEN_EN_Pin);
}


void ParseCANBatterySOC(uint8_t * CANMessageData)
{
	uint8_t batterySOC = CANMessageData[0];
	input_flags.battery_SOC_under_threshold = batterySOC < BATTERY_SOC_THRESHOLD;
}

void ParseCANVelocity(uint8_t * CANMessageData)
{
	FloatBytes velocity;
	for(int i = 0; i < (sizeof(float)/sizeof(uint8_t)); i++)
	{
		velocity.bytes[i] = CANMessageData[i + 4]; // Vehicle Velocity is stored in bits 32-63.
	}
	velocityOfCar = velocity.float_value;
	input_flags.velocity_under_threshold = velocity.float_value < VELOCITY_THRESHOLD;
}

void ParseCANBatteryTemp(uint8_t * CANMessageData)
{
	input_flags.battery_temp_under_threshold = isBitSetFromArray(CANMessageData, 17); // regen_disable bit is stored in bit 17
}

void TaskGetCANMessage()
{
	uint8_t CANMessageData[CAN_DATA_LENGTH];
	/* Infinite loop */
	for(;;)
	{
		// Check if there is a CAN Message
		if (HAL_CAN_GetRxFifoFillLevel(&hcan, CAN_RX_FIFO0))
		{
			HAL_CAN_GetRxMessage(&hcan, CAN_RX_FIFO0, &can_rx_header, CANMessageData);

			if (can_rx_header.StdId == CAN_ID_BATTERY_SOC)
			{
				ParseCANBatterySOC(CANMessageData);
			}
			else if (can_rx_header.StdId == CAN_ID_VELOCITY)
			{
				ParseCANVelocity(CANMessageData);
			}
			else if (can_rx_header.StdId == CAN_ID_BATTERY_TEMP)
			{
				ParseCANBatteryTemp(CANMessageData);
			}
		}
		osDelay(GET_BATTERY_SOC_DELAY);
	}
	/* USER CODE END getBatterySOC */
}




/*
 *   Takes current value and velocity float value and sends in via CAN as an array of bytes.
 */
void SendCANMotorCommand(MotorCommand motorCommand)
{
	union FloatBytes c;
	union FloatBytes v;

	c.float_value = motorCommand.throttle;
	v.float_value = motorCommand.velocity;

	uint8_t data_send[CAN_DATA_LENGTH];
	for (int i = 0; i < (uint8_t) CAN_DATA_LENGTH / 2; i++)
	{
		data_send[i] = v.bytes[i];
	    data_send[4 + i] = c.bytes[i];
	}
	HAL_CAN_AddTxMessage(&hcan, &drive_command_header, data_send, &can_mailbox);
}

/*
 *  Function used for normalizing(0-1) and accounting for deadzone of ADC inputs.
 */
float NormalizeADCValue(uint16_t value)
{
	return (value - ADC_DEADZONE >= 0 ? ((float)(value - ADC_DEADZONE))/(ADC_MAX - ADC_DEADZONE) : 0.0);
}

/*
 *  Function used for sending the Next Page command to DID.
 */
void SendCANDIDNextPage()
{
	uint8_t data_send[CAN_DATA_LENGTH] = {0};
	SETBIT(data_send[0], 0);
	HAL_CAN_AddTxMessage(&hcan, &DID_next_page_header, data_send, &can_mailbox);
}

/*
 * 	Sends a CAN message to the DID that contains the drive state of the MCB to display to the driver
 *  NOTE: The drive state displayed on the DID is a simplified version of the internal state of the MCB,
 *  IE there are only 5 states that can be shown on the DID: Drive, Regen, Cruise, Park and Reverse.
 *  INVALID = 0x00	// Should never be in INVALID state
 *  DRIVE   = 0x02
 *	CRUISE  = 0x04
 * 	PARK    = 0x06
 *	REVERSE = 0x07
 */
void SendCANDIDDriveState(DriveState state)
{
	/*
	static DriveState lastState = PARK;
	uint8_t data_send[CAN_DATA_LENGTH] = {0};

	if( state == DRIVE || state == CRUISE || state == PARK || state == REVERSE )
	{
		data_send[1] = state;
		lastState = state;
	}
	else if( state == IDLE && (lastState == DRIVE || lastState == REGEN || lastState == REVERSE) )
	{
		data_send[1] = lastState; // If in the IDLE state, use the last used state
	}
	else if( state == REGEN && (lastState == DRIVE || lastState == REGEN || lastState == REVERSE) )
	{
		data_send[1] = lastState; // If in the IDLE state, use the last used state
	}
	else if (state == CRUISE_ACCELERATE )
	{
		data_send[1] = CRUISE; // If in the CRUISE_ACCELERATE state, send the CRUISE state
	}
	else
	{
		data_send[1] = INVALID; // Should never be in this state.
	}

	HAL_CAN_AddTxMessage(&hcan, &DID_next_page_header, data_send, &can_mailbox);
	*/
}


bool isBitSet(int num, int pos)
{
	return (num & (1 << pos)) != 0;
}

bool isBitSetFromArray(uint8_t * num, int pos)
{
	int index = pos / 8;
	int remainder = pos % 8;
	return isBitSet(num[index], remainder);
}


