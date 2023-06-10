/**
 *  @file pack.h
 *  @brief Header file for common battery pack data structures and module mapping to hardware
 *
 *  @date 2023/03/18
 *  @author Tigran Hakobyan (Tik-Hakobyan)
 */

#ifndef INC_PACK_H_
#define INC_PACK_H_

#include <stdint.h>
#include <stdbool.h>

/*============================================================================*/
/* PUBLIC CONSTANTS AND MACROS */

#define PACK_NUM_BATTERY_MODULES 32U
#define PACK_MODULE_VOLTAGE_LSB_PER_V 10000U // Module voltage in units of 10ths of a mV

#define PACK_ANY_FAULTS_SET(STATUS) (\
    (STATUS)->bits.fault_communications || \
    (STATUS)->bits.fault_self_test || \
    (STATUS)->bits.fault_over_temperature || \
    (STATUS)->bits.fault_under_voltage || \
    (STATUS)->bits.fault_over_voltage || \
    (STATUS)->bits.fault_isolation_loss || \
    (STATUS)->bits.fault_over_current || \
    (STATUS)->bits.fault_short || \
    (STATUS)->bits.fault_temperature_expected_range \
) // STATUS needs to be of type Pack_BatteryStatusCode_t*

/*============================================================================*/
/* STRUCTURES */

typedef union {
    struct {
        bool fault_communications : 1;              // FLT_COMM
        bool fault_self_test : 1;                   // FLT_TEST
        bool fault_over_temperature : 1;            // FLT_OT
        bool fault_under_voltage : 1;               // FLT_UV
        bool fault_over_voltage : 1;                // FLT_OV
        bool fault_isolation_loss : 1;              // FLT_ISOL
        bool fault_over_current : 1;                // FLT_DOC_COC
        bool fault_short : 1;                       // FLT_SHORT
        bool fault_temperature_expected_range : 1;  // FLT_TEMP_RANGE

        bool balancing_active : 1;                  // TRIP_BAL
        bool llim : 1;                              // TRIP_BAL
        bool hlim : 1;                              // TRIP_HLIM
        bool charge_over_temperature_limit : 1;     // TRIP_CHARGE_OT

        bool warning_low_voltage : 1;               // WARN_LOW_V
        bool warning_high_voltage : 1;              // WARN_HIGH_V
        bool warning_low_temperature : 1;           // WARN_LOW_T
        bool warning_high_temperature : 1;          // WARN_HIGH_T
        bool warning_regen_off : 1;                 // WARN_REGEN_OFF

        uint32_t _reserved : 14;
    } bits;
    uint32_t raw;
} Pack_BatteryStatusCode_t;

/*
 * NOTE: the Pack_module entity would be considered a "cell" by the LTC6813
 * datasheet's naming conventions. Here it's called a module due to the fact
 * that we arrange physical battery cells in parallel to create modules.
 * (the cells in a module are in parallel - they're all at the same voltage
 * and their voltage is measured at the module, not cell level).
 */
typedef struct {
    uint16_t voltage; // stored in the same format in which it is received from the LTC6813
    float temperature;
    float state_of_charge;
    Pack_BatteryStatusCode_t status;
} Pack_Module_t;

typedef struct {
    Pack_Module_t module[PACK_NUM_BATTERY_MODULES];
    Pack_BatteryStatusCode_t status; // intended to be the summary of fault/warning/trip flags
} Pack_t;

// The Pack_module_mapping array stores the assignment of physical battery
// modules to "cell" inputs of the LTC6813-based analog front end ("slave")
// boards of the BMS. Here, "device" refers to one LTC6813 (or one slave board)

typedef struct {
    uint32_t device_num;
    uint32_t cell_num;
} Pack_ModuleIndex_t;

/*============================================================================*/
/* PUBLIC DATA */

extern const Pack_ModuleIndex_t Pack_module_mapping[PACK_NUM_BATTERY_MODULES];

/*============================================================================*/
/* PUBLIC FUNCTION PROTOTYPES */

uint32_t Pack_GetPackVoltage(Pack_t *pack);
float Pack_GetPackStateOfCharge(Pack_t *pack);

#endif // INC_PACK_H_
