#include <stdio.h>

void min_val(int* val, int* loc, int arr[], int size, int enable[]);
void main() {
	const int size = 36; // size of the pack_modules array (36 modules)
	int Vth = 40000; //4.0*10^4 threshold voltage to start the balancing at (multiplied by 10^4 to match the format in the pack)
	int Vtol = 500; //0.05*10^4 tolerance voltage through which cells are counted as balanced
	int pack_modules[] = { 39000,38000,36250,37000,40500,0,0,40600,40650,37600,41500,38500,
						   40500,36350,38650,37750,36800,0,0,39000,41600,39650,41000,40300,
						   40100,40500,40550,40400,40150,40300,41750,36500,40950,38500,40700,36550 };
	int pack_modules_en[] = { 1,1,1,1,1,0,0,1,1,1,1,1,
							  1,1,1,1,1,0,0,1,1,1,1,1,
							  1,1,1,1,1,1,1,1,1,1,1,1 };

	int Vmin;		// minimum voltage in the pack
	int Vmin_loc;		// location of the minimum voltage in the array

	while (1) {
		min_val(&Vmin, &Vmin_loc, pack_modules, size, pack_modules_en); // This function is written at the end of this file
		printf("Vmin is %d, index %d\n", Vmin, Vmin_loc + 1);

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
							printf("(NOT) module %d is not discharging in the first stack (V = %d)\n", i + 1, pack_modules[i]);
						else if (i < 24)
							printf("(NOT) module %d is not discharging in the second stack (V = %d)\n", i + 1 - 12, pack_modules[i]);
						else if (i < 36)
							printf("(NOT) module %d is not discharging in the third stack (V = %d)\n", i + 1 - 24, pack_modules[i]);
					}
				}
			}
			// Call BTM_BAL_setDischarge to send the settings to the configuration register
			printf("calling BTM_BAL_setDischarge\n\n");

			for (int i = 0; i < size; i++) {
				if (pack_modules_en[i]) {
					if (pack_modules[i] > Vth + 50)
						pack_modules[i] -= (rand() % 50);
					else
						pack_modules[i] += (rand() % 50);
				}
			}
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
							printf("(NOT) module %d is not discharging in the first stack (V = %d)\n", i + 1, pack_modules[i]);
						else if (i < 24)
							printf("(NOT) module %d is not discharging in the second stack (V = %d)\n", i + 1 - 12, pack_modules[i]);
						else if (i < 36)
							printf("(NOT) module %d is not discharging in the third stack (V = %d)\n", i + 1 - 24, pack_modules[i]);
					}
				}
			}
			// Call BTM_BAL_setDischarge to send the settings to the configuration register
			printf("calling BTM_BAL_setDischarge\n\n");

			for (int i = 0; i < size; i++) {
				if (pack_modules_en[i]) {
					if (pack_modules[i] > Vmin + Vtol)
						pack_modules[i] -= (rand() % 50);
					else if (pack_modules[i] < 41950)
						pack_modules[i] += (rand() % 200);
				}
			}
		}
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
