/*
 *  Includes
 */
#include "gpio.h"
#include "DID.h"
#include "LCD.h"
#include "math.h"

#define GETBIT(var, bit)	(((var) >> (bit)) & 1) // gives bit position 

/*
 *  Variables
 */
uint8_t current_page = 0;
uint8_t recent_warnings[4]; // [LV Warn, HV Warn, LT Warn, HT Warn]
uint8_t recent_faults[15];  // See BS Master BOM (0x622 Bits 0-12, 17, 18)

/*
 * Initialize cyclic data
 */
CREATE_CYCLIC_DATA_FLOAT( data_simulation_speed, MAX_CYCLE_TIME_SIMULATION_SPEED );
CREATE_CYCLIC_DATA_U8( data_battery_SOC, MAX_CYCLE_TIME_BATTERY_SOC );
CREATE_CYCLIC_DATA_FLOAT( data_target_velocity, MAX_CYCLE_TIME_MOTOR_COMMAND );
CREATE_CYCLIC_DATA_FLOAT( data_vehicle_velocity, MAX_CYCLE_TIME_VEHICLE_VELOCITY );
CREATE_CYCLIC_DATA_U8( data_MCB_drive_state, MAX_CYCLE_TIME_MCB_DRIVE_STATE );
CREATE_CYCLIC_DATA_U8( data_MCB_regen_enabled, MAX_CYCLE_TIME_MCB_REGEN_ENABLED );
CREATE_CYCLIC_DATA_FLOAT( data_motor_current, MAX_CYCLE_TIME_MOTOR_CURRENT );
CREATE_CYCLIC_DATA_FLOAT( data_array_current, MAX_CYCLE_TIME_ARRAY_CURRENT );
CREATE_CYCLIC_DATA_FLOAT( data_pack_current, MAX_CYCLE_TIME_PACK_CURRENT );
CREATE_CYCLIC_DATA_U16( data_pack_voltage, MAX_CYCLE_TIME_PACK_VOLTAGE );
CREATE_CYCLIC_DATA_S8( data_pack_temperature, MAX_CYCLE_TIME_PACK_TEMPERATURE );

/*
 *  Function declarations
 */


