/**
 *  @file balancing.h
 *  @brief Header file for module passive balancing algorithm
 *
 *  @date 2020/08/18
 *  @author abooodeee
 */

#ifndef INC_BALANCING_H_
#define INC_BALANCING_H_

#include "pack.h"

/*============================================================================*/
/* CONFIGURABLE PARAMETERS */

// Threshold module voltage above which to perform balancing
// At least one module must have a avoltage higher than this threshold for balancing to take place.
#define BAL_MIN_VOLTAGE_FOR_BALANCING 40000U // 4.0V

// Voltage difference within which modules are counted as balanced
#define BAL_VOLTAGE_DIFFERENCE_TOLERANCE 500U // 0.05V

// Voltages are multiplied by 10^4 to match the format in the pack

/*============================================================================*/
/* FUNCTION PROTOTYPES */

void BAL_updateBalancing(Pack_t *pack);

#endif /* INC_BALANCING_H_ */
