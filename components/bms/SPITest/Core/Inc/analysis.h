/**
 *  @file analysis.h
 *  @brief Header file for analysis functions
 *
 *  @date 2020/07/10
 *  @author Andrew Hanlon (a2k-hanlon)
 */

#ifndef INC_ANALYSIS_H_
#define INC_ANALYSIS_H_

#include "ltc6811_btm.h"

/*============================================================================*/
/* CONFIGURABLE PARAMETERS */

// Integer values in 10th's of mV
#define HLIM_WARNING_VOLTAGE    42000U // 4.2000 V
#define HLIM_FAULT_VOLTAGE      42100U // 4.2100 V
#define LLIM_WARNING_VOLTAGE    27100U // 2.7100 V
#define LLIM_FAULT_VOLTAGE      27000U // 2.7000 V
#define SHORT_FAULT_VOLTAGE     05000U // 0.5000 V

// A temperature beyond the limit temps will be regarded as a bad read (fault)
#define LOW_READ_LIMIT_TEMP     0.0  // degrees C
#define HIGH_READ_LIMIT_TEMP    150.0// degrees C
#define LOW_WARNING_TEMP        10.0 // degrees C
#define HIGH_WARNING_TEMP       45.0 // degrees C
#define OT_FAULT_TEMP           55.0 // degrees C

/*============================================================================*/
/* FUNCTION PROTOTYPES */
void ANA_analyzeModuleVoltages(BTM_PackData_t * pack);
void ANA_analyzeModuleTemps(BTM_PackData_t * pack);

#endif /* INC_ANALYSIS_H_ */