void parse_can_message( uint8_t* CAN_rx_data, uint32_t CAN_ID )
{
	switch(CAN_ID)
	{
		/*
		 *	DID next page
		 */
//		case CAN_ID_DID_NEXT_PAGE:
//			if ( CAN_rx_data[0] != 0 )
//			{
//				current_page = current_page + 1; 	// Increment page
//				if (current_page == NUM_PAGES) 		// Reset to 0 if changing from last page
//					current_page = 0;
//			}
//			break;

		/* Parse Warnings and Faults if received CAN message is 0x622
		 * and set GPIO output for fault lights accordingly
		 */
		case CAN_ID_FAULTS:
			parse_warnings(CAN_rx_data);
			break;

		// Simulation target speed
		case CAN_ID_SIMULATION_SPEED:
			FloatBytes temp_simulation_speed;
			temp_simulation_speed.bytes[0] = CAN_rx_data[0];
			temp_simulation_speed.bytes[1] = CAN_rx_data[1];
			temp_simulation_speed.bytes[2] = CAN_rx_data[2];
			temp_simulation_speed.bytes[3] = CAN_rx_data[3];

			temp_simulation_speed.float_value = round(MS_TO_MPH(temp_simulation_speed.float_value));

			SET_CYCLIC_DATA( data_simulation_speed, temp_simulation_speed.float_value );
			break;

		/*
		 *	Battery state of charge 
		 */
		case CAN_ID_BATTERY_SOC:

			SET_CYCLIC_DATA( data_battery_SOC, CAN_rx_data[0] );
			break;
		
		/*
		 *	Vehicle target velocity
		 */
		case CAD_ID_MOTOR_COMMAND:
			FloatBytes temp_target_velocity;
			temp_target_velocity.bytes[0] = CAN_rx_data[0];
			temp_target_velocity.bytes[1] = CAN_rx_data[1];
			temp_target_velocity.bytes[2] = CAN_rx_data[2];
			temp_target_velocity.bytes[3] = CAN_rx_data[3];

			temp_target_velocity.float_value = round(MS_TO_MPH(temp_target_velocity.float_value));

			SET_CYCLIC_DATA( data_target_velocity, temp_target_velocity.float_value );
			break;

		/*
		 * Vehicle velocity
		 */
		case CAN_ID_VEHICLE_VELOCITY:
			FloatBytes temp_vehicle_velocity;
			temp_vehicle_velocity.bytes[0] = CAN_rx_data[4];
			temp_vehicle_velocity.bytes[1] = CAN_rx_data[5];
			temp_vehicle_velocity.bytes[2] = CAN_rx_data[6];
			temp_vehicle_velocity.bytes[3] = CAN_rx_data[7];

			temp_vehicle_velocity.float_value = round(MS_TO_MPH(temp_vehicle_velocity.float_value));

			SET_CYCLIC_DATA( data_vehicle_velocity, temp_vehicle_velocity.float_value );
			break;

		/*
		 *	MCB Drive State
		 */
		case CAN_ID_MCB_DRIVE_STATE:
			SET_CYCLIC_DATA( data_MCB_drive_state, CAN_rx_data[0] );
			break;

		/*
		 *  MCB Diagnostics
		 */
		case CAN_ID_MCB_REGEN_ENABLED:
			uint8_t regen_enabled = GETBIT(CAN_rx_data[2], 1);
			SET_CYCLIC_DATA( data_MCB_regen_enabled, regen_enabled);
			break;

		/*
		 *  Motor Current
		 */
		case CAN_ID_MOTOR_CURRENT:
			FloatBytes temp_motor_current;
			temp_motor_current.bytes[0] = CAN_rx_data[0];
			temp_motor_current.bytes[1] = CAN_rx_data[1];
			temp_motor_current.bytes[2] = CAN_rx_data[2];
			temp_motor_current.bytes[3] = CAN_rx_data[3];

			SET_CYCLIC_DATA( data_motor_current, temp_motor_current.float_value );
			break;
		
		/*
		 *	Array Current
		 */
		case CAN_ID_ARRAY_CURRENT:
			FloatBytes temp_array_current;
			temp_array_current.bytes[0] = CAN_rx_data[4];
			temp_array_current.bytes[1] = CAN_rx_data[5];
			temp_array_current.bytes[2] = CAN_rx_data[6];
			temp_array_current.bytes[3] = CAN_rx_data[7];

			SET_CYCLIC_DATA( data_array_current, temp_array_current.float_value );
			break;

		/*
		 *	Pack Current
		 */
		case CAN_ID_PACK_CURRENT:
			U16Bytes temp_pack_current;
			FloatBytes pack_current_float;
			temp_pack_current.bytes[0] = CAN_rx_data[0];
			temp_pack_current.bytes[1] = CAN_rx_data[1];
			pack_current_float.float_value = temp_pack_current.U16_value / PACK_CURRENT_DIVISOR;
	
			SET_CYCLIC_DATA( data_pack_current, pack_current_float.float_value );
			break;

		/*
		 *	Pack Voltage
		 */
		case CAN_ID_PACK_VOLTAGE:
			U16Bytes temp_pack_voltage;
			temp_pack_voltage.bytes[0] = CAN_rx_data[0];
			temp_pack_voltage.bytes[1] = CAN_rx_data[1];
			temp_pack_voltage.U16_value = temp_pack_voltage.U16_value / PACK_VOLTAGE_DIVISOR;

			SET_CYCLIC_DATA( data_pack_voltage, temp_pack_voltage.U16_value );
			break;

		/*
		 *	Pack Temperature
		 */
		case CAN_ID_PACK_TEMPERATURE:
			S8Byte temp_pack_temperature;
			temp_pack_temperature.byte = CAN_rx_data[3];

			SET_CYCLIC_DATA( data_pack_temperature, temp_pack_temperature.S8_value );
			break;

		
	}
}



