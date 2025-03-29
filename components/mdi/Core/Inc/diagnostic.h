#ifndef DIAGNOSTIC_H_
#define DIAGNOSTIC_H_

#include "stdint.h"
#include "stdbool.h"

typedef union {
    struct {
        volatile bool mdi_crash_iwdg : 1;         
    } bits;
    volatile uint8_t raw;
} MDI_Diagnostic_Flags_t;

extern MDI_Diagnostic_Flags_t g_mdi_diagnostic_flags;

#endif /* DIAGNOSTIC_H_ */
