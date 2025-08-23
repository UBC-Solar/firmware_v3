#ifndef DIAGNOSTIC_H_
#define DIAGNOSTIC_H_

#include "stdint.h"
#include "stdbool.h"

typedef union {
    struct {
        volatile bool tel_crash_iwdg : 1;         
        volatile bool imu_read_fail : 1;
        volatile bool imu_write_fail : 1;
        volatile bool gps_read_fail : 1;
        volatile bool gps_write_fail : 1;
    } bits;
    volatile uint8_t raw;
} TEL_Diagnostic_Flags_t;

extern volatile uint32_t g_time_since_bootup;
extern TEL_Diagnostic_Flags_t g_tel_diagnostic_flags;

void DIAGNOSTIC_send_can();
void DIAGNOSTIC_init();

#endif /* DIAGNOSTIC_H_ */