void update_DID_screen()
{
	static uint32_t last_update_time = 0;
	if( last_update_time + DID_REFRESH_DELAY > HAL_GetTick() )
		return;
	last_update_time = HAL_GetTick();

	ClearScreen();
	switch(current_page)
	{
		case PAGE_0:
			UpdateScreenTitles(PAGE_0);

			// Get pointers to cyclic data variables
			float* vehicle_velocity = GET_CYCLIC_DATA( data_vehicle_velocity );
			uint16_t* pack_voltage = GET_CYCLIC_DATA( data_pack_voltage );
//			uint8_t* battery_soc = GET_CYCLIC_DATA( data_battery_SOC );
			uint8_t* MCB_drive_state = GET_CYCLIC_DATA( data_MCB_drive_state );
			uint8_t* MCB_regen_enabled = GET_CYCLIC_DATA (data_MCB_regen_enabled);
//			float* simulation_speed = GET_CYCLIC_DATA( data_simulation_speed );
			float* pack_current = GET_CYCLIC_DATA( data_pack_current );

			// Vehicle Velocity
			if ( vehicle_velocity != NULL )
			{
				UpdateScreenParameter(SPEED_DATA_XPOS, SPEED_DATA_YPOS, (int32_t)(*vehicle_velocity), 0, FALSE);
			}
			else
			{
				OutputString("---", SPEED_DATA_XPOS, SPEED_DATA_YPOS);
			}

			// MCB drive state
			if ( MCB_drive_state != NULL )
			{
				if ( *MCB_drive_state == 0x01 ) 		
					OutputString("DRV", STATE_DATA_XPOS, STATE_DATA_YPOS); // DRIVE
				else if (*MCB_drive_state == 0x02)   
					OutputString("CRS", STATE_DATA_XPOS, STATE_DATA_YPOS); // CRUISE
				else if (*MCB_drive_state == 0x03)   
					OutputString("PRK", STATE_DATA_XPOS, STATE_DATA_YPOS); // PARK
				else if (*MCB_drive_state == 0x04)  
					OutputString("REV", STATE_DATA_XPOS, STATE_DATA_YPOS); // REVERSE
				else   							
					OutputString("ERR", STATE_DATA_XPOS, STATE_DATA_YPOS); // INVALID (MCB should never send this)
			}	
			else
			{
				OutputString("---", STATE_DATA_XPOS, STATE_DATA_YPOS);
			}

			// Regen Enable
			if ( MCB_regen_enabled != NULL )
			{
				if (*MCB_regen_enabled == ENABLED) {
					OutputString("   ", REGEN_DATA_XPOS, REGEN_DATA_YPOS);
					OutputString("ON", REGEN_DATA_XPOS, REGEN_DATA_YPOS);
				} else {
					OutputString("   ", REGEN_DATA_XPOS, REGEN_DATA_YPOS);
					OutputString("OFF", REGEN_DATA_XPOS, REGEN_DATA_YPOS);
				}
			}
			else
			{
				OutputString("---", REGEN_DATA_XPOS, REGEN_DATA_YPOS);
			}

			// Pack Voltage
			if( pack_voltage != NULL )
			{
				UpdateScreenParameter(PACK_VOLT_DATA_XPOS, PACK_VOLT_DATA_YPOS, (uint32_t)(*pack_voltage), 0, FALSE);
			}
			else
			{
				OutputString("---", PACK_VOLT_DATA_XPOS, PACK_VOLT_DATA_YPOS);
			}

			// Pack Current
			if ( pack_current != NULL )
			{
				// % 10 to get the decimal place.
				UpdateScreenParameter(PACK_CURRENT_DATA_XPOS, PACK_CURRENT_DATA_YPOS, (uint32_t)(*pack_current), ((uint8_t)((*pack_current) * 10) % 10), TRUE);
			}
			else
			{
				// Using 4 dashes as the floating point requires more space.
				OutputString("----", PACK_CURRENT_DATA_XPOS, PACK_CURRENT_DATA_YPOS);
			}

			break;

		case PAGE_1:
			UpdateScreenTitles(PAGE_1);	// TODO: Cyclic data for warnings?
			if (recent_warnings[0]) { // Low Voltage Warning
				OutputString("     ", LV_WARN_DATA_XPOS, LV_WARN_DATA_YPOS); // Clear
				OutputString("YES", LV_WARN_DATA_XPOS, LV_WARN_DATA_YPOS); // Write "YES"
			} else {
				OutputString("     ", LV_WARN_DATA_XPOS, LV_WARN_DATA_YPOS); // Clear
				OutputString("---", LV_WARN_DATA_XPOS, LV_WARN_DATA_YPOS); // Write "---"
			}
			if (recent_warnings[1]) { // High Voltage Warning
				OutputString("     ", HV_WARN_DATA_XPOS, HV_WARN_DATA_YPOS); // Clear
				OutputString("YES", HV_WARN_DATA_XPOS, HV_WARN_DATA_YPOS); // Write "YES"
			} else {
				OutputString("     ", HV_WARN_DATA_XPOS, HV_WARN_DATA_YPOS); // Clear
				OutputString("---", HV_WARN_DATA_XPOS, HV_WARN_DATA_YPOS); // Write "---"
			}
			if (recent_warnings[2]) { // Low Temperature Warning
				OutputString("     ", LT_WARN_DATA_XPOS, LT_WARN_DATA_YPOS); // Clear
				OutputString("YES", LT_WARN_DATA_XPOS, LT_WARN_DATA_YPOS); // Write "YES"
			} else {
				OutputString("     ", LT_WARN_DATA_XPOS, LT_WARN_DATA_YPOS); // Clear
				OutputString("---", LT_WARN_DATA_XPOS, LT_WARN_DATA_YPOS); // Write "---"
			}
			if (recent_warnings[3]) { // High Temperature Warning
				OutputString("     ", HT_WARN_DATA_XPOS, HT_WARN_DATA_YPOS); // Clear
				OutputString("YES", HT_WARN_DATA_XPOS, HT_WARN_DATA_YPOS); // Write "YES"
			} else {
				OutputString("     ", HT_WARN_DATA_XPOS, HT_WARN_DATA_YPOS); // Clear
				OutputString("---", HT_WARN_DATA_XPOS, HT_WARN_DATA_YPOS); // Write "---"
			}
			break;

		case PAGE_2:
			UpdateScreenTitles(PAGE_2);
			// Get pointers to cyclic data variables
			float* motor_current = GET_CYCLIC_DATA( data_motor_current );
			float* array_current = GET_CYCLIC_DATA( data_array_current );
//			uint16_t* pack_current = GET_CYCLIC_DATA( data_pack_current );

			// Motor Current
			if( motor_current != NULL )
			{
				UpdateScreenParameter(MOTOR_CURRENT_DATA_XPOS, MOTOR_CURRENT_DATA_YPOS, (int32_t)(*motor_current), 0, FALSE);
			}
			else
			{
				OutputString("---", MOTOR_CURRENT_DATA_XPOS, MOTOR_CURRENT_DATA_YPOS);
			}

			// Array Current
			if ( array_current != NULL )
			{
				UpdateScreenParameter(ARRAY_CURRENT_DATA_XPOS, ARRAY_CURRENT_DATA_YPOS, (uint32_t)(*array_current), ((uint32_t) ((*array_current) * 10)) % 10, TRUE); // Float
			}
			else
			{
				OutputString("---", ARRAY_CURRENT_DATA_XPOS, ARRAY_CURRENT_DATA_YPOS);
			}

//			// Pack Current
//			if ( pack_current != NULL )
//			{
//				UpdateScreenParameter(PACK_CURRENT_DATA_XPOS, PACK_CURRENT_DATA_YPOS, (uint32_t)(*pack_current), 0, FALSE);
//			}
//			else
//			{
//				OutputString("---", PACK_CURRENT_DATA_XPOS, PACK_CURRENT_DATA_YPOS);
//			}
//			break;

			case PAGE_3:
				UpdateScreenTitles(PAGE_3);
				// Get pointers to cyclic data variables
//				uint16_t* pack_voltage = GET_CYCLIC_DATA( data_pack_voltage );
				int8_t* pack_temperature = GET_CYCLIC_DATA( data_pack_temperature ); 

				// Pack Voltage
//				if( pack_voltage != NULL )
//				{
//					UpdateScreenParameter(PACK_VOLT_DATA_XPOS, PACK_VOLT_DATA_YPOS, (uint32_t)(*pack_voltage), 0, FALSE);
//				}
//				else
//				{
//					OutputString("---", PACK_VOLT_DATA_XPOS, PACK_VOLT_DATA_YPOS);
//				}

				// Pack Temperature
				if( pack_temperature != NULL)
				{
					UpdateScreenParameter(PACK_TEMP_DATA_XPOS, PACK_TEMP_DATA_YPOS, (*pack_temperature), 0, FALSE); //displays the pack temperature
				}
				else
				{
					OutputString("---", PACK_TEMP_DATA_XPOS, PACK_TEMP_DATA_YPOS);
				}
				break;

		default:
			break;
	}
}

