/**
 * @file bms_main.c
 * @brief BMS top-level functions
 *
 * @date June 10, 2023
 * @author Mischa Johal
 * @author Andrew Hanlon
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h> // TODO: remove
#include "stm32f1xx_hal.h"
#include "bms_main.h"
#include "pack.h"
#include "ltc6813_btm.h"
#include "ltc6813_btm_temp.h"
#include "ltc6813_btm_bal.h"
#include "analysis.h"
#include "balancing.h"
#include "can.h"
#include "control.h"
#include "soc.h"
#include "selftest.h"

/*============================================================================*/
/* PRIVATE FUNCTION DEFINITIONS */

/**
 * @brief Helper function for initial system checks
 * Returns true for full match, false otherwise
 */
static bool doesRegGroupMatch(uint8_t reg_group1[BTM_NUM_DEVICES][BTM_REG_GROUP_SIZE],
                              uint8_t reg_group2[BTM_NUM_DEVICES][BTM_REG_GROUP_SIZE])
{
    for (int ic_num = 0; ic_num < BTM_NUM_DEVICES; ic_num++)
    {
        for (int i = 0; i < BTM_REG_GROUP_SIZE; i++)
        {
            if (reg_group1[ic_num][i] != reg_group2[ic_num][i])
                return false;
        }
    }
    return true;
}

static void processSelfTestStatus(Pack_t *pack, BTM_Status_t *self_test_status)
{
    if (self_test_status->error == BTM_ERROR_SELFTEST)
    {
        pack->status.bits.fault_self_test = true;
    }
    else if (self_test_status->error != BTM_OK)
    {
        pack->status.bits.fault_communications = true;
    }
}

static void stopBalancing(Pack_t *pack)
{
    // Disable balancing of all modules
    bool discharge_setting[PACK_NUM_BATTERY_MODULES] = {false};
    BTM_BAL_setDischarge(pack, discharge_setting); // writes balancing commands for all modules
}

static void clearModuleData(Pack_t *pack)
{
    // Zero out data since valid data could not be collected
    for (uint32_t module_num = 0; module_num < PACK_NUM_BATTERY_MODULES; module_num++)
    {
        pack->module[module_num].voltage = 0;
        pack->module[module_num].temperature = 0.0f;
        pack->module[module_num].state_of_charge = 0.0f;
    }
}

/*============================================================================*/
/* PUBLIC FUNCTION DEFINITIONS */

/**
 * @brief Perform initial system checks
 *
 * Writes COMM bit in status code if checks fail
 */
void BMS_MAIN_startupChecks(Pack_t *pack)
{
    uint8_t test_data[BTM_NUM_DEVICES][BTM_REG_GROUP_SIZE] = {
        {0x55, 0x6E, 0x69, 0x42, 0x43, 0x20},
        {0x53, 0x6f, 0x6c, 0x61, 0x72, 0x21}};
    uint8_t test_data_rx[BTM_NUM_DEVICES][BTM_REG_GROUP_SIZE] = {0};
    BTM_Status_t comm_status = {BTM_OK, 0};
    BTM_Status_t ltc_temp_status = {BTM_OK, 0};
    BTM_Status_t ltc_vref2_status = {BTM_OK, 0};
    BTM_Status_t shorted_cells_status = {BTM_OK, 0};
    BTM_Status_t open_wire_status = {BTM_OK, 0};
    BTM_Status_t overlap_measurement_status = {BTM_OK, 0};
    // BTM_Status_t discharge_test_status = {BTM_OK, 0};
    bool reg_group_match;

    BTM_writeRegisterGroup(CMD_WRCOMM, test_data);
    comm_status = BTM_readRegisterGroup(CMD_RDCOMM, test_data_rx);
    reg_group_match = doesRegGroupMatch(test_data, test_data_rx);

    printf("reg group match: %d\r\n", reg_group_match);

    // checks for comms error
    // note: a lack of comms is different than the self-tests failing
    if (comm_status.error != BTM_OK)
    {
        pack->status.bits.fault_communications = true;
        return; // Don't continue; if there is a communication problem, cannot perform other tests
    }
    if (!reg_group_match)
    {
        pack->status.bits.fault_self_test = true;
    }

    // ltc_temp_status = ST_checkLTCtemp();
    // processSelfTestStatus(pack, &ltc_temp_status);

    // ltc_vref2_status = ST_checkVREF2();
    // processSelfTestStatus(pack, &ltc_vref2_status);

    // shorted_cells_status = ST_shortedCells();
    // processSelfTestStatus(pack, &shorted_cells_status);

    // open_wire_status = ST_checkOpenWire();
    // processSelfTestStatus(pack, &open_wire_status);

    // overlap_measurement_status = ST_checkOverlapVoltage();
    // processSelfTestStatus(pack, &overlap_measurement_status);

    // TODO: check if this works once BMS is connected to real batteries
    // It works based on voltage drop due to resistance in the voltage tap leads when discharge is on... not sure if
    // that will be sufficient to produce measureable difference in voltage between discharge on and discharge off
    // discharge_test_status = ST_verifyDischarge();
    // processSelfTestStatus(pack, &discharge_test_status);

    if (PACK_ANY_FAULTS_SET(pack->status))
    {
        printf("Startup failed\r\n");
    }
    else
    {
        printf("Startup passed\r\n");
    }
}

