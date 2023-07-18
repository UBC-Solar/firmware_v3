/**
 *  @file ltc6813_btm_temp.h
 *  @brief Header file for driver for LTC6813-1 temperature monitoring
 *
 *  @date 2020/08/18
 *  @author abooodeee
 */

#ifndef INC_LTC6813_BTM_TEMP_H_
#define INC_LTC6813_BTM_TEMP_H_

#include "pack.h"
#include "ltc6813_btm.h"

/*============================================================================*/
/* DEFINITIONS */

#define BTM_TEMP_NUM_THERMISTOR_INPUTS_PER_DEVICE 16

/*============================================================================*/
/* STRUCTURES */

struct BTM_TEMP_RawDeviceTemperatures_s {
    float temperature[BTM_TEMP_NUM_THERMISTOR_INPUTS_PER_DEVICE];
};

typedef struct {
    struct BTM_TEMP_RawDeviceTemperatures_s device[BTM_NUM_DEVICES];
} BTM_TEMP_RawTemperatures_t;

/*============================================================================*/
/* FUNCTION PROTOTYPES */

BTM_Status_t BTM_TEMP_getTemperaturesRaw(BTM_TEMP_RawTemperatures_t *temperatureData);
BTM_Status_t BTM_TEMP_getTemperatures(Pack_t *pack);

#endif /* INC_LTC6813_BTM_TEMP_H_ */
