/**
 * @file bms_main.h
 * @brief Header file for BMS top-level functions
 *
 * @date June 10, 2023
 * @author Mischa Johal
 * @author Andrew Hanlon
 */

#ifndef INC_BMS_MAIN_H_
#define INC_BMS_MAIN_H_

#include "pack.h"

/*============================================================================*/
/* FUNCTION PROTOTYPES */

void BMS_MAIN_startupChecks(Pack_t *pack);
void BMS_MAIN_updatePackData(Pack_t *pack);
void BMS_MAIN_driveOutputs(Pack_t *pack);
void BMS_MAIN_sendCanMessages(Pack_t *pack);

#endif // INC_BMS_MAIN_H_
