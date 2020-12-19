/**
 * @file selftest.h
 * @brief Header file for BMS startup self tests
 *
 * @date 2020/10/03
 * @author
 */

#ifndef INC_SELFTEST_H_
#define INC_SELFTEST_H_

#include "ltc6813_btm.h"

/*============================================================================*/
/* FUNCTION PROTOTYPES */
BTM_Status_t ST_checkLTCtemp(void);
BTM_Status_t ST_checkOpenWire(void);
BTM_Status_t ST_checkOverlapVoltage(void);
BTM_Status_t ST_checkVREF2(void);
BTM_Status_t ST_verifyDischarge(BTM_PackData_t* pack);

/*============================================================================*/
/* CONFIGURABLE PARAMETERS */

// ANDREW: To avoid overwriting constants, I'd suggest prefixing any constants
// in the header file here with "ST_". Just don't forget to change them where
// they're used if you do this!

/*
ANDREW: The way define statements in this program have been organized so far
is in 3 categories as follows:

a) Configurable parameters, in .h files - eg. ST_LTC_TEMPLIMIT
b) "Public constants", in .h files - constants that are needed by code that calls
   functions declared here, but should not ever need to be changed; eg. some array sizes
c) "Private constants", in .c files - constants that are only needed within
   the selftest functions, and should not be configurable

I think all the constants that don't have ST_ at the beginning, plus
ST_SC_CELLS, ST_SC_REGS, ST_OPEN_WIRE_VOLTAGE, ST_VREF_LOWERBOUND, ST_VREF_UPPERBOUND
could be moved to selftest.c since they are category c) constants as above.

Then, consider separating the category a) and category b) constants here in the
header file if necessary.
*/

#define ST_LTC_TEMPLIMIT 100 // Maximum Acceptable Die Temperature for LTC6813


#define ST_SC_CELLS 2 // Pins C18 and C12 on LTC6813-1 Slave Board should be shorted.
#define ST_SC_REGS 2  // Their voltages are stored in Registers D and F
#define EXPECTED_SC_VOLTAGE 0.0
#define ST_SC_DELTA 0.003 // ANDREW: a comment here on what this is for would be helpful

#define OVERLAP_TEST_REGS 2 // Number of registers overlap voltage is read from.
							// One register to compare ADC 1 and ADC 2 (Group C),
							// and another to compare ADC 2 and ADC 3 (Group E).

#define NUM_TEST_CELLS 2	// Overlap Voltage test reads cells 7 and 13
#define OVERLAP_READINGS_PER_REG 2 // A voltage reading from each ADC is stored in the same register
#define OVERLAP_READINGS_PER_BOARD 4 // 2 bytes combine to represent a single voltage reading
#define ST_OVERLAP_DELTA 0.003 // Maximum tolerated measurement difference (in V) between ADCs

#define NUM_CELL_VOLT_REGS 6 // ANDREW: also defined in ltc_btm.c - should unify
#define READINGS_PER_REG 3 // ANDREW: also defined in ltc_btm.c - should unify
#define ST_OPEN_WIRE_VOLTAGE -0.400

#define ST_VREF_LOWERBOUND 2.990 // Establishes range of acceptable voltages for VREF2 measurement.
#define ST_VREF_UPPERBOUND 3.014 // (specified on p.30 of LTC6813-1 datasheet)

#define ST_DCH_COMPARE_PCT 0.50 // *This is a placeholder value. The actual percentage will be dependent
								// on the resistance values chosen for Rfilter and Rdischarge in the
								// next PCB design revision (currently v3).
#define ST_DCH_PCT_DELTA 0.01   // Tolerated difference between measured and expected percentage

#endif /* INC_SELFTEST_H_ */
