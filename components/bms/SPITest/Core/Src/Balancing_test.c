#include <stdio.h>

void min_val(int* val, int* loc, int arr[], int size, int enable[]);
void main() {
	const int size = 36; // size of the pack_modules array (36 modules)
	int Vth = 40000; //4.0*10^4 threshold voltage to start the balancing at (multiplied by 10^4 to match the format in the pack)
	int Vtol = 500; //0.05*10^4 tolerance voltage through which cells are counted as balanced
	int pack_modules[] = { 40750,41000,40250,40100,40500,0,0,40600,40650,41500,41500,41000,
	                       40500,41500,42000,42500,41000,0,0,42000,41600,41750,41000,40300,
	                       40100,40500,40550,40400,40150,40300,41750,41555,40950,40800,40700,40600 };
	int pack_modules_en[] = { 1,1,1,1,1,0,0,1,1,1,1,1,
	                          1,1,1,1,1,0,0,1,1,1,1,1,
                                  1,1,1,1,1,1,1,1,1,1,1,1 };

	int Vmin;		// minimum voltage in the pack
	int Vmin_loc;		// location of the minimum voltage in the array
	// Storing all cell voltages from the 3 stacks:

	/*for (int module_num = 0; module_num < 12; module_num++)
	{
		pack_modules[module_num] = pack->stack[0].module[module_num].voltage;
		pack_modules_en[module_num] = pack->stack[0].module[module_num].enable;
	}

	for (int module_num = 12; module_num < 24; module_num++)
	{
		pack_modules[module_num] = pack->stack[1].module[module_num - 12].voltage;
		pack_modules_en[module_num] = pack->stack[1].module[module_num - 12].enable;
	}

	for (int module_num = 24; module_num < 36; module_num++)
	{
		pack_modules[module_num] = pack->stack[2].module[module_num - 24].voltage;
		pack_modules_en[module_num] = pack->stack[2].module[module_num - 24].enable;
	}
	*/

	min_val(&Vmin, &Vmin_loc, pack_modules, size, pack_modules_en); // This function is written at the end of this file
	printf("Vmin is %d, index %d\n", Vmin, Vmin_loc);

	// discharge all modules that are above the threshold voltage if any cell is below the threshold
	if (Vmin < Vth) {

		for (int i = 0; i < size; i++) {
			if (pack_modules_en[i]) {
				if (pack_modules[i] > Vth) {
					// turn on the corresponding s pins if the module is enabled

					if (i < 12)
						printf("module %d is discharging in the first stack (V = %d)\n", i + 1, pack_modules[i]);
					else if (i < 24)
						printf("module %d is discharging in the second stack (V = %d)\n", i + 1 - 12, pack_modules[i]);
					else if (i < 36)
						printf("module %d is discharging in the third stack (V = %d)\n", i + 1 - 24, pack_modules[i]);
				}

				else {
					if (i < 12)
						printf("(charging) module %d is charging in the first stack (V = %d)\n", i + 1, pack_modules[i]);
					else if (i < 24)
						printf("(charging) module %d is charging in the second stack (V = %d)\n", i + 1 - 12, pack_modules[i]);
					else if (i < 36)
						printf("(charging) module %d is charging in the third stack (V = %d)\n", i + 1 - 24, pack_modules[i]);
				}
			}
		}
		// Call BTM_BAL_setDischarge to send the settings to the configuration register
		printf("calling BTM_BAL_setDischarge\n");
	}

	// Otherwise, balance to the lowest cell voltage
	else {

		for (int i = 0; i < size; i++) {
			if (pack_modules_en[i]) {
				if (pack_modules[i] > Vmin + Vtol) {
					// turn on the corresponding s pins if the module is enabled

					if (i < 12)
						printf("module %d is discharging in the first stack (V = %d)\n", i + 1, pack_modules[i]);
					else if (i < 24)
						printf("module %d is discharging in the second stack (V = %d)\n", i + 1 - 12, pack_modules[i]);
					else if (i < 36)
						printf("module %d is discharging in the third stack (V = %d)\n", i + 1 - 24, pack_modules[i]);
				}

				else {
					if (i < 12)
						printf("(charging) module %d is charging in the first stack (V = %d)\n", i + 1, pack_modules[i]);
					else if (i < 24)
						printf("(charging) module %d is charging in the second stack (V = %d)\n", i + 1 - 12, pack_modules[i]);
					else if (i < 36)
						printf("(charging) module %d is charging in the third stack (V = %d)\n", i + 1 - 24, pack_modules[i]);
				}	
			}	
		}
		// Call BTM_BAL_setDischarge to send the settings to the configuration register
		printf("calling BTM_BAL_setDischarge\n");
	}
	return;
}

void min_val(int* val, int* loc, int arr[], int size, int enable[]) {
	int temp_loc = 0;
	int temp_val = arr[0]; // the first element of the array is the first point of comparison 
				 // and will change if another element is smaller
	for (int i = 1; i < size; i++) {
		if (arr[i] < temp_val) {
			if (enable[i]) // only enabled modules are considered (since we're using 32 of the 36 modules)
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
