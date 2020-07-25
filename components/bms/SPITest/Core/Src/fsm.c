/**
 *  @file fsm.c
 *  @brief Main finite state machine for the BMS
 *
 *  @date 2020/05/02
 *  @author Andrew Hanlon (a2k-hanlon)
 */

/*
 * Note: Probably the most common implementation of an FSM in C uses
 * a switch statement with a case for each state. This is perfectly
 * valid, but that's not what is done here.
 * What's done here uses a function pointer array (FSM_state_table)
 * along with a function for every state. For longer and more complicated
 * FSMs, this style of FSM is beneficial as it separates the states
 * more distinctly.
 */

// Comment out the line below to disable printf debugging
#define PRINTF_DEBUG

#include "fsm.h"
#ifdef PRINTF_DEBUG
#include "stdio.h"
#endif

// Private function prototypes
void FSM_reset(BTM_PackData_t * pack, BTM_BAL_dch_setting_pack_t* dch_setting_pack);
void FSM_normal(BTM_PackData_t * pack, BTM_BAL_dch_setting_pack_t* dch_setting_pack);
void FSM_fault_comm(BTM_PackData_t * pack, BTM_BAL_dch_setting_pack_t* dch_setting_pack);
void FSM_fault_general(BTM_PackData_t * pack, BTM_BAL_dch_setting_pack_t* dch_setting_pack);
// Helper functions:
int commsProblem(BTM_Status_t func_status, int * sys_status);
int doesRegGroupMatch(uint8_t reg_group1[BTM_NUM_DEVICES][BTM_REG_GROUP_SIZE],
                      uint8_t reg_group2[BTM_NUM_DEVICES][BTM_REG_GROUP_SIZE]);
void driveControlSignals(int status);
#ifdef PRINTF_DEBUG
void printBTMStatus(BTM_Status_t status, int print_ok);
#endif

// Function pointer array to BMS FSM state functions
// Function names go in this array declaration, and must be in the same order
// as the FSM_BMS_state_t enumeration.
void (* FSM_state_table[])(BTM_PackData_t * pack, BTM_BAL_dch_setting_pack_t* dch_setting_pack)
    = {FSM_reset, FSM_normal, FSM_fault_comm, FSM_fault_general};

// FSM Global Variables
FSM_state_t FSM_state;
unsigned int last_uptime_tick; // for keeping track of uptime
int system_status; // status code for system


/**
 * @brief Initializes FSM state.
 *
 * This must be called once, before calling FSM_run().
 */
void FSM_init()
{
    last_uptime_tick = HAL_GetTick();
    system_status = 0;
    FSM_state = FSM_RESET;
#ifdef PRINTF_DEBUG
    printf("FSM initialized\n");
#endif
}


// This function should be placed in the main firmware loop
void FSM_run(BTM_PackData_t * pack, BTM_BAL_dch_setting_pack_t* dch_setting_pack)
{
    unsigned int current_tick;

    FSM_state_table[FSM_state](pack, dch_setting_pack);

    // Update uptime
    current_tick = HAL_GetTick();
    if (current_tick - last_uptime_tick >= 1000) {
        uptime++;
    }
}


/**
 * @brief Performs initialization of BMS hardware and initial system checks.
 *
 * This does not initialize the microcontroller. That must happen before running the
 * state machine.
 * If communication with all slave boards cannot be established, goes to BMS_FAULT_COMM
 * If communication can be established but any self test fails or a measured value is
 * outside the relevant safe range.
 */
