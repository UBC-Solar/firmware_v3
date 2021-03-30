/**
 * @file selftest.h
 * @brief Header file for BMS startup self tests
 *
 * @date 2020/10/03
 * @author matthewjegan
 */

#ifndef INC_SELFTEST_H_
#define INC_SELFTEST_H_

#include "ltc6813_btm.h"
#include "ltc6813_btm_bal.h"

/*============================================================================*/
/* FUNCTION PROTOTYPES */
BTM_Status_t ST_checkLTCtemp(void);
BTM_Status_t ST_checkOpenWire(void);
BTM_Status_t ST_checkOverlapVoltage(void);
BTM_Status_t ST_checkVREF2(void);
BTM_Status_t ST_verifyDischarge(BTM_PackData_t* pack);

/*============================================================================*/
/* CONFIGURABLE PARAMETERS */
#define ST_LTC_TEMPLIMIT 100 	// Maximum Acceptable Die Temperature for LTC6813

#define ST_VOLTAGE_ERROR 0.003 	// Tolerated error (in volts) for all voltage measurements

#define ST_DCH_COMPARE_PCT 0.50 // *This is a placeholder value. The actual percentage will be dependent
                                // on the resistance values chosen for Rdischarge in the
                                // next PCB design revision (currently v3).

#define ST_DCH_PCT_DELTA 0.01   // Tolerated difference between measured and expected percentage

#endif /* INC_SELFTEST_H_ */
