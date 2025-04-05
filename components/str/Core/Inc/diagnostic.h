#ifndef DIAGNOSTIC_H_
#define DIAGNOSTIC_H_

#include "stdint.h"
#include "stdbool.h"

typedef union {
    struct {
        volatile bool str_crash_iwdg : 1;         
    } bits;
    volatile uint8_t raw;
} STR_Diagnostic_Flags_t;

extern STR_Diagnostic_Flags_t g_str_diagnostic_flags;

#endif /* DIAGNOSTIC_H_ */