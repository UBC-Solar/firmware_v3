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
MotorCommand motorCommand;
float g_throttle = 0.0;
uint16_t g_throttle_ADC = 0;
/*
 *	Main state machine task
 */
void drive_state_machine_handler()
{
	UpdateInputFlags(&input_flags);

	uint16_t g_throttle_ADC = ReadADC(&hadc1);

	g_throttle = normalize_adc_value(g_throttle_ADC);

	switch(state)
	{
		case DRIVE:
			motorCommand = DoStateDRIVE(input_flags);
			TransitionDRIVEstate(input_flags, &state);
		break;

		case REVERSE:
			motorCommand = DoStateREVERSE(input_flags);
			TransitionREVERSEstate(input_flags, &state);
		break;

		case PARK:
			motorCommand = DoStatePARK(input_flags);
			TransitionPARKstate(input_flags, &state);
		break;

		case CRUISE:
			motorCommand = DoStateCRUISE(input_flags);
			TransitionCRUISEstate(input_flags, &state);
		break;

		default:
			motorCommand = GetMotorCommand(0.0, 0.0);
	}

	SendCANMotorCommand(motorCommand);
}

/*
 * Sends MCB diagnostics over CAN
 * Bits 0-15: pedal ADC reading
 * Bit 16: pedal ADC out of range
 * Bit 17: regen on
 * Bit 18: cruise on
 */
void send_mcb_diagnostics()
{
	uint8_t data_send[CAN_DATA_LENGTH] = {0};

	data_send[0] = g_throttle_ADC & 0xFF;
	data_send[1] = (g_throttle_ADC >> 8) & 0xFF;

	if (input_flags.throttle_ADC_out_of_range == true)	
	{
		SETBIT(data_send[2], 0);
	}

	if (input_flags.regen_enabled == true)
	{
		SETBIT(data_send[2], 1);
	}

	if (input_flags.cruise_enabled == true)
	{
		SETBIT(data_send[2], 2);
	}

	if (input_flags.mech_brake_pressed == true)
	{
		SETBIT(data_send[2], 3);
	}

	HAL_CAN_AddTxMessage(&hcan, &mcb_diagnostics, data_send, &can_mailbox);

	return;
}

MotorCommand DoStateDRIVE( InputFlags input_flags )
{
	// Check if mech brake is pressed
	if ( input_flags.mech_brake_pressed )
		return GetMotorCommand(0.0, 0.0);

	// Check if regen is enabled
	if ( input_flags.regen_enabled && input_flags.battery_SOC_under_threshold && input_flags.battery_temp_under_threshold )
	{
		if( g_throttle == 0.0 )
			return GetMotorCommand(g_throttle, 0.0);
		if( g_throttle > 0.0 )
			return GetMotorCommand(g_throttle, 100.0);
	}

	// Regen disabled
	#ifdef MITSUBA
		if( g_throttle == 0.0 )
			return GetMotorCommand(g_throttle, 0.0);
		if( g_throttle < P1 )
			return GetMotorCommand(P1, 100.0);
		if( g_throttle > P1)
			return GetMotorCommand(g_throttle, 100.0);
	#endif
	#ifdef TRITIUM
		if( g_throttle == 0.0 )
			return GetMotorCommand( 0.0, 100.0 );
		if( g_throttle > 0.0 )
			return GetMotorCommand( g_throttle, 100.0 );
	#endif

	return GetMotorCommand(0.0,0.0);
}

void TransitionDRIVEstate( InputFlags input_flags, DriveState * state)
{
	if( input_flags.switch_pos_reverse && input_flags.velocity_under_threshold )
		*state = REVERSE;
	else if( input_flags.switch_pos_park && input_flags.velocity_under_threshold )
		*state = PARK;
	else if( input_flags.cruise_enabled )
		*state = CRUISE;
}


MotorCommand DoStateREVERSE(InputFlags input_flags)
{
	// Check if mech brake is pressed
	if ( input_flags.mech_brake_pressed )
		return GetMotorCommand(0.0, 0.0);

	// Regen disabled
	#ifdef MITSUBA
		if( g_throttle == 0.0 )
			return GetMotorCommand(g_throttle, 0.0);
		if( g_throttle < P1 )
			return GetMotorCommand(P1, -100.0);
		if( g_throttle > P1)
			return GetMotorCommand(g_throttle, -100.0);
	#endif
	#ifdef TRITIUM
		if( g_throttle == 0.0 )
			return GetMotorCommand( 0.0, -100.0 );
		if( g_throttle > 0.0 )
			return GetMotorCommand( g_throttle, -100.0 );
	#endif

	return GetMotorCommand(0.0,0.0);
}

void TransitionREVERSEstate(InputFlags input_flags, DriveState * state)
{
	if ( input_flags.switch_pos_drive && input_flags.mech_brake_pressed && input_flags.velocity_under_threshold )
		*state = DRIVE;
	else if ( input_flags.switch_pos_park && input_flags.mech_brake_pressed && input_flags.velocity_under_threshold )
		*state = PARK;
}

MotorCommand DoStatePARK()
{
	return GetMotorCommand(1.0, 0.0);
}

