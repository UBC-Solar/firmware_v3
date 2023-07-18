/**
 * @file selftest.h
 * @brief Header file for BMS startup self tests
 *
 * @date 2020/10/03
 * @author matthewjegan
 */

#ifndef INC_SELFTEST_H_
#define INC_SELFTEST_H_

#include "util.h"
#include "ltc6813_btm.h"
#include "ltc6813_btm_bal.h"

/*============================================================================*/
/* FUNCTION PROTOTYPES */
BTM_Status_t ST_checkLTCtemp(void);
BTM_Status_t ST_checkVREF2(void);
BTM_Status_t ST_shortedCells(void);
BTM_Status_t ST_checkOpenWire(void);
BTM_Status_t ST_checkOverlapVoltage(void);
BTM_Status_t ST_verifyDischarge(Pack_t *pack);

// Testable private functions
#ifdef TEST
STATIC_TESTABLE void itmpConversion(uint16_t ITMP[BTM_NUM_DEVICES], float temp_celsius[BTM_NUM_DEVICES]);
#endif // TEST

/*============================================================================*/
/* CONFIGURABLE PARAMETERS */

#define ST_LTC_TEMPLIMIT 70.0f 	// Maximum acceptable Die Temperature for LTC6813
                                // This should be chosen by team as a safe temp.
                                // to see inside a battery pack
                                // The IC itself can survive higher temperatures

#define ST_VOLTAGE_ERROR 0.003f // Tolerated error (in volts) for all voltage measurements

#define ST_DCH_COMPARE_PCT 0.50f// *This is a placeholder value. The actual percentage will be dependent
                                // on the resistance values chosen for Rdischarge in the
                                // next PCB design revision (currently v3).

#define ST_DCH_PCT_DELTA 0.01f  // Tolerated difference between measured and expected percentage

#endif /* INC_SELFTEST_H_ */
