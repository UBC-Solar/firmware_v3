/**
 *  @file analysis.h
 *  @brief Header file for analysis functions
 *
 *  @date 2020/07/10
 *  @author Andrew Hanlon (a2k-hanlon)
 *  @note see https://docs.google.com/spreadsheets/d/1xxisHuuusQCDnes3OIaucntGGlHj25iUrbfAIXvNhaM/edit#gid=0

 */

#ifndef INC_ANALYSIS_H_
#define INC_ANALYSIS_H_

#include "ltc6813_btm.h"

/*============================================================================*/
/* CONFIGURABLE PARAMETERS */

// Voltage values in 10th's of mV

// Faults
#define FLT_OV_THRESHOLD 42100U             // 4.2100 V
#define FLT_UV_THRESHOLD 27000U             // 2.7000 V
#define FLT_SHORT_THRESHOLD 05000U          // 0.5000 V
#define FLT_TEMP_RANGE_LOW_THRESHOLD 0.0    // degrees C
#define FLT_TEMP_RANGE_HIGH_THRESHOLD 150.0 // degrees C
#define FLT_OT_THRESHOLD 65.0               // degrees C

// Trips
#define TRIP_HLIM_THRESHOLD 42000U    // 4.2000 V
#define TRIP_LLIM_THRESHOLD 27100U    // 2.7100 V
#define TRIP_CHARGE_OT_THRESHOLD 45.0 // degrees C

// Warns
#define WARN_HIGH_V_THRESHOLD 41500U // 4.1500 V
#define WARN_LOW_V_THRESHOLD 27500U  // 2.7500 V
#define WARN_LOW_T_THRESHOLD 10.0    // degrees C
#define WARN_HIGH_T_THRESHOLD 45.0   // degrees C

/*============================================================================*/
/* PUBLIC CONSTANTS */

// TODO: update bitmasks with new bit order and values

// BMS Status Code Bitmasks
#define BMS_FAULT_COMM 0x0001
#define BMS_FAULT_ST 0x0002
#define BMS_FAULT_OT 0x0004
#define BMS_FAULT_UV 0x0008
#define BMS_FAULT_OV 0x0010
#define BMS_FAULT_NO_VOLT 0x0020
#define BMS_FAULT_TEMP_RANGE 0x0040

#define BMS_TRIP_BAL 0x0100
#define BMS_TRIP_LLIM 0x0200
#define BMS_TRIP_HLIM 0x0400
#define BMS_TRIP_CHARGE_OT 0x0800
#define BMS_WARNING_LOW_V 0x1000
#define BMS_WARNING_HIGH_V 0x2000
#define BMS_WARNING_LOW_T 0x4000
#define BMS_WARNING_HIGH_T 0x8000

#define MASK_BMS_FAULT 0x007F        // Covers all faults
#define MASK_BMS_SYSTEM_FAULT 0x0003 // Covers COMM and ST faults

/*============================================================================*/
/* FUNCTION PROTOTYPES */
void ANA_analyzeModules(BTM_PackData_t *pack);
int ANA_mergeModuleStatusCodes(BTM_PackData_t *pack);
float ANA_findHighestModuleTemp(BTM_PackData_t *pack);

#endif /* INC_ANALYSIS_H_ */
