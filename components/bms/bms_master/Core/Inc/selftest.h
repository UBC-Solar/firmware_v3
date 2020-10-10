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

/*============================================================================*/
/* CONFIGURABLE PARAMETERS */
#define LTC_TEMPLIMIT 100


#endif /* INC_SELFTEST_H_ */
