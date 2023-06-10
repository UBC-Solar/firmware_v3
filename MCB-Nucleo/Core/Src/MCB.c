/*
 * MCB.c
 *
 *  Created on: Jun. 1, 2023
 *      Author: Kyle Groulx
 */

#include <mcb.h>

struct InputFlags event_flags; // Event flags for deciding what state to be in.
enum DriveState state;
float cruise_velocity; // Velocity for cruise control
float velocity_of_car; // Current velocity of the car will be stored here.



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
	//Todo Check BOM for this CAN message
	uint8_t data_send[CAN_DATA_LENGTH];
	HAL_CAN_AddTxMessage(&hcan, &drive_command_header, data_send, &can_mailbox);
}



