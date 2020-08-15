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
#define OV_FAULT_VOLTAGE        42100U // 4.2100 V
#define HLIM_TRIP_VOLTAGE       42000U // 4.2000 V
#define HIGH_WARNING_VOLTAGE    41500U // 4.1500 V

#define LOW_WARNING_VOLTAGE     27500U // 2.7500 V
#define LLIM_TRIP_VOLTAGE       27100U // 2.7100 V
#define UV_FAULT_VOLTAGE        27000U // 2.7000 V

#define SHORT_FAULT_VOLTAGE     05000U // 0.5000 V

// A temperature beyond the limit temps will be regarded as a bad read (fault)
#define LOW_READ_LIMIT_TEMP     0.0  // degrees C
#define HIGH_READ_LIMIT_TEMP    150.0// degrees C
#define LOW_WARNING_TEMP        10.0 // degrees C
#define HIGH_WARNING_TEMP       45.0 // degrees C
#define CHARGE_OT_TEMP          45.0 // degrees C
#define OT_FAULT_TEMP           65.0 // degrees C

/*============================================================================*/
/* PUBLIC CONSTANTS */

// BMS Status Code Bitmasks
#define BMS_FAULT_COMM          0x0001
#define BMS_FAULT_ST            0x0002
#define BMS_FAULT_OT            0x0004
#define BMS_FAULT_UV            0x0008
#define BMS_FAULT_OV            0x0010
#define BMS_FAULT_NO_VOLT       0x0020
#define BMS_FAULT_TEMP_RANGE    0x0040

#define BMS_TRIP_BAL            0x0100
#define BMS_TRIP_LLIM           0x0200
#define BMS_TRIP_HLIM           0x0400
#define BMS_TRIP_CHARGE_OT      0x0800
#define BMS_WARNING_LOW_V       0x1000
#define BMS_WARNING_HIGH_V      0x2000
#define BMS_WARNING_LOW_T       0x4000
#define BMS_WARNING_HIGH_T      0x8000

#define MASK_BMS_FAULT          0x007F // Covers all faults
#define MASK_BMS_SYSTEM_FAULT   0x0003 // Covers COMM and ST faults

/*============================================================================*/
/* FUNCTION PROTOTYPES */
void ANA_analyzeModules(BTM_PackData_t * pack);
int ANA_mergeModuleStatusCodes(BTM_PackData_t * pack);
float ANA_findHighestModuleTemp(BTM_PackData_t * pack);

#endif /* INC_ANALYSIS_H_ */
