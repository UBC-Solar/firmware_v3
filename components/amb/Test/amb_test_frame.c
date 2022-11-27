/*
 * THIS IS THE MAIN TEST FRAME FOR THE AMB (ARRAY MONITOR BOARD)
 *
 * amb_test_frame.c contains code for directly interfacing
 * with the firmware.
 *
 * See usage below for how to use this file.
 */


/* INCLUDES */
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <stdint.h>


/* MACRO DEFINITIONS */
#define ALL "all"
#define ALL_VOLT "all_voltage"
#define ALL_CURR "all_current"
#define ALL_TEMP "all_temperature"
#define V1 "vsense1"
#define V2 "vsense2"
#define I1 "isense1"
#define I2 "isense2"
#define T1 "temp1"
#define T2 "temp2"
#define T3 "temp3"
#define T4 "temp4"
#define T5 "temp5"
#define T6 "temp6"
#define T7 "temp7"
#define T8 "temp8"

#define NUM_TESTS 12
#define DEF 255

/* Number of ADC Channels Total */
#define ADC_CHANNELS 16

/* 12 ADC Channel Assignments (10-13 NOT IN USE) */
#define VSENSE1			6
#define VSENSE2			5
#define ISENSE1			7
#define ISENSE2			4
#define TEMP_1			0
#define TEMP_2			1
#define TEMP_3			2
#define TEMP_4			3
#define TEMP_5			14
#define TEMP_6			15
#define TEMP_7			8
#define TEMP_8			9

/* Type Definitions */
typedef union FloatBytes {
	float float_value;			/**< Float value member of the union. */
	uint8_t bytes[4];			/**< Array of 4 bytes member of union. */
} FloatBytes;

/* GLOBAL VARIABLES */
uint32_t ADC_VALUES[ADC_CHANNELS] = {0};	// unprocessed ADC Values
union FloatBytes CONVERTED_VALUES[ADC_CHANNELS] = {0};	// processed values

/* FUNCTION DECLARATIONS */
void usage();
bool parse_input(char input[]);
bool run_test(uint8_t tests[]);
void Convert_Values(uint8_t index);

/* MAIN */
/* Accepts test types as command line arguments */

/* Usage:
 *
 *
 *
 */
int main(int argc, char *argv[]) {
	/* Check Argument Count, print usage if incorrect */
	if (argc != 2) {
		printf("Please enter valid test arguments.\n");
		usage();
		return 1;
	}
	/* Check if TEST_TYPE argument is valid */
	if (parse_input(argv[1]) == false) {
		/* Invalid TEST_TYPE Selection */
		return 1;
	}


	return 0;
}



/* HELPER FUNCTIONS */
void usage() {
	printf("USAGE:\n");
	printf("     1. Compile\n");
	printf("        gcc amb_test_frame.c -o amb_test_frame\n");
	printf("\n");
	printf("     2. Run\n");
	printf("        ./amb_test_frame TEST_TYPE");
	printf("\n");
	printf("        TEST_TYPE options:");
	printf("\n");
	printf("           all ---------- runs all tests - voltage, current, temperature (tests 0-11)\n");
	printf("           all_voltage -- runs all voltage tests (tests 0,1)\n");
	printf("           all_current -- runs all current tests (tests 2,3)\n");
	printf("           all_temperature -- runs all temperature tests (tests 4-11)\n");
	printf("           vsense1 (test 0)\n");
	printf("           vsense2 (test 1)\n");
	printf("           isense1 (test 2)\n");
	printf("           isense2 (test 3)\n");
	printf("           temp1   (test 4)\n");
	printf("           temp2   (test 5)\n");
	printf("           temp3   (test 6)\n");
	printf("           temp4   (test 7)\n");
	printf("           temp5   (test 8)\n");
	printf("           temp6   (test 9)\n");
	printf("           temp7   (test 10)\n");
	printf("           temp8   (test 11)\n");

}

