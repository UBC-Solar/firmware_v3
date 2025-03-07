/*
 * fault_lights.c
 *
 *	@brief   Contains functions to handle fault lights
 *
 *  Created on: March 01, 2025
 *      Author: Mridul Singh
 */

/*	Includes	*/
#include "stdint.h"
#include "fault_lights.h"
#include "gpio.h"
#include "can.h"


#define GETBIT(var, bit)	(((var) >> (bit)) & 1) // gives bit position

void Set_fault_lights(uint32_t CAN_ID, uint8_t* data){

	uint8_t pack_current_sign;

	switch(CAN_ID){
		case CAN_ID_PACK_CURRENT:
			estop(data);
			pack_current_sign = GETBIT(data[1],7); //reading the 2's compliment pack current's MSB to get the sign
			break;

		case CAN_ID_MTR_FAULTS:

			break;

		case CAN_ID_BATT_FAULTS:
			parse_batt_faults(data, pack_current_sign);

			break;

		case CAN_ID_PACK_VOLTAGE:

			break;

		default:
			break;
		}

    return;
}

void estop(uint8_t* can_rx_data){
	uint8_t temp_byte, estop;

	/* Byte 5 reading*/
	temp_byte = can_rx_data[5];
	estop = GETBIT(temp_byte, 5);
	HAL_GPIO_WritePin(ESTOP_GPIO_Port, ESTOP_Pin, estop);    								// Estop pressed
	return;
}

void parse_batt_faults(uint8_t* can_rx_data, uint8_t current_sign){

	uint8_t temp_byte, slave_board_comm_fault, overvolt_fault, self_test_fault,
				overtemp_fault, undervolt_fault, discharge_or_charge_overcurr_fault,
				charge_overcurrent_fault, discharge_overcurrent_fault;

	/* Byte 0 readings */
	temp_byte = can_rx_data[0]; // Contains bits 0-7, 7 6 5 4 3 2 1 0

	slave_board_comm_fault = GETBIT(temp_byte, 0);
	self_test_fault = GETBIT(temp_byte, 1);
	overtemp_fault = GETBIT(temp_byte, 2);
	undervolt_fault = GETBIT(temp_byte, 3);
	overvolt_fault = GETBIT(temp_byte, 4);
	discharge_or_charge_overcurr_fault = GETBIT(temp_byte, 6);
	charge_overcurrent_fault = (discharge_or_charge_overcurr_fault && current_sign);  	 	// Charging overcurrent fault if pack current is negative(MSB = 1)
	discharge_overcurrent_fault = (discharge_or_charge_overcurr_fault && (!current_sign));  // Discharging overcurrent fault if pack current is positive (MSB = 0)

	HAL_GPIO_WritePin(BMS_COMM_FLT_GPIO_Port, BMS_COMM_FLT_Pin, slave_board_comm_fault); 	// BMS communications fault
	HAL_GPIO_WritePin(BATT_OV_GPIO_Port, BATT_OV_Pin, overvolt_fault); 					 	// Battery over voltage fault
	HAL_GPIO_WritePin(BATT_FLT_GPIO_Port, BATT_FLT_Pin, self_test_fault); 				 	// Battery self-test fault
	HAL_GPIO_WritePin(BATT_OT_GPIO_Port, BATT_OT_Pin, overtemp_fault); 					 	// Battery over temperature fault
	HAL_GPIO_WritePin(BATT_UV_GPIO_Port, BATT_UV_Pin, undervolt_fault); 				 	// Battery under voltage fault
	HAL_GPIO_WritePin(CH_OC_GPIO_Port, CH_OC_Pin, charge_overcurrent_fault); 			 	// Charge Over-current fault
	HAL_GPIO_WritePin(DCH_OC_GPIO_Port, DCH_OC_Pin, discharge_overcurrent_fault); 	     	// Discharge Over-current fault

	return;
}
