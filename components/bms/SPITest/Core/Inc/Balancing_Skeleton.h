/**
 * @file Balancing_Skeleton.h
 *
 * @date Jun. 9, 2020
 *
 */

#ifndef INC_BALANCING_SKELETON_H_
#define INC_BALANCING_SKELETON_H_

#include "ltc6811_btm.h"
#include "ltc6811_btm_bal.h"

void BTM_BAL_settings(
    BTM_PackData_t* pack,
    BTM_BAL_dch_setting_pack_t* dch_setting_pack);

#endif /* INC_BALANCING_SKELETON_H_ */
