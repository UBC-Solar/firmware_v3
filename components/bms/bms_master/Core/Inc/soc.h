/**
 * @file soc.h
 * @brief Header file for functions for calculating the state of charge for
 * UBC Solar's V3 solar car, Brightside, using interpolation and coloumb counting
 * 
 * @date 2021/11/09
 * @author Edward Ma, Forbes Choy
 */


#ifndef INC_SOC_H_
#define INC_SOC_H_

#include "util.h"
#include <stdint.h>
#include "stm32f1xx_hal.h"
#include "ltc6813_btm.h"

/*============================================================================*/
/* PUBLIC DEFINITIONS */

// our cell's website: https://www.18650batterystore.com/products/sanyo-ncr18650ga
// our cell's datasheet: https://cdn.shopify.com/s/files/1/0481/9678/0183/files/Sanyo-Spec-NCR18650GA.pdf?v=1605877395

#define SOC_CELL_MIN_VOLTAGE 2.5                //voltage of a cell when fully discharged
#define SOC_CELL_MAX_VOLTAGE 4.2                //voltage of a cell when fully charged
#define SOC_CELL_DISCHARGE_EFFICIENCY 0.85      //according to http://large.stanford.edu/courses/2010/ph240/sun1/, but subject to (experimental) change
#define SOC_CELL_CHARGE_EFFICIENCY 0.85         //according to http://large.stanford.edu/courses/2010/ph240/sun1/, but subject to (experimental) change
#define SOC_MODULE_RATED_CAPACITY (3.45 * 13)   // capacity [Ah] of each module; each cell has a typical capacity of 3450mAh and each module has 13 cells in parallel

/*============================================================================*/
/* PUBLIC FUNCTION PROTOTYPES */

// functions for initializing the SOC of the modules
float SOC_moduleInit(float cell_voltage);
void SOC_allModulesInit(BTM_PackData_t *pack);

// functions require calling each time when CAN transmits a new current to update our SOC estimation
float SOC_moduleEst(float last_SOC, uint32_t cell_voltage_100uV, int32_t current_reading, uint32_t total_time_elasped);
void SOC_allModulesEst(BTM_PackData_t *pack, int32_t current_reading, uint32_t total_time_elasped);

// functions for getting SOC-related parameters
uint8_t getDOD(BTM_PackData_t *pack);
uint8_t getCapacity(BTM_PackData_t *pack);

/*============================================================================*/
/* PRIVATE FUNCTION PROTOTYPES */

#ifdef TEST
STATIC_TESTABLE int indexOfNearestCellVoltage(float cell_votlage);
STATIC_TESTABLE float calculateDeltaDOD(float present_current, float present_time, float past_current, float past_time);
#endif // TEST

#endif /* INC_SOC_H_ */
