/**
 * @file fsm.c
 * @brief Finite state machine for the ECU
 * 
 * @date 2020/11/28
 * @author Blake Shular (blake-shular)
 */

// TODO Have global variables for timer_check function calls --> maybe QUARTER_SECOND, SECOND, etc
// TODO Refactor to make function names consistent in capitalization conventions

/*============================================================================*/
/* FILE IMPORTS */

#include "main.h"
#include <stdbool.h>
#include "adc.h"
#include "can.h"
#include "fsm.h"


/*============================================================================*/
/* STATE MACHINE STATES */

typedef enum {
    FSM_RESET = 0,
    WAIT_FOR_BMS_POWERUP,
    WAIT_FOR_BMS_READY,
    PC_DCDC,
    DCDC_PLUS,
    DISABLE_MDU_DCH,
    CLOSE_NEG,
    WAIT_FOR_PC,
    LLIM_CLOSED,
    CHECK_HLIM,
    DASH_MCB_ON,
    MDU_ON,
    TELEM_ON,
    AMB_ON,
    MONITORING,
    FAULT
} FSM_state_t;


/*============================================================================*/
/* GLOBAL VARIABLES */

unsigned int last_tick;
FSM_state_t FSM_state;
bool LVS_power = false;
bool last_HLIM_status;
bool last_LLIM_status;

#define MESSAGE_INTERVAL 1000
#define LOW false
#define HIGH true


/*============================================================================*/
/* PRIVATE FUNCTION PROTOTYPES */

// State Functions:
void FSM_reset();
void BMS_powerup();
void BMS_ready();
void DCDC_Minus();
void DCDC_Plus();
void disable_MDU_DCH();
void close_NEG();
void pc_wait();
void LLIM_Closed();
void check_HLIM();
void DASH_MCB_On();
void MDU_On();
void TELEM_On();
void AMB_On();
void ECU_Monitor();
void fault();

// Helper Functions:
bool timer_check(unsigned int millis);


/*============================================================================*/
/* STATE TABLE */

void (*FSM_state_table[])(void) = {
    FSM_reset,
    BMS_powerup,
    BMS_ready,
    DCDC_Minus,
    DCDC_Plus,
    disable_MDU_DCH,
    close_NEG,
    pc_wait,
    LLIM_Closed,
    check_HLIM,
    DASH_MCB_On,
    MDU_On,
    TELEM_On,
    AMB_On,
    ECU_Monitor,
    fault
};


/*============================================================================*/
/* STATE MACHINE FUNCTIONS */

/**
 * @brief Initialization of FSM.
 */
void FSM_init() {
    last_tick = HAL_GetTick();
    FSM_state = RESET;
    return;
}

/**
 * @brief Main loop of the FSM. Will be called in main.c
 */
void FSM_run () {
    FSM_state_table[FSM_state]();
}

/**
 * @brief Runs at the start of every FSM run. Turns fans off, opens all contactors.
 * 
 * Exit Condition: V_Supp > 10.5 volts.
 * Exit Action: -
 * Exit State: WAIT_FOR_BMS_POWERUP
 * 
 * Exit Condition: V_Supp < 10.5 volts.
 * Exit Action: Turn on supp low LED.
 * Exit State: WAIT_FOR_BMS_POWERUP
 */