void TransitionPARKstate(InputFlags input_flags, DriveState * state)
{
	if ( input_flags.switch_pos_drive && input_flags.mech_brake_pressed && input_flags.velocity_under_threshold )
		*state = DRIVE;
	else if ( input_flags.switch_pos_reverse && input_flags.mech_brake_pressed && input_flags.velocity_under_threshold )
		*state = REVERSE;
}

MotorCommand DoStateCRUISE()
{
	return GetMotorCommand(CRUISE_THROTTLE, cruiseVelocity);
}

void TransitionCRUISEstate(InputFlags input_flags, DriveState * state)
{
	if ( input_flags.mech_brake_pressed || !input_flags.cruise_enabled )
		*state = DRIVE;
}

/*
 *  Updates input flags
 */
void UpdateInputFlags(InputFlags * input_flags)
{
	input_flags->mech_brake_pressed = HAL_GPIO_ReadPin(BRK_IN_GPIO_Port, BRK_IN_Pin);
	input_flags->regen_enabled = !HAL_GPIO_ReadPin(REGEN_EN_GPIO_Port, REGEN_EN_Pin);
	input_flags->velocity_under_threshold = velocityOfCar < VELOCITY_THRESHOLD;
	GetSwitchState(input_flags);
}


/*
 *  Parses CAN message for battery state of charge
 */
void ParseCANBatterySOC(uint8_t * CANMessageData)
{
	uint8_t batterySOC = CANMessageData[0];
	input_flags.battery_SOC_under_threshold = batterySOC < BATTERY_SOC_THRESHOLD;
}

/*
 *  Parses CAN message for velocity
 */
void ParseCANVelocity(uint8_t * CANMessageData)
{
	FloatBytes velocity;
	for(int i = 0; i < (sizeof(float)/sizeof(uint8_t)); i++)
	{
		velocity.bytes[i] = CANMessageData[i + 4]; // Vehicle Velocity is stored in bits 32-63.
	}
	velocityOfCar = velocity.float_value;
}

/*
 *  Parses CAN message for battery temp
 */
void ParseCANBatteryTemp(uint8_t * CANMessageData)
{
	input_flags.battery_temp_under_threshold = isBitSetFromArray(CANMessageData, 17); // regen_disable bit is stored in bit 17
}

/*
 *  Returns a MotorCommand struct based on the input throttle and velocity arguments
 */
MotorCommand GetMotorCommand(float throttle, float velocity)
{
	MotorCommand motorCommand;
	motorCommand.throttle = throttle;
	motorCommand.velocity = velocity;
	return motorCommand;
}

/*
 *  RTOS task for receiving CAN messages
 */
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
		osDelay(DELAY_GET_CAN_MESSAGES);
	}
	/* USER CODE END getBatterySOC */
}




/*
 *   Takes current value and velocity float value and sends in via CAN as an array of bytes.
 */
void SendCANMotorCommand(MotorCommand motorCommand)
{
	union FloatBytes throttle;
	union FloatBytes velocity;

	throttle.float_value = motorCommand.throttle;
	velocity.float_value = motorCommand.velocity;

	uint8_t data_send[CAN_DATA_LENGTH];
	for (int i = 0; i < (uint8_t) CAN_DATA_LENGTH / 2; i++)
	{
		data_send[i] = velocity.bytes[i];
	    data_send[4 + i] = throttle.bytes[i];
	}
	HAL_CAN_AddTxMessage(&hcan, &drive_command_header, data_send, &can_mailbox);
}

/*
 *  Function used for normalizing(0-1) and accounting for deadzone of ADC inputs.
 */
float normalize_adc_value(uint16_t value)
{
	if(value > ADC_MAX || value < ADC_MIN)
	{
		input_flags.throttle_ADC_out_of_range = true;
		return 0.0;
	}
	float normalized_value = (float)((float)(value - ADC_MIN) / (float)(ADC_MAX - ADC_MIN));
	input_flags.throttle_ADC_out_of_range = false;

	return normalized_value;
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
 */
void SendCANDIDDriveState()
{
	uint8_t data_send[CAN_DATA_LENGTH] = {0};
	data_send[0] = state;
	HAL_CAN_AddTxMessage(&hcan, &drive_state_header, data_send, &can_mailbox);
}

void GetSwitchState(InputFlags * input_flags)
{
	if( !HAL_GPIO_ReadPin(SWITCH_IN1_GPIO_Port, SWITCH_IN1_Pin) && !HAL_GPIO_ReadPin(SWITCH_IN2_GPIO_Port, SWITCH_IN2_Pin) )
	{
		input_flags->switch_pos_drive = false;
		input_flags->switch_pos_reverse = true;
		input_flags->switch_pos_park = false;
	}
	else if( HAL_GPIO_ReadPin(SWITCH_IN1_GPIO_Port, SWITCH_IN1_Pin) && HAL_GPIO_ReadPin(SWITCH_IN2_GPIO_Port, SWITCH_IN2_Pin) )
	{
		input_flags->switch_pos_drive = true;
		input_flags->switch_pos_reverse = false;
		input_flags->switch_pos_park = false;
	}
	else
	{
		input_flags->switch_pos_drive = false;
		input_flags->switch_pos_reverse = false;
		input_flags->switch_pos_park = true;
	}
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


