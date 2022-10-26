/**
 *  @file fsm.c
 *  @brief Main finite state machine for the BMS
 *
 *  @date 2020/05/02
 *  @author Andrew Hanlon (a2k-hanlon)
 *  @author Mischa Johal (mjohal67) - 2022/10/23
 */

#include "fsm.h"

#ifdef PRINTF_DEBUG
#include "stdio.h"
#endif

/*============================================================================*/
/* PRIVATE FUNCTION PROTOTYPES */

void analysisStatusUpdate(BTM_PackData_t *pack, int *status);
int doesRegGroupMatch(uint8_t reg_group1[BTM_NUM_DEVICES][BTM_REG_GROUP_SIZE],
                      uint8_t reg_group2[BTM_NUM_DEVICES][BTM_REG_GROUP_SIZE]);
void driveControlSignals(int status);

/*============================================================================*/
/* FSM GLOBAL VARIABLES */

FSM_state_t FSM_state;
unsigned int last_slave_communication; // must be updated after each command to slave board

/*============================================================================*/
/* FSM FUNCTIONS */

/**
 * @brief Initializes FSM.
 *
 * This must be called once, before calling FSM_run().
 */
void FSM_init(BTM_PackData_t *pack)
{
    pack->status = 0;
    FSM_state = FSM_RESET;
    return;
}

/**
 * @brief
 *
 * FSM entry point, must be called once.
 */
void FSM_run(BTM_PackData_t *pack)
{
    FSM_state_table[FSM_state](pack);
}

/**
 * @brief Initilizes hardware, pack datastructure, and preforms startup self checks.
 * Exit Condition: Faulty communication with slave
 * Exit Action: Write FLT_COMM status bit
 * Exit State: FSM_FAULT
 *
 * Exit Condition: Slave comms ok
 * Exit Action: None
 * Exit State: FSM_MEASURE
 *
 * @note This does not initialize the microcontroller. That must happen before running the
 * state machine.
 */
void FSM_reset(BTM_PackData_t *pack)
{
    uint8_t test_data[BTM_NUM_DEVICES][BTM_REG_GROUP_SIZE] = {{0x55, 0x6E, 0x69, 0x42, 0x43, 0x20}, {0x53, 0x6f, 0x6c, 0x61, 0x72, 0x21}};
    uint8_t test_data_rx[BTM_NUM_DEVICES][BTM_REG_GROUP_SIZE] = {0};
    BTM_Status_t comm_status = {BTM_OK, 0};
    int reg_group_match_failed = 0;

    // Initialize hardware and pack struct
    CONT_init();    // control signals
    BTM_init(pack); // initialize the LTC6813 and system data
    BTM_BAL_initDchPack(pack);
    // TODO: Initialize CAN

    // Perform startup system self checks
    BTM_writeRegisterGroup(CMD_WRCOMM, test_data);
    last_slave_communication = HAL_getTick();
    comm_status = BTM_readRegisterGroup(CMD_RDCOMM, test_data_rx);
    reg_group_match_failed = doesRegGroupMatch(test_data, test_data_rx);

    // checks for comms error
    if (reg_group_match_failed || comm_status.error & FLT_COMM_MASK)
    {
        pack->status |= FLT_COMM_MASK;
        FSM_state = FSM_FAULT;
        return;
    }

    FSM_state = FSM_MEASURE;
    return;
}

/**
 * @brief
 * Exit Condition:
 * Exit Action:
 * Exit State:
 *
 */

void FSM_measure(BTM_PackData_t *pack)
{
    unsigned int current_tick;
    BTM_Status_t comm_status = {BTM_OK, 0};
    unsigned int fan_PWM = 0;
    float max_temp = 0;
    int32_t pack_current;
    static uint8_t initial_soc_measurement = 1;

    current_tick = HAL_GetTick();
    if (last_slave_communication - current_tick >= FSM_MIN_UPDATE_INTERVAL)
    {

        /* TODO: rx ECU's CAN message, check DOC_COC bit
            DOC_COC, update status code, fault
            !DOC_COC, store current in pack_current variable
        */

        // voltage measurements
        comm_status = BTM_readBatt(pack);
        last_slave_communication = current_tick;

        // check for communications error
        if (comm_status.error != BTM_OK)
        {
            pack->status |= FLT_COMM_MASK;
            FSM_state = FSM_fault;
            return;
        }

        // temperature measurements
        comm_status = BTM_TEMP_measureState(pack);
        last_slave_communication = current_tick;

        // check for communications error
        if (comm_status.error != BTM_OK)
        {
            pack->status |= FLT_COMM_MASK;
            FSM_state = FSM_fault;
            return;
        }

        // update status code from measurements
        analysisStatusUpdate(pack);

        // check if any faults are active
        if (pack->status & FAULTS_MASK)
        {
            FSM_state = FSM_FAULT;
            return;
        }

        // SOC calculations
        // check if first time initilizing
        if (initial_soc_measurement)
        {
            SOC_allModulesInit(pack);
            initial_soc_measurement = 0;
        }
        else
        {
            SOC_allModulesEst(pack, pack_current, current_tick);
        }

        FSM_state = FSM_CTRL;
        return;
    }

    // TODO: what to do if it hasn't been 500ms yet? how to "wait" until it gets to 500ms?

    return;
}