void FSM_reset () {
    // Turn fans off
    HAL_GPIO_WritePin(FAN1_CTRL_GPIO_Port, FAN1_CTRL_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(FAN2_CTRL_GPIO_Port, FAN2_CTRL_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(FAN3_CTRL_GPIO_Port, FAN3_CTRL_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MDUFAN_CTRL_GPIO_Port, MDUFAN_CTRL_Pin, GPIO_PIN_RESET);

    // Open all contactors
    HAL_GPIO_WritePin(HLIM_CTRL_GPIO_Port, HLIM_CTRL_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LLIM_CTRL_GPIO_Port, LLIM_CTRL_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(DCDC_M_CTRL_GPIO_Port, DCDC_M_CTRL_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(DCDC_P_CTRL_GPIO_Port, DCDC_P_CTRL_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(NEG_CTRL_GPIO_Port, NEG_CTRL_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(PC_CTRL_GPIO_Port, PC_CTRL_Pin, GPIO_PIN_RESET);

    // Read supplemental battery
    int supp_voltage = 0;
    if (ADC_getSuppBattVoltage(&supp_voltage) < 10500
        && HAL_GPIO_ReadPin(SUPP_LOW_GPIO_Port, SUPP_LOW_Pin) != GPIO_PIN_SET) {
        HAL_GPIO_WritePin(SUPP_LOW_GPIO_Port, SUPP_LOW_Pin, GPIO_PIN_SET);
    } else {
        HAL_GPIO_WritePin(SUPP_LOW_GPIO_Port, SUPP_LOW_Pin, GPIO_PIN_RESET);
    }

    FSM_state = WAIT_FOR_BMS_POWERUP;
    last_tick = HAL_GetTick();
    return;
}


/**
 * @brief Wait until the condition for BMS being powered on is met
 * 
 * Exit Condition: FLT High.
 * Exit Action: Start timer
 * Exit State: WAIT_FOR_BMS_READY
 */
void BMS_powerup () {
    if (timer_check(5000)) {
        FSM_state = FAULT;
    } else if (HAL_GPIO_ReadPin(FLT_BMS_GPIO_Port, FLT_BMS_Pin) == GPIO_PIN_SET) {
        last_tick = HAL_GetTick();
        FSM_state = WAIT_FOR_BMS_READY;
    }
    return;
}

/**
 * @brief Waits for the BMS to be in a ready-to-operate condition FLT pin going low means OK, if it doesn't within 5s, fault.
 * 
 * Exit Condition: FLT Low && timer < 5 seconds
 * Exit Action: Reset timer, Close DCDC-.
 * Exit State: PC_DCDC
 * 
 * Exit Condition: Timer surpasses 5 seconds.
 * Exit Action: Stop timer
 * Exit State: FAULT
 */
void BMS_ready () {
    if (timer_check(5000)) {
        FSM_state = FAULT;
    } else if (HAL_GPIO_ReadPin(FLT_BMS_GPIO_Port, FLT_BMS_Pin) == GPIO_PIN_RESET) {
        last_tick = HAL_GetTick();
        HAL_GPIO_WritePin(DCDC_M_CTRL_GPIO_Port, DCDC_M_CTRL_Pin, GPIO_PIN_SET);
        FSM_state = PC_DCDC;
    }
    return;
}

/**
 * @brief Waits for the DCDC- to fully pre-charge.
 * 
 * Exit Condition: Timer surpasses 0.2 seconds.
 * Exit Action: Reset timer, Close DCDC+.
 * Exit State: DCDC_PLUS
 */
void DCDC_Minus () {
    if (timer_check(200)) {
        HAL_GPIO_WritePin(DCDC_P_CTRL_GPIO_Port, DCDC_P_CTRL_Pin, GPIO_PIN_SET);
        FSM_state = DCDC_PLUS;
    }
    return;
}

/**
 * @brief After waiting for DCDC+ to turn on, turn on fans, switch to DCDC convertor, and enable MDU Discharge pin. 
 * 
 * Exit Condition: Timer surpasses 0.25 seconds.
 * Exit Action: Reset timer, enable battery fans, set MDU Discharge pin high.
 * Exit State: DISABLE_MDU_DCH
 */
void DCDC_Plus () {
    if (timer_check(250)) {
        HAL_GPIO_WritePin(SWAP_EN_GPIO_Port, SWAP_EN_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(FAN1_CTRL_GPIO_Port, FAN1_CTRL_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(FAN2_CTRL_GPIO_Port, FAN2_CTRL_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(FAN3_CTRL_GPIO_Port, FAN3_CTRL_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(DIST_RST_GPIO_Port, DIST_RST_Pin, GPIO_PIN_SET);
        FSM_state = DISABLE_MDU_DCH;
    }
    return;
}

/**
 * @brief Waits for the discharge resistor for the MDU to fully discharge.
 * 
 * Exit Condition: Timer surpasses 0.5 seconds.
 * Exit Action: Set MDU Discharge pin low, close negative contactor.
 * Exit State: CLOSE_NEG
 */
void disable_MDU_DCH () {
    if (timer_check(500)) {
        HAL_GPIO_WritePin(DIST_RST_GPIO_Port, DIST_RST_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(NEG_CTRL_GPIO_Port, NEG_CTRL_Pin, GPIO_PIN_SET);
        FSM_state = CLOSE_NEG;
    }
    return;
}

/**
 * @brief Checks if voltage level is too low through LLIM pin, decides whether to go through pre-charge sequence.
 * 
 * Exit Condition: LLIM Low.
 * Exit Action: Close Pre-charge contactor, start timer.
 * Exit State: WAIT_FOR_PC
 * 
 * Exit Condition: LLIM High.
 * Exit Action: -
 * Exit State CHECK_HLIM
 */
void close_NEG () {
    // Read LLIM voltage
    if (HAL_GPIO_ReadPin(LLIM_GPIO_Port, LLIM_Pin) == GPIO_PIN_RESET) {
        last_LLIM_status = LOW;
        HAL_GPIO_WritePin(PC_CTRL_GPIO_Port, PC_CTRL_Pin, GPIO_PIN_SET);
        last_tick = HAL_GetTick();
        FSM_state = WAIT_FOR_PC;
    } else if (HAL_GPIO_ReadPin(LLIM_GPIO_Port, LLIM_Pin) == GPIO_PIN_SET) {
        last_LLIM_status = HIGH;
        FSM_state = CHECK_HLIM;
    }
    return;
}

/**
 * @brief Waits for pre-charge to complete.
 * 
 * Exit Condition: Timer surpasses 0.35 seconds.
 * Exit Action: Reset timer, close LLIM.
 * Exit State: LLIM_CLOSED
 */
void pc_wait () {
    if (timer_check(350)) {
        HAL_GPIO_WritePin(LLIM_CTRL_GPIO_Port, LLIM_CTRL_Pin, GPIO_PIN_SET);
        FSM_state = LLIM_CLOSED;
    }
    return;
}

/**
 * @brief Waits for LLIM contactor to close, opens pre-charge contactor.
 * 
 * Exit Condition: Timer surpasses 0.25 seconds.
 * Exit Action: Open pre-charge contactor, reset timer.
 * Exit State: CHECK_HLIM
 */
void LLIM_Closed () {
    if (timer_check(250)) {
        HAL_GPIO_WritePin(PC_CTRL_GPIO_Port, PC_CTRL_Pin, GPIO_PIN_RESET);
        FSM_state = CHECK_HLIM;
    }
    return;
}

/**
 * @brief Checks if voltage level is too high through HLIM pin, decides whether to connect arrays.
 * 
 * Exit Condition: HLIM High.
 * Exit Action: -
 * Exit State: LVS_ON
 * 
 * Exit Condition: HLIM Low.
 * Exit Action: Close HLIM.
 * Exit State: LVS_ON
 */
void check_HLIM () {
    if (HAL_GPIO_ReadPin(HLIM_GPIO_Port, HLIM_Pin) == GPIO_PIN_SET) {
        last_HLIM_status = HIGH;
    } else if (HAL_GPIO_ReadPin(HLIM_GPIO_Port, HLIM_Pin) == GPIO_PIN_RESET) {
        last_HLIM_status = LOW;
        HAL_GPIO_WritePin(HLIM_CTRL_GPIO_Port, HLIM_CTRL_Pin, GPIO_PIN_SET);
    }

    if (LVS_power) {
        FSM_state = MONITORING;
    } else {
        FSM_state = DASH_MCB_ON;
    }
    return;
}

void DASH_MCB_On() {
    if(timer_check(200)) {
        // HAL_GPIO_WritePin(DASH_MCB_EN_GPIO_Port, DASH_MCB_EN_Pin, GPIO_PIN_SET);
    }
    FSM_state = MDU_ON;
    return;
}

void MDU_On() {
    if(timer_check(200)) {
        // HAL_GPIO_WritePin(MDU_EN_GPIO_Port, MDU_EN_Pin, GPIO_PIN_SET);
    }
    FSM_state = TELEM_ON;
    return;
}

void TELEM_On() {
    if(timer_check(200)) {
        // HAL_GPIO_WritePin(TEL_SPAR1_EN_GPIO_Port, TEL_SPAR1_EN_Pin, GPIO_PIN_SET);
    }
    FSM_state = AMB_ON;
    return;
}

void AMB_On() {
    if(timer_check(200)) {
        // HAL_GPIO_WritePin(AMB_SPAR2_EN_GPIO_Port, AMB_SPAR2_EN_Pin, GPIO_PIN_SET);
    }
    LVS_power = true;
    FSM_state = MONITORING;
    return;
}

/**
 * @brief Monitors ECU for any faults, monitors voltage levels.
 * 
 * Exit Condition: Fault
 * Exit Action: Go to fault?
 * Exit State: FAULT
 */
void ECU_Monitor () {
    // Check battery capacity
    if (HAL_GPIO_ReadPin(HLIM_GPIO_Port, HLIM_Pin) == GPIO_PIN_SET && last_HLIM_status == LOW) {
        HAL_GPIO_WritePin(HLIM_CTRL_GPIO_Port, HLIM_CTRL_Pin, GPIO_PIN_RESET);
        last_HLIM_status = HIGH;
    } else if (HAL_GPIO_ReadPin(HLIM_GPIO_Port, HLIM_Pin) == GPIO_PIN_RESET && last_HLIM_status == HIGH) {
        HAL_GPIO_WritePin(HLIM_CTRL_GPIO_Port, HLIM_CTRL_Pin, GPIO_PIN_SET);
        last_HLIM_status = LOW;
    } else if (HAL_GPIO_ReadPin(LLIM_GPIO_Port, LLIM_Pin) == GPIO_PIN_SET && last_LLIM_status == LOW) {
        HAL_GPIO_WritePin(LLIM_CTRL_GPIO_Port, LLIM_CTRL_Pin, GPIO_PIN_RESET);
        last_LLIM_status = HIGH;
    } else if (HAL_GPIO_ReadPin(LLIM_GPIO_Port, LLIM_Pin) == GPIO_PIN_RESET && last_LLIM_status == HIGH) {
        HAL_GPIO_WritePin(PC_CTRL_GPIO_Port, PC_CTRL_Pin, GPIO_PIN_SET);
        last_LLIM_status = LOW;
        last_tick = HAL_GetTick();
        FSM_state = WAIT_FOR_PC;
        return;
    }

    if (HAL_GPIO_ReadPin(FLT_BMS_GPIO_Port, FLT_BMS_Pin) == GPIO_PIN_SET
        || HAL_GPIO_ReadPin(COM_BMS_GPIO_Port, COM_BMS_Pin) == GPIO_PIN_SET
        || HAL_GPIO_ReadPin(OT_OUT_GPIO_Port, OT_OUT_Pin) == GPIO_PIN_SET
        || HAL_GPIO_ReadPin(ESTOP_5V_IN_GPIO_Port, ESTOP_5V_IN_Pin) == GPIO_PIN_SET) {
            FSM_state = FAULT;
            return;
    }

    // TODO read current, if too high or too low, set/pulse?? OC LATCH // OR do a reset in the init fcn

    // send CAN message with current values to BMS
    if (timer_check(MESSAGE_INTERVAL)) {    
            int motor_current = 0;
            int array_current = 0;
            ADC_getArrayCurrent(&array_current);
            ADC_getMotorCurrent(&motor_current);
            CAN_send_current(ADC_netCurrentOut(motor_current, array_current));
    }    
    
    // check supplemental battery voltage
    int supp_voltage = 0;
    if (ADC_getSuppBattVoltage(&supp_voltage) < 10500
        && HAL_GPIO_ReadPin(SUPP_LOW_GPIO_Port, SUPP_LOW_Pin) != GPIO_PIN_SET) {
        HAL_GPIO_WritePin(SUPP_LOW_GPIO_Port, SUPP_LOW_Pin, GPIO_PIN_SET);
    }
    return;
}

/**
 * @brief FAULT! OH NO!
 */
void fault() {
    // TODO send fault message via CAN
    // also send current message via CAN?
    
    // switch to SUPP
    HAL_GPIO_WritePin(SWAP_EN_GPIO_Port, SWAP_EN_Pin, GPIO_PIN_RESET);

    // blink fault light at 2Hz minimum
    if (timer_check(200)) {
        HAL_GPIO_TogglePin(FLT_OUT_GPIO_Port, FLT_OUT_Pin);
    }
    
    // MDU off
    // HAL_GPIO_WritePin(MDU_EN_GPIO_Port, MDU_EN_Pin, GPIO_PIN_RESET);

    // Open all contactors
    HAL_GPIO_WritePin(HLIM_CTRL_GPIO_Port, HLIM_CTRL_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LLIM_CTRL_GPIO_Port, LLIM_CTRL_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(DCDC_M_CTRL_GPIO_Port, DCDC_M_CTRL_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(DCDC_P_CTRL_GPIO_Port, DCDC_P_CTRL_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(NEG_CTRL_GPIO_Port, NEG_CTRL_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(PC_CTRL_GPIO_Port, PC_CTRL_Pin, GPIO_PIN_RESET);
    return;
}


/*============================================================================*/
/* HELPER FUNCTIONS */

/**
 * @brief returns true if we have passed the timer threshold
 */
bool timer_check(unsigned int millis) {
    unsigned int current_tick = HAL_GetTick();
    if (current_tick - last_tick >= millis) {
        last_tick = current_tick;
        return true;
    }
    return false;
}
