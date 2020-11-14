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

/*============================================================================*/
/* CONFIGURABLE PARAMETERS */

// ANDREW: To avoid overwriting constants, I'd suggest prefixing any constants
// in the header file here with "ST_". Just don't forget to change them where
// they're used if you do this!
#define ST_LTC_TEMPLIMIT 100

#define OVERLAP_TEST_REGS 2
#define NUM_TEST_CELLS 2
#define OVERLAP_READINGS_PER_REG 2
#define OVERLAP_READINGS_PER_BOARD 4
#define ST_OVERLAP_DELTA 0.003

#define NUM_CELL_VOLT_REGS 6
#define READINGS_PER_REG 3
#define ST_OPEN_WIRE_VOLTAGE -0.400

#define ST_VREF_READINGS_PER_REG 1
#define ST_VREF_LOWERBOUND 2.990
#define ST_VREF_UPPERBOUND 3.014

#define DCPERMITTED 1
#define ST_NUM_S_PINS 18

#endif /* INC_SELFTEST_H_ */
