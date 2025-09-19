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
#include "pack.h"
#include <stdbool.h>

/*============================================================================*/
/* CONFIGURABLE PARAMETERS */

// Voltage values in 10th's of mV

// Faults
#define FLT_OV_THRESHOLD 42100U             // 4.2100 V
#define FLT_UV_THRESHOLD 27000U             // 2.7000 V
#define FLT_SHORT_THRESHOLD 5000U           // 0.5000 V
#define FLT_TEMP_RANGE_LOW_THRESHOLD 0.0    // degrees C
#define FLT_TEMP_RANGE_HIGH_THRESHOLD 150.0 // degrees C
#define FLT_OT_DCH_THRESHOLD 60.0               // degrees C
#define FLT_OT_CH_THRESHOLD 45.0

// Trips
#define TRIP_HLIM_THRESHOLD 42000U    // 4.2000 V
#define TRIP_LLIM_THRESHOLD 27100U    // 2.7100 V
#define TRIP_CHARGE_OT_THRESHOLD 44.0 // degrees C

// Warnings
#define WARN_HIGH_V_THRESHOLD 41500U // 4.1500 V
#define WARN_LOW_V_THRESHOLD 27500U  // 2.7500 V
#define WARN_LOW_T_THRESHOLD 10.0    // degrees C
#define WARN_HIGH_T_THRESHOLD 55.0   // degrees C

// Hysteresis prevents spurious flip-flopping of status bits
// derived from measurements if a measurement is right around
// a threshold and not very steady
#define VOLTAGE_THRESHOLD_HYSTERESIS 100U // 0.01 V
#define TEMPERATURE_THRESHOLD_HYSTERESIS 2.0 // degrees C

/*============================================================================*/
/* FUNCTION PROTOTYPES */

void ANA_analyzePack(Pack_t *pack);
void ANA_writeBalStatus(Pack_Module_t *module, bool discharge_active);
float ANA_findHighestModuleTemp(Pack_t *pack);

#endif /* INC_ANALYSIS_H_ */
