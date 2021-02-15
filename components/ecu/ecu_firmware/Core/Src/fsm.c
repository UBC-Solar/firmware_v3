/**
 * @file fsm.c
 * @brief Finite state machine for the ECU
 * 
 * @date 2020/11/28
 * @author Blake Shular (blake-shular)
 */


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
    LVS_ON,
    MONITORING,
    FAULT
} FSM_state_t;


/*============================================================================*/
/* GLOBAL VARIABLES */

unsigned int last_tick;
FSM_state_t FSM_state;

#define NUM_LVS_BOARDS 6


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
void LVS_On();
void ECU_Monitor();
void fault();

// Helper Functions
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
    LVS_On,
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

    ADC_supp_batt_adc = &hadc1;
    ADC_motor_adc = &hadc2;
    ADC_array_adc = &hadc3;
}

/**
 * @brief Main loop of the FSM. Will be called in main.c
 */
void FSM_run () {
    FSM_state_table[FSM_state]();
    //timer also here

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
    HAL_GPIO_WritePin(MDUFAN_CTRL_GPIO_Port, MDUFAN_CTRL_Pin, GPIO_PIN_RESET); // Is this necessary?

    // Open all contactors
    HAL_GPIO_WritePin(HLIM_CTRL_GPIO_Port, HLIM_CTRL_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LLIM_CTRL_GPIO_Port, LLIM_CTRL_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(DCDC_M_CTRL_GPIO_Port, DCDC_M_CTRL_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(DCDC_P_CTRL_GPIO_Port, DCDC_P_CTRL_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(NEG_CTRL_GPIO_Port, NEG_CTRL_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(PC_CTRL_GPIO_Port, PC_CTRL_Pin, GPIO_PIN_RESET);
    
    // Read supplemental battery voltage
    // Convert suppVoltage into a voltage level we can use.
    // if suppVoltage > 10.5V, state = WAITBMS
    // else supp low, state = WAITBMS

    FSM_state = WAIT_FOR_BMS_POWERUP;

    return;
}

// TODO Hash out the details of when BMS is considered 'ON', have a timeout here if FLT doesnt go high?
/**
 * @brief Wait until the condition for BMS being powered on is met
 * 
 * Exit Condition: FLT High.
 * Exit Action: Start timer
 * Exit State: WAIT_FOR_BMS_READY
 */
void BMS_powerup () {
    if (HAL_GPIO_ReadPin(FLT_BMS_GPIO_Port, FLT_BMS_Pin) == GPIO_PIN_SET) {
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
 * @brief 
 * 
 * Exit Condition: Timer surpasses 0.25 seconds.
 * Exit Action: Reset timer, enable battery fans, set MDU Discharge pin high.
 * Exit State: DISABLE_MDU_DCH
 */
void DCDC_Plus () {
    if (timer_check(250)) {
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
 * @brief Checks if voltage level is too low through LLIM pin, makes decision based on this.
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
        HAL_GPIO_WritePin(PC_CTRL_GPIO_Port, PC_CTRL_Pin, GPIO_PIN_SET);
        last_tick = HAL_GetTick();
        FSM_state = WAIT_FOR_PC;
    } else if (HAL_GPIO_ReadPin(LLIM_GPIO_Port, LLIM_Pin) == GPIO_PIN_SET) {
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
 * @brief 
 * 
 * Exit Condition: Timer surpasses 0.25 seconds.
 * Exit Action: Open pre-charge contactor, stop(reset?) timer.
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
 * @brief Checks if voltage level is too high through HLIM pin, makes decision based on this.
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
        FSM_state = LVS_ON;
    } else if (HAL_GPIO_ReadPin(HLIM_GPIO_Port, HLIM_Pin) == GPIO_PIN_RESET) {
        HAL_GPIO_WritePin(HLIM_CTRL_GPIO_Port, HLIM_CTRL_Pin, GPIO_PIN_SET);
        FSM_state = LVS_ON;
    }
    return;
}

/**
 * @brief Turns on all LVS boards. Uses a counter where each count is a particular board. Has delay of 0.2 seconds(?) between each board.
 * 
 * Exit Condition: Counter surpasses NUM_BOARDS.
 * Exit Action: -
 * Exit State: MONITOR
 */
void LVS_On () {
    for (int i = 0; i < NUM_LVS_BOARDS; i++) {
        if (i == 0) {
            // Turn on the first few boards
        } else if (i == 1) {
            // next
        } else if (i == 2) {
            
        } //etc...
        HAL_Delay(200);
    }

    // Change state
    
    return;
}

/**
 * @brief Monitors ECU for any faults, voltage levels.
 * 
 * Exit Condition: Fault
 * Exit Action: Go to fault?
 * Exit State: FAULT
 */
void ECU_Monitor () {
    // TODO Implement this
    
    // While (state != FAULT) 
        // Monitor
        // If (something goes wrong)
            // Change state to FAULT
    return;
}

/**
 * @brief FAULT! OH NO!
 */
void fault () {
    // TODO Implement this
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