void BMS_MAIN_updatePackData(Pack_t *pack)
{
    // ECU CAN Message
    int8_t pack_current = 0;
    uint8_t low_voltage_current = 0;
    bool overcurrent_detected = 0;
    uint32_t ecu_can_rx_timestamp = 0;
    bool new_ecu_can_message_received;
    // Other
    static bool soc_initialized = false;

    // Check for new ECU CAN message
    new_ecu_can_message_received = CAN_GetMessage0x450Data(&pack_current, &low_voltage_current, &overcurrent_detected, &ecu_can_rx_timestamp);

    if (new_ecu_can_message_received && overcurrent_detected)
    {
        pack->status.bits.fault_over_current = true; // set FLT_DOC_COC bit
    }

    pack->status.bits.warning_no_ecu_message = !new_ecu_can_message_received;

    // TODO: isolation sensor check (if it's BMS's responsibilty)

    // get pack measurements
    BTM_wakeup(); // With 1s update rate, LTC6813 watchdog won't time out but its isoSPI interface does, so need wakeup
    if (BTM_getVoltages(pack).error != BTM_OK)
    {
        pack->status.bits.fault_communications = true;
        pack->status.bits.warning_regen_off = true;
        clearModuleData(pack);
    }
    else if (BTM_TEMP_getTemperatures(pack).error != BTM_OK)
    {
        pack->status.bits.fault_communications = true;
        pack->status.bits.warning_regen_off = true;
        clearModuleData(pack);
    }
    else
    {
        // SOC estimation
        if (!soc_initialized) // only preform initialization of SOC once (at startup)
        {
            SOC_allModulesInit(pack);
            soc_initialized = true;
        }
        else if (new_ecu_can_message_received)
        {
            SOC_allModulesEst(pack, pack_current, ecu_can_rx_timestamp);
        }

        // write pack status code
        ANA_analyzePack(pack);
    }
}

/**
 * @brief Update GPIO control signal outputs, balancing state, and fan pwm
 *
 * @param[in] pack Pack data to align output states to
 */
void BMS_MAIN_driveOutputs(Pack_t *pack)
{
    uint32_t fan_PWM = 0;
    float max_temp = 0;

    // if any fault active, drive FLT, COM, OT GPIOs, turn off balancing, drive fans 100%
    if (PACK_ANY_FAULTS_SET(pack->status))
    {
        CONT_FLT_switch(true);
        CONT_COM_switch(pack->status.bits.fault_communications);
        CONT_OT_switch(pack->status.bits.fault_over_temperature);
        stopBalancing(pack);
        CONT_BAL_switch(pack->status.bits.balancing_active);
        CONT_FAN_PWM_set(FAN_FULL);
    }
    else // no fault; balance modules, drive control signals and fans
    {
        BAL_updateBalancing(pack); // write bal settings, send bal commands
        CONT_BAL_switch(pack->status.bits.balancing_active);

        // HLIM active if TRIP_HLIM or TRIP_CHARGE_OT are active
        CONT_HLIM_switch(pack->status.bits.hlim || pack->status.bits.charge_over_temperature_limit);
        CONT_LLIM_switch(pack->status.bits.llim);

        // set fans
        if (pack->status.bits.charge_over_temperature_limit) // if TRIP_CHARGE_OT active, drive fans 100%
        {
            fan_PWM = FAN_FULL;
        }
        else // otherwise, fans set proportional to temperature
        {
            max_temp = ANA_findHighestModuleTemp(pack);
            fan_PWM = CONT_fanPwmFromTemp(max_temp);
        }
        CONT_FAN_PWM_set(fan_PWM);
    }
}

/**
 * @brief Send all of the BMS CAN messages with the most current pack data
 *
 * @param[in] pack Pack data used to populate message data
 */
void BMS_MAIN_sendCanMessages(Pack_t *pack)
{
    CAN_SendMessage622(pack);
    CAN_SendMessage623(pack);
    CAN_SendMessage624(pack);
    CAN_SendMessage625(pack);
    CAN_SendMessages626(pack);
    CAN_SendMessages627(pack);
    CAN_SendMessages628(pack);
    CAN_SendMessages629(pack);
}