void FSM_reset(BTM_PackData_t * pack, BTM_BAL_dch_setting_pack_t* dch_setting_pack)
{
    uint8_t test_data[BTM_NUM_DEVICES][BTM_REG_GROUP_SIZE]
        = {{0x55, 0x6E, 0x69, 0x42, 0x43, 0x20}/*, {0x53, 0x6f, 0x6c, 0x61, 0x72, 0x21}*/};
    uint8_t test_data_rx[BTM_NUM_DEVICES][BTM_REG_GROUP_SIZE] = {0};
    BTM_Status_t func_status = {BTM_OK, 0};
    int reg_group_match_failed = 0;

    uptime = 0; // Reset uptime

    // Initialize control signals
    CONT_init();
    // initialize the LTC6811 and system data
    BTM_init(pack);
    BTM_BAL_initDchPack(dch_setting_pack);
    // TODO: initialize CAN

    // initialize other stuff?

#ifdef PRINTF_DEBUG
    printf("BTM system initialized\nChecking communication with LTC devices: ");
#endif

    // Perform startup system self checks
    BTM_writeRegisterGroup(CMD_WRCOMM, test_data);
    func_status = BTM_readRegisterGroup(CMD_RDCOMM, test_data_rx);
    reg_group_match_failed = doesRegGroupMatch(test_data, test_data_rx);
#ifdef PRINTF_DEBUG
    printBTMStatus(func_status, 1);
    putchar('\n');
#endif
    if (reg_group_match_failed)
    {
        system_status |= BMS_FAULT_COMM;
#ifdef PRINTF_DEBUG
        printf("Comms check data match FAILED\n");
#endif
    }
    if (commsProblem(func_status, &system_status)) return;


    // TODO: more self tests...

    if (commsProblem(func_status, &system_status)) return;

    // perform initial voltage measurement
    func_status = BTM_readBatt(pack);
#ifdef PRINTF_DEBUG
    printf("Performing initial measurements\nVolt: ");
    printBTMStatus(func_status, 1);
    putchar(' ');
#endif
    if (commsProblem(func_status, &system_status)) return;

    // perform initial temperature measurement
    func_status = BTM_TEMP_measureState(pack);
#ifdef PRINTF_DEBUG
    printf("\nTemp: ");
    printBTMStatus(func_status, 1);
    putchar('\n');
#endif
    if (commsProblem(func_status, &system_status)) return;

    // analyze initial measurements
    ANA_analyzeModules(pack);
    system_status = (system_status & MASK_BMS_SYSTEM_FAULT) | ANA_mergeModuleStatusCodes(pack);
    // ^ prevent system-level faults from being overwritten

    driveControlSignals(system_status);

    // switch state
    if (system_status & MASK_BMS_FAULT) // If any faults are active
    {
#ifdef PRINTF_DEBUG
        printf("ENTERING GENERAL FAULT STATE\n");
#endif
        CONT_FAN_PWM_set(FAN_FULL);
        FSM_state = FSM_FAULT_GENERAL;
    }
    else
    {
#ifdef PRINTF_DEBUG
        printf("Entering NORMAL state\n");
#endif
        FSM_state = FSM_NORMAL;
    }

    return;
}

void FSM_normal(BTM_PackData_t * pack, BTM_BAL_dch_setting_pack_t* dch_setting_pack) {
    unsigned int fan_PWM = 0;
    float max_temp = 0;

    // perform measurements

    // check for communication fault

    // analyze measurements, update system status code

    // drive control signals based on status code
    driveControlSignals(system_status);

    // drive Fan PWM based on temperature
    max_temp = ANA_findHighestModuleTemp(pack);
    fan_PWM = CONT_fanPwmFromTemp(max_temp);
    CONT_FAN_PWM_set(fan_PWM);
    // run balancing calculations and set balancing

    // perform CAN communication

    return;
}

void FSM_fault_comm(BTM_PackData_t * pack, BTM_BAL_dch_setting_pack_t* dch_setting_pack) {
    // do not attempt any more slave-board communication

    // drive fans at full power

    // send status-only CAN messages (no data)?

    return;
}