bool parse_input(char input[]) {
	// Create test list
	uint8_t tests[NUM_TESTS] = {[0 ... NUM_TESTS-1] = DEF};
	printf("\n");
	if (strcmp(input, ALL) == 0) {
		printf("Selected TEST_TYPE: %s\n", input);
		uint8_t tests[] = {6, 5, 7, 4, 0, 1, 2, 3, 14, 15, 8, 9};
	}
	else if (strcmp(input, ALL_VOLT) == 0) {
		printf("Selected TEST_TYPE: %s\n", input);
		tests[0] = 6;
		tests[1] = 5;
	}
	else if (strcmp(input, ALL_CURR) == 0) {
		printf("Selected TEST_TYPE: %s\n", input);
		tests[2] = 7;
		tests[3] = 4;
	}
	else if (strcmp(input, ALL_TEMP) == 0) {
		printf("Selected TEST_TYPE: %s\n", input);
		uint8_t tests[] = {DEF, DEF, DEF, DEF, 0, 1, 2, 3, 14, 15, 8, 9};
	}
	else if (strcmp(input, V1) == 0) {
		printf("Selected TEST_TYPE: %s\n", input);
		tests[0] = 6;
	}
	else if (strcmp(input, V2) == 0) {
		printf("Selected TEST_TYPE: %s\n", input);
		tests[1] = 5;
	}
	else if (strcmp(input, I1) == 0) {
		printf("Selected TEST_TYPE: %s\n", input);
		tests[2] = 7;
	}
	else if (strcmp(input, I2) == 0) {
		printf("Selected TEST_TYPE: %s\n", input);
		tests[3] = 4;
	}
	else if (strcmp(input, T1) == 0) {
		printf("Selected TEST_TYPE: %s\n", input);
		tests[4] = 0;
	}
	else if (strcmp(input, T2) == 0) {
		printf("Selected TEST_TYPE: %s\n", input);
		tests[5] = 1;
	}
	else if (strcmp(input, T3) == 0) {
		printf("Selected TEST_TYPE: %s\n", input);
		tests[6] = 2;
	}
	else if (strcmp(input, T4) == 0) {
		printf("Selected TEST_TYPE: %s\n", input);
		tests[7] = 3;
	}
	else if (strcmp(input, T5) == 0) {
		printf("Selected TEST_TYPE: %s\n", input);
		tests[8] = 14;
	}
	else if (strcmp(input, T6) == 0) {
		printf("Selected TEST_TYPE: %s\n", input);
		tests[9] = 15;
	}
	else if (strcmp(input, T7) == 0) {
		printf("Selected TEST_TYPE: %s\n", input);
		tests[10] = 8;
	}
	else if (strcmp(input, T8) == 0) {
		printf("Selected TEST_TYPE: %s\n", input);
		tests[11] = 9;
	}
	else {
		/* No Valid TEST_TYPE Selection */
		printf("Please enter a valid TEST_TYPE\n");
		usage();
		return false;
	}
	/* Valid TEST_TYPE */
	bool result = run_test(tests);
	return result;
}

bool run_test(uint8_t tests[]) {
	printf("\n------- Starting Tests -------\n\n\n");

	for (int i = 0; i < NUM_TESTS; i++) {
		int adc_channel = tests[i];
		if (adc_channel == DEF) {
			/* Skip test */
			continue;
		}
		/* test == 1, run test */
		char name[10] = "";
		switch(adc_channel) {
			case 6: strcpy(name, "VSENSE1"); break;
			case 5: strcpy(name, "VSENSE2"); break;
			case 7: strcpy(name, "ISENSE1"); break;
			case 4: strcpy(name, "ISENSE2"); break;
			case 0: strcpy(name, "TEMP1"); break;
			case 1: strcpy(name, "TEMP2"); break;
			case 2: strcpy(name, "TEMP3"); break;
			case 3: strcpy(name, "TEMP4"); break;
			case 14: strcpy(name, "TEMP5"); break;
			case 15: strcpy(name, "TEMP6"); break;
			case 8: strcpy(name, "TEMP7"); break;
			case 9: strcpy(name, "TEMP8"); break;
		}
		printf("Running test for %s \n\n\n", name);
		/* Generate Inputs */
		for (int j = 6; j < 13; j += 1) {
			ADC_VALUES[adc_channel] = (int) pow(2, j);
			printf("Input ADC Value: %d\n", ADC_VALUES[adc_channel]);
			Convert_Values(adc_channel);
			char unit[3] = "";
			switch(adc_channel) {
				case 6: case 5: strcpy(unit, "mV"); break;
				case 7: case 4: strcpy(unit, "A "); break;
				default: strcpy(unit, "C "); break;
			}
			printf("Converted Value: %.2lf%s\n\n", CONVERTED_VALUES[adc_channel].float_value, unit);
		}
		printf("\n\n----------------------------------------\n\n");
	}
}

void Convert_Values(uint8_t index) {
	/* Convert value with appropriate relationship given the index */

	switch(index) {
		case VSENSE1:
			/* Relationship for converting ADC for VSENSE1 and modify CONVERTED_VALUES[index] */
			/* TODO: Figure out actual Voltage Equations */
			/* V = 3.3 * ADC_VAL / 2^12 */
			CONVERTED_VALUES[index].float_value = 3.3 * ADC_VALUES[index] / pow(2, 12) * 1000;	// slightly overcompensates
			break;
		case VSENSE2:
			/* Relationship for converting ADC for VSENSE1 and modify CONVERTED_VALUES[index] */
			CONVERTED_VALUES[index].float_value = 3.3 * ADC_VALUES[index] / pow(2, 12) * 1000;	// slightly undercompensates
			break;

		case ISENSE1:
			/* ISENSE [V] = 0.11 * I [A] + 1.65 */
			CONVERTED_VALUES[index].float_value = (ADC_VALUES[index] - 1.65) / 0.11;
			break;
		case ISENSE2:
			/* ISENSE [V] = 0.11 * I [A] + 1.65 */
			CONVERTED_VALUES[index].float_value = (ADC_VALUES[index] - 1.65) / 0.11;
			break;
		case TEMP_1:
			/* TODO */
			break;
		case TEMP_2:
			/* TODO */
			break;
		case TEMP_3:
			/* TODO */
			break;
		case TEMP_4:
			/* TODO */
			break;
		case TEMP_5:
			/* TODO */
			break;
		case TEMP_6:
			/* TODO */
			break;
		case TEMP_7:
			/* TODO */
			break;
		case TEMP_8:
			/* Relationship for converting to temperature and modify CONVERTED_VALUES[index] */
			/* TODO */
			break;
	}
}
