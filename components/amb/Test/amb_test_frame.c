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

#include "../Core/Inc/main.h"


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



/* GLOBAL VARIABLES */



/* FUNCTION DECLARATIONS */
void usage();
bool check_input(char input[]);
bool isense1();


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
	if (check_input(argv[1]) == false) {
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
	printf("           all (runs all tests - voltage, current, temperature)\n");
	printf("           all_voltage (runs all voltage tests)\n");
	printf("           all_current (runs all current tests)\n");
	printf("           all_temperature (runs all temperature tests)\n");
	printf("           vsense1\n");
	printf("           vsense2\n");
	printf("           isense1\n");
	printf("           isense2\n");
	printf("           temp1\n");
	printf("           temp2\n");
	printf("           temp3\n");
	printf("           temp4\n");
	printf("           temp5\n");
	printf("           temp6\n");
	printf("           temp7\n");
	printf("           temp8\n");

}

bool check_input(char input[]) {
	printf("\n");
	if (strcmp(input, ALL) == 0) {
		printf("Selected TEST_TYPE: %s\n", input);
	}
	else if (strcmp(input, ALL_VOLT) == 0) {
		printf("Selected TEST_TYPE: %s\n", input);
	}
	else if (strcmp(input, ALL_CURR) == 0) {
		printf("Selected TEST_TYPE: %s\n", input);
	}
	else if (strcmp(input, ALL_TEMP) == 0) {
		printf("Selected TEST_TYPE: %s\n", input);
	}
	else if (strcmp(input, V1) == 0) {
		printf("Selected TEST_TYPE: %s\n", input);
	}
	else if (strcmp(input, V2) == 0) {
		printf("Selected TEST_TYPE: %s\n", input);
	}
	else if (strcmp(input, I1) == 0) {
		printf("Selected TEST_TYPE: %s\n", input);
		isense1();
	}
	else if (strcmp(input, I2) == 0) {
		printf("Selected TEST_TYPE: %s\n", input);
	}
	else if (strcmp(input, T1) == 0) {
		printf("Selected TEST_TYPE: %s\n", input);
	}
	else if (strcmp(input, T2) == 0) {
		printf("Selected TEST_TYPE: %s\n", input);
	}
	else if (strcmp(input, T3) == 0) {
		printf("Selected TEST_TYPE: %s\n", input);
	}
	else if (strcmp(input, T4) == 0) {
		printf("Selected TEST_TYPE: %s\n", input);
	}
	else if (strcmp(input, T5) == 0) {
		printf("Selected TEST_TYPE: %s\n", input);
	}
	else if (strcmp(input, T6) == 0) {
		printf("Selected TEST_TYPE: %s\n", input);
	}
	else if (strcmp(input, T7) == 0) {
		printf("Selected TEST_TYPE: %s\n", input);
	}
	else if (strcmp(input, T8) == 0) {
		printf("Selected TEST_TYPE: %s\n", input);
	}
	else {
		/* No Valid TEST_TYPE Selection */
		printf("Please enter a valid TEST_TYPE\n");
		usage();
		return false;
	}
	/* Valid TEST_TYPE */
	return true;
}

bool isense1() {
	printf("\n------- Starting ISENSE 1 Test -------\n");

}
