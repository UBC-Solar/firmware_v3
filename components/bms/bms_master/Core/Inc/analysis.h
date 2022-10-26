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

// BMS Status Code Bitmasks

// Faults
#define FLT_COMM_MASK 0x00001
#define FLT_TEST_MASK 0x00002
#define FLT_OT_MASK 0x00004
#define FLT_UV_MASK 0x00008
#define FLT_OV_MASK 0x00010
#define FLT_ISOL_MASK 0x00020
#define FLT_DOC_COC_MASK 0x00040
#define FLT_SHORT_MASK 0x00080
#define FLT_TEMP_RANGE_MASK 0x00100

// Trips
#define TRIP_BAL_MASK 0x00200
#define TRIP_LLIM_MASK 0x00400
#define TRIP_HLIM_MASK 0x00800
#define TRIP_CHARGE_OT_MASK 0x01000

// Warns
#define WARN_LOW_V_MASK 0x02000
#define WARN_HIGH_V_MASK 0x04000
#define WARN_LOW_T_MASK 0x08000
#define WARN_HIGH_T_MASK 0x10000
#define WARN_REGEN_OFF 0x20000

// Helper masks
#define FAULTS_MASK 0x001FF // Covers all faults

/*============================================================================*/
/* FUNCTION PROTOTYPES */
void ANA_analyzeModules(BTM_PackData_t *pack);
unsigned int ANA_mergeModuleStatusCodes(BTM_PackData_t *pack);
float ANA_findHighestModuleTemp(BTM_PackData_t *pack);
void ANA_writePackBalStatus(BTM_PackData_t *pack);

#endif /* INC_ANALYSIS_H_ */