void FSM_ctrl(BTM_PackData_t *pack)
{

    CONT_BAL_switch((pack->status & TRIP_BAL_MASK) ? CONT_ACTIVE : CONT_INACTIVE);
    CONT_HLIM_switch((pack->status & TRIP_HLIM_MASK) ? CONT_ACTIVE : CONT_INACTIVE);
    CONT_LLIM_switch((pack->status & TRIP_LLIM_MASK) ? CONT_ACTIVE : CONT_INACTIVE);

    // run balancing calculations and set balancing accordingly
    BTM_BAL_settings(pack);
    // drive fans, potentially switch state

    else if (system_status & BMS_TRIP_CHARGE_OT)
    {
        // any module is too hot to charge (not a fault)
        CONT_FAN_PWM_set(FAN_FULL);
    }
    else
    {
        // drive Fan PWM based on highest module temperature
        max_temp = ANA_findHighestModuleTemp(pack);
        fan_PWM = CONT_fanPwmFromTemp(max_temp);
        CONT_FAN_PWM_set(fan_PWM);
    }

    return;
}

void FSM_fault(BTM_PackData_t *pack)
{

    BTM_Status_t comm_status = {BTM_OK, 0};

    // set fault GPIOs
    CONT_FLT_switch((pack->status & FAULTS_MASK) ? CONT_ACTIVE : CONT_INACTIVE);
    CONT_COM_switch((pack->status & FLT_COMM_MASK) ? CONT_ACTIVE : CONT_INACTIVE);
    CONT_OT_switch((pack->status & FLT_OT_MASK) ? CONT_ACTIVE : CONT_INACTIVE);

    // set all fans full
    CONT_FAN_PWM_set(FAN_FULL);

    // Stop all balancing
    BTM_BAL_initDchPack(pack);
    BTM_BAL_setDischarge(pack);

    /*
   need to stay in fault state, but only need to do the above once
   BMS should continue to read pack and send data over CAN

   */

    // Perform measurements and update the system state no more frequently than
    // FSM_MIN_UPDATE_INTERVAL milliseconds
    current_tick = HAL_GetTick();
    if (last_update_tick - current_tick >= FSM_MIN_UPDATE_INTERVAL)
    {
        last_update_tick = current_tick;

        // perform measurements
        func_status = BTM_readBatt(pack);

        if (commsProblem(func_status, &system_status))
            return;
        func_status = BTM_TEMP_measureState(pack);

        if (commsProblem(func_status, &system_status))
            return;

        // analyze measurements, update system status code
        analysisStatusUpdate(pack, &system_status);
    }

    // TODO: perform CAN communication
    // this is not necessarily synchronous to the main system update
    // CANState();

    // fans should have been set at full power when the transition to this state took place
    // can ONLY switch from here to FAULT_COMM, or stay in this state
    return;
}

/*============================================================================*/
/* HELPER FUNCTIONS */

// TODO: update function implementation
void analysisStatusUpdate(BTM_PackData_t *pack)
{
    ANA_analyzeModules(pack);
    *status = (system_status & MASK_BMS_SYSTEM_FAULT);
    *status |= ANA_mergeModuleStatusCodes(pack);
    // ^ prevent system-level faults from being overwritten
    // Activate HLIM under CHARGE_OT condition
    if (*status & BMS_TRIP_CHARGE_OT)
    {
        *status |= BMS_TRIP_HLIM;
    }
    // note that TRIP_HLIM will be cleared by analysis.c functions next loop
    // if conditions change
}

// Helper function for initial system checks
// Returns 0 for full match, 1 otherwise
int doesRegGroupMatch(uint8_t reg_group1[BTM_NUM_DEVICES][BTM_REG_GROUP_SIZE],
                      uint8_t reg_group2[BTM_NUM_DEVICES][BTM_REG_GROUP_SIZE])
{
    for (int ic_num = 0; ic_num < BTM_NUM_DEVICES; ic_num++)
    {
        for (int i = 0; i < BTM_REG_GROUP_SIZE; i++)
        {
            if (reg_group1[ic_num][i] != reg_group2[ic_num][i])
                return 1;
        }
    }
    return 0;
}

void driveControlSignals(int status)
{
    CONT_FLT_switch((status & MASK_BMS_FAULT) ? CONT_ACTIVE : CONT_INACTIVE);
    CONT_BAL_switch((status & BMS_TRIP_BAL) ? CONT_ACTIVE : CONT_INACTIVE);
    CONT_COM_switch((status & BMS_FAULT_COMM) ? CONT_ACTIVE : CONT_INACTIVE);
    CONT_HLIM_switch((status & BMS_TRIP_HLIM) ? CONT_ACTIVE : CONT_INACTIVE);
    CONT_LLIM_switch((status & BMS_TRIP_LLIM) ? CONT_ACTIVE : CONT_INACTIVE);
    CONT_OT_switch((status & BMS_FAULT_OT) ? CONT_ACTIVE : CONT_INACTIVE);
    return;
}

void printMeasurements(BTM_PackData_t * pack)
{
    for (int ic_num = 0; ic_num < BTM_NUM_DEVICES; ic_num++)
    {
        printf("BOARD #%d\r\n", ic_num + 1);
        for (int module_num = 0; module_num < BTM_NUM_MODULES; module_num++)
        {
            printf("C%d\t", module_num + 1);
        }
        printf("\r\n");

        for (int module_num = 0; module_num < BTM_NUM_MODULES; module_num++) {
            if (pack->stack[ic_num].module[module_num].enable) {
                printf("%.4f\t", BTM_regValToVoltage(
                        pack->stack[ic_num].module[module_num].voltage));
            } else {
                printf("x\t"); // Don't print the voltage for inactive modules
            }
        }
        printf("\r\n");

        for (int module_num = 0; module_num < BTM_NUM_MODULES; module_num++) {
            if (pack->stack[ic_num].module[module_num].enable) {
                printf("%.3f\t", pack->stack[ic_num].module[module_num].temperature);
            } else {
                printf("x\t"); // Don't print the voltage for inactive modules
            }
        }
        printf("\r\n");
    }
    printf("\r\n");
}
