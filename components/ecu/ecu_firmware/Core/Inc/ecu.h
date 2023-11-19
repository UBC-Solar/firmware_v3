/**
 *  @file ecu.h
 *  @brief Header file for common ecu data structures and module mapping to hardware
 *
 *  @date 2023/11/18
 *  @author Harris Mai (harristmai)
 */

#ifndef __ECU_H
#define __ECU_H

#include <stdint.h>
#include <stdbool.h>

/*============================================================================*/
/* STRUCTURES */

typedef union {
    struct {
        bool warning_pack_overdischarge : 1;          // WARN_PACK_ODC
        bool warning_pack_overcharge : 1;             // WARN_PACK_OC

        bool fault_discharge_overcurrent : 1;         // FLT_DOC
        bool fault_charge_overcurrent : 1;            // FLT_COC

        bool DOC_COC : 1;                             // Discharge and charge overcurrent status
        bool ESTOP : 1;                               // Active High: ESTOP pressed

        uint8_t _reserved : 2;
    } bits;
    uint8_t raw;
} ECU_CurrentStatusCode_t;

extern struct {
    int8_t pack_current;
    uint8_t lv_current;
    ECU_CurrentStatusCode_t status;
} ECU_t;

#endif /* __ECU_H */