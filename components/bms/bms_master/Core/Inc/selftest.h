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
#define ST_LTC_TEMPLIMIT 100 // Maximum Acceptable Die Temperature for LTC6813

#define OVERLAP_TEST_REGS 2 // Number of registers overlap voltage is read from.
							// One register to compare ADC 1 and ADC 2 (Group C),
							// and another to compare ADC 2 and ADC 3 (Group E).

#define NUM_TEST_CELLS 2	// Overlap Voltage test reads cells 7 and 13
#define OVERLAP_READINGS_PER_REG 2 // A voltage reading from each ADC is stored in the same register
#define OVERLAP_READINGS_PER_BOARD 4 // 2 bits combine to represent a single voltage reading
#define ST_OVERLAP_DELTA 0.003 // Maximum tolerated measurement difference (in V) between ADCs

#define NUM_CELL_VOLT_REGS 6
#define READINGS_PER_REG 3
#define ST_OPEN_WIRE_VOLTAGE -0.400

#define ST_VREF_LOWERBOUND 2.990 // Establishes range of acceptable voltages for VREF2 measurement.
#define ST_VREF_UPPERBOUND 3.014 // (specified on p.30 of LTC6813-1 datasheet)

#define ST_DCH_COMPARE_PCT 0.50 // *This is a placeholder value. The actual percentage will be dependent
								// on the resistance values chosen for Rfilter and Rdischarge in the
								// next PCB design revision (currently v3).
#define ST_DCH_PCT_DELTA 0.01   // Tolerated difference between measured and expected percentage

#endif /* INC_SELFTEST_H_ */
