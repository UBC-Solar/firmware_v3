/* @brief: skeleton for a function for controlling the flow of the balancing operation
 *
 * @param[in] pack The main pack data structure of the program to write
 *  balancing status flags to and read module enable flags from
 *
 * @param[out] dch_setting_pack The discharge settings for the entire pack.
*/
// remember to add function prototypes in the header file
void BTM_BAL_settings(
	BTM_PackData_t* pack,
	BTM_BAL_dch_setting_pack_t* dch_setting_pack)
{
	const int size = 35; // size of the pack_modules array
	float Vth = 4.0;   // threshold voltage to start the balancing at
	float Vtol = 0.05; // tolerance voltage through which cells are counted as balanced
	float pack_modules[size]; // Array to store all cell voltages
	float Vmin;			// minimum voltage in the pack
	int Vmin_loc;		// location of the minimum voltage in the array
	int pack_modules_en[size]; // array to store modules' enable flags consistant with pack_modules array
	
	// Storing all cell voltages from the 3 stacks:
	for(int module_num = 0; module_num < 12; module_num++) // Uncertainty: Are we sure that the last 2 modules of the first 2 stacks are discarded? 
	{
		if(pack->stack[0].module[module_num].enable = 1)
		pack_modules[module_num] = pack->stack[0].module[module_num].voltage;
		pack_modules_en[module_num] = pack->stack[0].module[module_num].enable;
	}
	
	for(int module_num = 12; module_num < 24; module_num++)
	{
		pack_modules[module_num] = pack->stack[1].module[module_num-12].voltage;
		pack_modules_en[module_num] = pack->stack[1].module[module_num-12].enable;
	}
	
	for(int module_num = 24; module_num < 36; module_num++)
	{
		pack_modules[module_num] = pack->stack[2].module[module_num-24].voltage;
		pack_modules_en[module_num] = pack->stack[2].module[module_num-24].enable;
	}
	
	
	
	/*for(int stack_num = 0; stack_num < BTM_NUM_DEVICES; stack_num++)
    {
        for(int module_num = 0; module_num < BTM_NUM_MODULES; module_num++)
        {
            if (dch_pack_target->stack[stack_num].module_dch[module_num] < Vth) { // need to discard the last 2 modules of the first 2 slave boards
				charg_mod = 1;
				stack_num = BTM_NUM_DEVICES;
				module_num = BTM_NUM_MODULES;
				break;
			}
        }
    }*/
	
	min_val(&Vmin, &Vmin_loc, pack_modules, size, pack_modules_en);
	
	
	// discharge all modules that are above the threshold voltage if any cell is below the threshold
	if (Vmin < Vth) { 
		
		for(int i = 0; i < size; i++) {
			if(pack_modules[i] > Vth) {
				// turn on the corresponding s pins (while making sure the others are turned off
				// write the settings pack structure (last 2 bytes of the configuration register)
			}
			// Call BTM_BAL_setDischarge to send the settings to the configuration register
		}
		
	}
	
	// Otherwise, balance to the lowest cell voltage
	else {
		
		for(int i = 0; i < size; i++) {
			if(pack_modules[i] > Vmin + Vtol) {
				// turn on the corresponding s pin
				// write the settings pack structure (last 2 bytes of the configuration register)
			}
			// Call BTM_BAL_setDischarge to send the settings to the configuration register
		}
	}

}

/*@brief: a function that outputs the minimum value in an array and its location
 *
 *@param[in/out] val, pointer that gets the address and outputs the minimum value
 *@param[in/out] val, pointer that gets the address and outputs the location of the minimum value
 *@param[in] arr, the array we want to extract the minimum value from
 *@param[in] size, the size of the array
*/	
void min_val(float* val, int* loc, float arr[], size, enable[]) {
	int temp_loc = 0;
	float temp_val = arr[0];
		
	for (int i = 1; i < size; i++) {
		if(arr[i] < temp) {
			if(enable[i]) {
				temp_val = arr[i];
				temp_loc = i;
			}
		}
	}
	*val = temp_val;
	*loc = temp_loc;
	return;
}