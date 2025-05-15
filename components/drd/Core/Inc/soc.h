#ifndef SOC_H
#define SOC_H

/* INCLUDES */
#include <stdint.h>

/* DEFINES */
#define TIME_STEP           (100.0f)
#define SOC_TIME_STEP       (TIME_STEP / 1000.0f)   // ms

/* GLOBALS */
extern volatile float g_total_pack_voltage_soc;
extern volatile float g_pack_current_soc;

/* FUNCTION DECLARATIONS */
void SOC_predict_then_update(float g_total_pack_voltage_soc, float g_pack_current_soc, float time_step);
float SOC_get_soc();        // 0 - 1
float SOC_get_voltage();
float SOC_get_Uc();

#endif // SOC_H
