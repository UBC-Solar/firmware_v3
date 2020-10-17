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
#define LTC_TEMPLIMIT 100

#define OVERLAP_TEST_REGS 2
#define NUM_TEST_CELLS 2
#define OVERLAP_READINGS_PER_REG 2
#define OVERLAP_READINGS_PER_BOARD 4
#define OVERLAP_DELTA 0.003

#define NUM_CELL_VOLT_REGS 6
#define READINGS_PER_REG 3
#define BTM_VOLTAGE_CONVERSION_FACTOR 0.0001
#define OPEN_WIRE_VOLTAGE_THRESHOLD -0.400


#endif /* INC_SELFTEST_H_ */
