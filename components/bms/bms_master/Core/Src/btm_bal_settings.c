/**
 *  @file btm_bal_settings.c
 *  @brief Module balancing algorithm
 *
 *  @date 2020/08/18
 *  @author abooodeee
 */

#include "btm_bal_settings.h"

// Private function prototypes
void min_val(uint16_t* val, int* loc, uint16_t arr[], int size, int enable[]);

/** @brief skeleton for a function for controlling the flow of the balancing operation
 *
 * @param[in] pack The main pack data structure of the program that contains the voltage
 * of each module for each stack
 * @param[out] dch_setting_pack The discharge settings for the entire pack.
*/
void BTM_BAL_settings(
	BTM_PackData_t* pack,
	BTM_BAL_dch_setting_pack_t* dch_setting_pack)
{
	const int size = 36; // size of the pack_modules array (36 modules)

	// PROBLEM (fixed): Vth and Vtol had 0x prefixes before - that's for hex not binary. 0b for binary.
	uint16_t Vth = 40000; //threshold voltage to start the balancing at (multiplied by 10^4 to match the format in the pack)
	uint16_t Vtol = 500; //tolerance voltage through which cells are counted as balanced
	uint16_t pack_modules[size]; // Array to store all cell voltages
	uint16_t Vmin;		// minimum voltage in the pack
	int Vmin_loc;		// location of the minimum voltage in the array
	int pack_modules_en[size]; // array to store modules' enable flags consistent with the order of indices in pack_modules array
	
	// Storing all cell voltages from the 2 stacks:
	for(int module_num = 0; module_num < 18; module_num++) 
	{
		pack_modules[module_num] = pack->stack[0].module[module_num].voltage;
		pack_modules_en[module_num] = pack->stack[0].module[module_num].enable;
	}
	
	for(int module_num = 18; module_num < 36; module_num++)
	{
		pack_modules[module_num] = pack->stack[1].module[module_num-18].voltage;
		pack_modules_en[module_num] = pack->stack[1].module[module_num-18].enable;
	}
	
	
	min_val(&Vmin, &Vmin_loc, pack_modules, size, pack_modules_en); // This function is written at the end of this file
	
	
	// discharge all modules that are above the threshold voltage if any cell is below the threshold
	if (Vmin < Vth) { 
		
		for(int i = 0; i < size; i++) {
			if(pack_modules_en[i]) {
				if(pack_modules[i] > Vth) {
					// turn on the corresponding s pins
					if (i < 18)
						dch_setting_pack->stack[0].module_dch[i] = DISCHARGE_ON;
					else if (i < 36)
						dch_setting_pack->stack[1].module_dch[i-18] = DISCHARGE_ON; 
				}
				
				else {
					if (i < 18)
						dch_setting_pack->stack[0].module_dch[i] = DISCHARGE_OFF;
					else if (i < 36)
						dch_setting_pack->stack[1].module_dch[i-18] = DISCHARGE_OFF;
				}
			}
		}
		// Call BTM_BAL_setDischarge to send the settings to the configuration register
		BTM_BAL_setDischarge(pack, dch_setting_pack);
	}
	
	// Otherwise, balance to the lowest cell voltage
	else {
		
		for(int i = 0; i < size; i++) {
			if(pack_modules_en[i]) {
				if(pack_modules[i] > Vmin + Vtol) {
					// turn on the corresponding s 
					if (i < 18)
						dch_setting_pack->stack[0].module_dch[i] = DISCHARGE_ON;
					else if (i < 36)
						dch_setting_pack->stack[1].module_dch[i-18] = DISCHARGE_ON; 
				}
				
				else {
					if (i < 18)
						dch_setting_pack->stack[0].module_dch[i] = DISCHARGE_OFF;
					else if (i < 36)
						dch_setting_pack->stack[1].module_dch[i-18] = DISCHARGE_OFF;
				}
			}
		}
		// Call BTM_BAL_setDischarge to send the settings to the configuration register
		BTM_BAL_setDischarge(pack, dch_setting_pack);
	}

}

/**
 * @brief a function that outputs the minimum value in an array and its location
 *
 * @param[in/out] val, pointer that gets the address and outputs the minimum value
 * @param[in/out] loc, pointer that gets the address and outputs the location of the minimum value
 * @param[in] arr, the array we want to extract the minimum value from
 * @param[in] size, the size of the array
 * @param[in] pack_modules_en, enable flag to check which modules are enabled
*/	
void min_val(uint16_t* val, int* loc, uint16_t arr[], int size, int enable[]) {
	int temp_loc = 0;
	uint16_t temp_val = arr[0]; // the first element of the array is the first point of comparison and will change if another element is smaller
	for (int i = 1; i < size; i++) {
		if(arr[i] < temp_val) {
			if(enable[i]) // only enabled modules are considered (since we're using 32 of the 36 modules)
			{ 
				temp_val = arr[i];
				temp_loc = i;
			}
		}
	}
	*val = temp_val;
	*loc = temp_loc;
	return;
}
