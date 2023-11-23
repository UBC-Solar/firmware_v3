/*
 * mcb.c
 *
 *  Created on: Jun. 1, 2023
 *      Author: Kyle Groulx
 */

#include "mcb.h"

bool gBatteryRegenEnabled = false;
bool gBatterySOCUnderThreshold = false;

float gCruiseVelocity = 0; 	   // Velocity for cruise control
float gVelocityOfCar = 0; 	   // Current velocity of the car will be stored here.
uint8_t gBatterySOC = 0; 	   // Stores the charge of the battery, updated in a task.
InputFlags input_flags;
DriveState state;

/*
 *	Main state machine task
 */
void TaskMCBStateMachine()
{
	for(;;)
	{
		taskENTER_CRITICAL();

		uint16_t ADC_throttle_val;
		uint16_t ADC_regen_val;
		float velocity = 0.0;
		float current = 0.0;

		// Get ADC values from throttle and regen
		ADC_throttle_val = ReadADC(&hadc1);
		ADC_regen_val = ReadADC(&hadc2);

		// Set input flags
		input_flags.throttle_pressed = ADC_throttle_val > ADC_DEADZONE;
		input_flags.cruise_accelerate_enabled = NormalizeADCValue(ADC_throttle_val) > CRUISE_CURRENT;
		input_flags.regen_enabled = (gBatterySOC < BATTERY_SOC_THRESHOLD) && gBatteryRegenEnabled && HAL_GPIO_ReadPin(REGEN_EN_GPIO_Port, REGEN_EN_Pin);

		if(ADC_regen_val > ADC_DEADZONE)
			input_flags.cruise_enabled = false;

		input_flags.mech_brake_pressed = HAL_GPIO_ReadPin(BRK_IN_GPIO_Port, BRK_IN_Pin);
		input_flags.park_enabled = HAL_GPIO_ReadPin(PARK_EN_GPIO_Port, PARK_EN_Pin);
		input_flags.reverse_enabled = HAL_GPIO_ReadPin(RVRS_EN_GPIO_Port, RVRS_EN_Pin);
		input_flags.velocity_under_threshold = (gVelocityOfCar < MIN_REVERSE_VELOCITY);

		// Calculate state
		if (input_flags.park_enabled && input_flags.velocity_under_threshold)
			state = PARK;
		else if (input_flags.mech_brake_pressed)
			state = IDLE;
		else if (input_flags.regen_enabled)
		  	state = REGEN;
		else if (input_flags.cruise_enabled && input_flags.cruise_accelerate_enabled)
		  	state = CRUISE_ACCELERATE;
		else if (input_flags.cruise_enabled)
		    state = CRUISE;
		else if (input_flags.reverse_enabled && input_flags.velocity_under_threshold)
			state = REVERSE;
		else if (input_flags.throttle_pressed)
		    state = DRIVE;
		else
		  	state = IDLE;

		// Calculate velocity and current based on input_flags
		if(state == PARK)
		{
			velocity = 0.0;
			current = 1.0;
		}
		else if(state == REGEN)
	    {
	    	velocity = 0.0;
	    	current = NormalizeADCValue(ADC_regen_val);
	    }
	    else if(state == DRIVE)
	    {
	    	velocity = 100.0;
	    	current = NormalizeADCValue(ADC_throttle_val);
	    }
	    else if(state == REVERSE)
	    {
	    	velocity = -100.0;
	    	current = NormalizeADCValue(ADC_throttle_val);
	    }
	    else if (state == CRUISE)
	    {
	    	velocity = gCruiseVelocity;
	    	current = 1.0;
	    }
		else if (state == CRUISE_ACCELERATE)
		{
			velocity = 100.0;
			current = NormalizeADCValue(ADC_throttle_val);
		}

		// Send CAN message to motor controller
		SendCANMotorCommand(velocity, current);

		taskEXIT_CRITICAL();
		osDelay(DELAY_MCB_STATE_MACHINE);
	}
}

void ParseCANBatterySOC(uint8_t * CANMessageData)
{
	// if the battery SOC is out of range, assume it is at 100% as a safety measure
	if (CANMessageData[0] < BATTERY_SOC_EMPTY || CANMessageData[0] > BATTERY_SOC_FULL)
	{
		gBatterySOC = BATTERY_SOC_FULL;
	}
	else
	{
		gBatterySOC = CANMessageData[0];
	}

}

void ParseCANVelocity(uint8_t * CANMessageData)
{
	FloatBytes velocity;
	for(int i = 0; i < (sizeof(float)/sizeof(uint8_t)); i++)
	{
		velocity.bytes[i] = CANMessageData[i + 4]; // Vehicle Velocity is stored in bits 32-63.
	}
	gVelocityOfCar = velocity.float_value;
}

void ParseCANBatteryTemp(uint8_t * CANMessageData)
{
	gBatteryRegenEnabled = isBitSetFromArray(CANMessageData, 17); // regen_disable bit is stored in bit 17
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
void SendCANMotorCommand(float current, float velocity)
{
	union FloatBytes c;
	union FloatBytes v;

	c.float_value = current;
	v.float_value = velocity;

	uint8_t data_send[CAN_DATA_LENGTH];
	for (int i = 0; i < (uint8_t) CAN_DATA_LENGTH / 2; i++)
	{
		data_send[i] = v.bytes[i];
	    data_send[4 + i] = c.bytes[i];
	}
	HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
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