void FSM_fault_general(BTM_PackData_t * pack, BTM_BAL_dch_setting_pack_t* dch_setting_pack) {
    // perform measurements

    // check for communication fault

    // analyze measurements, update system status code

    // if new faults come up, drive control signals;
    // drive fans at full power

    // If there is an HLIM fault, **and** no SHORT or OT fault, continue running balancing to
    // try to alleviate the problem

    // send CAN messages

    // can switch from here to FAULT_COMM, or stay in this state ONLY

    return;
}

/*
 * State functions should follow this format:
 * void FSM_<state name>()
 * {
 *      <Things to do in this state no matter what>
 *
 *      if(<state change condition>)
 *      {
 *          <Things to do on state transition>
 *          FSM_state = <state to go to on next loop>;
 *      }
 *      else if (<other state change condition>)
 *      {
 *          <Things to do on state transition>
 *          FSM_state = <state to go to on next loop>;
 *      }
 *      // staying in the same state otherwise can be implicit.
 * }
 */

// Checks for a communication fault condition, switches FSM to FAULT_COMM state if necessary
// Returns 1 if there is a communication fault, 0 otherwise
int commsProblem(BTM_Status_t func_status, int * sys_status) {
    if (func_status.error != BTM_OK || (*sys_status & BMS_FAULT_COMM))
    {
#ifdef PRINTF_DEBUG
        printf("ENTERING COMM FAULT STATE\n");
#endif
        *sys_status |= BMS_FAULT_COMM;
        driveControlSignals(*sys_status); // Update control signals
        CONT_FAN_PWM_set(FAN_FULL); // Drive fans at full power
        FSM_state = FSM_FAULT_COMM;
        return 1;
    }

    return 0;
}

// Helper function for initial system checks
// Returns 0 for full match, 1 otherwise
int doesRegGroupMatch(uint8_t reg_group1[BTM_NUM_DEVICES][BTM_REG_GROUP_SIZE],
                      uint8_t reg_group2[BTM_NUM_DEVICES][BTM_REG_GROUP_SIZE])
{
    for(int ic_num = 0; ic_num < BTM_NUM_DEVICES; ic_num++) {
        for(int i = 0; i < BTM_REG_GROUP_SIZE; i++) {
            if (reg_group1[ic_num][i] != reg_group2[ic_num][i])
                return 1;
        }
    }
    return 0;
}

void driveControlSignals(int status) {
    CONT_FLT_switch( (status & MASK_BMS_FAULT) ? CONT_ACTIVE : CONT_INACTIVE);
    CONT_BAL_switch( (status & BMS_TRIP_BAL)   ? CONT_ACTIVE : CONT_INACTIVE);
    CONT_COM_switch( (status & BMS_FAULT_COMM) ? CONT_ACTIVE : CONT_INACTIVE);
    CONT_HLIM_switch((status & BMS_TRIP_HLIM)  ? CONT_ACTIVE : CONT_INACTIVE);
    CONT_LLIM_switch((status & BMS_TRIP_LLIM)  ? CONT_ACTIVE : CONT_INACTIVE);
    CONT_OT_switch(  (status & BMS_FAULT_OT)   ? CONT_ACTIVE : CONT_INACTIVE);
    return;
}

#ifdef PRINTF_DEBUG
void printBTMStatus(BTM_Status_t status, int print_ok)
{
    switch (status.error) {
    case BTM_OK:
        if (print_ok) printf("BTM OK");
        break;
    case BTM_ERROR_PEC:
        printf("BTM PEC error");
        break;
    case BTM_ERROR_TIMEOUT:
        printf("BTM timeout error");
        break;
    case BTM_ERROR_HAL:
        printf("BTM general HAL error");
        break;
    case BTM_ERROR_HAL_BUSY:
        printf("BTM HAL busy error");
        break;
    case BTM_ERROR_HAL_TIMEOUT:
        printf("BTM HAL timeout error");
        break;
    default:
        printf("Unknown BTM error");
        break;
    }

    if (status.device_num != 0)
    {
        printf("at LTC device #%d", status.device_num);
    }
    return;
}
#endif