/**
 * Called when warning CAN message received
 * Updates recent_warnings array with latest warnings
 */
void parse_warnings(uint8_t* CAN_rx_data)
{
	/* Local Variables declarations */
	uint8_t temp_byte, slave_board_comm_fault, bms_self_test_fault, overtemp_fault,
			undervolt_fault, overvolt_fault, isolation_loss_fault, discharge_or_charge_overcurr_fault,
			volt_out_of_range_fault, temp_out_of_range_fault, pack_balancing_active, LLIM_active,
			HLIM_active, charge_overtemp_trip, request_regen_turn_off, no_ecu_curr_message_received_warn,
			low_voltage_warning, high_voltage_warning, low_temperature_warning, high_temperature_warning;

	/* Byte 0 readings */
	temp_byte = CAN_rx_data[0]; // Contains bits 0-7, 7 6 5 4 3 2 1 0
	slave_board_comm_fault = GETBIT(temp_byte, 0);
	bms_self_test_fault = GETBIT(temp_byte, 1);
	overtemp_fault = GETBIT(temp_byte, 2);
	undervolt_fault = GETBIT(temp_byte, 3);
	overvolt_fault = GETBIT(temp_byte, 4);
	isolation_loss_fault = GETBIT(temp_byte, 5);
	discharge_or_charge_overcurr_fault = GETBIT(temp_byte, 6);
	volt_out_of_range_fault = GETBIT(temp_byte, 7);

	/* Byte 1 readings */
	temp_byte = CAN_rx_data[1]; // Contains bits 8-15, 15 14 13 12 11 10 9 8
	temp_out_of_range_fault = GETBIT(temp_byte, 0);
	pack_balancing_active = GETBIT(temp_byte, 1);
	LLIM_active = GETBIT(temp_byte, 2);
	HLIM_active = GETBIT(temp_byte, 3);
	charge_overtemp_trip = GETBIT(temp_byte, 4);
	low_voltage_warning = GETBIT(temp_byte, 5);
	high_voltage_warning = GETBIT(temp_byte, 6);
	low_temperature_warning = GETBIT(temp_byte, 7);

	/* Byte 2 readings */
	temp_byte = CAN_rx_data[2]; // Contains bits 16-23, 23 22 21 20 19 18 17 16
	high_temperature_warning = GETBIT(temp_byte, 0);
	request_regen_turn_off = GETBIT(temp_byte, 1);
	no_ecu_curr_message_received_warn = GETBIT(temp_byte, 2);

	/* Update Warnings for Screen */
	recent_warnings[0] = low_voltage_warning;
	recent_warnings[1] = high_voltage_warning;
	recent_warnings[2] = low_temperature_warning;
	recent_warnings[3] = high_temperature_warning;

	/* Update Faults */
	recent_faults[0] = slave_board_comm_fault; // bms_comm_flt
	recent_faults[1] = bms_self_test_fault;
	recent_faults[2] = overtemp_fault;  // batt_ot
	recent_faults[3] = undervolt_fault; // batt_uv
	recent_faults[4] = overvolt_fault; // batt_ov
	recent_faults[5] = isolation_loss_fault;
	recent_faults[6] = discharge_or_charge_overcurr_fault; // dch_oc
	recent_faults[7] = volt_out_of_range_fault;
	recent_faults[8] = temp_out_of_range_fault;
	recent_faults[9] = pack_balancing_active;
	recent_faults[10] = LLIM_active;
	recent_faults[11] = HLIM_active;
	recent_faults[12] = charge_overtemp_trip; // ch_oc
	recent_faults[13] = request_regen_turn_off;
	recent_faults[14] = no_ecu_curr_message_received_warn;



	HAL_GPIO_WritePin(BMS_COMM_FLT_GPIO_Port, BMS_COMM_FLT_Pin, slave_board_comm_fault); // BMS communications fault
	HAL_GPIO_WritePin(BATT_OT_GPIO_Port, BATT_OT_Pin, overtemp_fault); 					 // Battery over temperature fault
	HAL_GPIO_WritePin(BATT_UV_GPIO_Port, BATT_UV_Pin, undervolt_fault); 				 // Battery under voltage fault
	HAL_GPIO_WritePin(BATT_OV_GPIO_Port, BATT_OV_Pin, overvolt_fault); 					 // Battery over voltage fault
	HAL_GPIO_WritePin(DCH_OC_GPIO_Port, DCH_OC_Pin, discharge_or_charge_overcurr_fault); // Discharging overcurrent fault
	HAL_GPIO_WritePin(CH_OC_GPIO_Port, CH_OC_Pin, charge_overtemp_trip); 				 // Charging overcurrent fault
	// HAL_GPIO_WritePin(BATT_OV_GPIO_Port, BATT_OV_Pin, high_voltage_warning); 	    // Battery voltage upper limit fault

}

void DID_timeout()
{
    static uint8_t lastPage;
    static uint32_t lastPageTime;

    // If DID is on a page more than DID_PAGE_TIMEOUT ms, set current page to PAGE_0
	if( current_page != lastPage )
	{
		lastPage = current_page;
		lastPageTime = HAL_GetTick();
	}
	else if((HAL_GetTick() - lastPageTime) > DID_PAGE_TIMEOUT)
	{
		current_page = PAGE_0;
        lastPageTime = HAL_GetTick();
	}
}
