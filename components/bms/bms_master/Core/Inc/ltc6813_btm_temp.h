/**
 *  @file ltc6813_btm_temp.h
 *  @brief Header file for driver for LTC6813-1 temperature monitoring
 *
 *  @date 2020/08/18
 *  @author abooodeee
 */

#ifndef INC_LTC6813_BTM_TEMP_H_
#define INC_LTC6813_BTM_TEMP_H_

#include "ltc6813_btm.h"

/*============================================================================*/
/* FUNCTION PROTOTYPES */

BTM_Status_t BTM_TEMP_measureState(BTM_PackData_t* pack);

#endif /* INC_LTC6813_BTM_TEMP_H_ */
