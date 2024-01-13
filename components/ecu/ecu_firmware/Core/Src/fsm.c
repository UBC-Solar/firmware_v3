/**
 * @file fsm.c
 * @brief Finite state machine for the ECU
 *
 * @date 2020/11/28
 * @author Blake Shular (blake-shular)
 */

/*============================================================================*/
/* FILE IMPORTS */

#include "fsm.h"
/**
 * @brief Initialization of FSM.
 */
void FSM_Init()
{
    last_tick = HAL_GetTick();
    FSM_state = FSM_RESET;
    return;
}

/**
 * @brief Main loop of the FSM. Will be called in main.c.
 */
void FSM_run()
{
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
void FSM_reset()
{
    // Turn fans off
    HAL_GPIO_WritePin(FAN1_CTRL_GPIO_Port, FAN1_CTRL_Pin, LOW);
    HAL_GPIO_WritePin(FAN2_CTRL_GPIO_Port, FAN2_CTRL_Pin, LOW);
    HAL_GPIO_WritePin(FAN3_CTRL_GPIO_Port, FAN3_CTRL_Pin, LOW);
    HAL_GPIO_WritePin(FAN4_CTRL_GPIO_Port, FAN4_CTRL_Pin, LOW);
    HAL_GPIO_WritePin(MDU_FAN_CTRL_GPIO_Port, MDU_FAN_CTRL_Pin, LOW);

    // Open all contactors
    HAL_GPIO_WritePin(HLIM_CTRL_GPIO_Port, HLIM_CTRL_Pin, LOW);
    HAL_GPIO_WritePin(LLIM_CTRL_GPIO_Port, LLIM_CTRL_Pin, LOW);
    HAL_GPIO_WritePin(DCDC_MINUS_CTRL_GPIO_Port, DCDC_MINUS_CTRL_Pin, LOW);
    HAL_GPIO_WritePin(DCDC_PLUS_CTRL_GPIO_Port, DCDC_PLUS_CTRL_Pin, LOW);
    HAL_GPIO_WritePin(NEG_CTRL_GPIO_Port, NEG_CTRL_Pin, LOW);
    HAL_GPIO_WritePin(PC_CTRL_GPIO_Port, PC_CTRL_Pin, LOW);

    // Read supplemental battery
    if (ecu_data.adc_data.ADC_supp_batt_volt < SUPP_LIMIT)
    {
        HAL_GPIO_WritePin(SUPP_LOW_GPIO_Port, SUPP_LOW_Pin, HIGH);
    }
    else
    {
        HAL_GPIO_WritePin(SUPP_LOW_GPIO_Port, SUPP_LOW_Pin, LOW);
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
 * 
 * Exit Condition: Timer surpasses 5 seconds.
 * Exit Action: -
 * Exit State: FAULT
 */
void BMS_powerup()
{
    if (timer_check(BMS_STARTUP_INTERVAL))
    {
        FSM_state = FAULT;
    }
    else if (HAL_GPIO_ReadPin(FLT_BMS_GPIO_Port, FLT_BMS_Pin) == HIGH)
    {
        last_tick = HAL_GetTick();
        FSM_state = WAIT_FOR_BMS_READY;
    }
    return;
}

/**
 * @brief Waits for the BMS to be in a ready-to-operate condition. FLT pin going low means OK, if it doesn't within 5s, fault.
 *
 * Exit Condition: FLT Low && timer < 5 seconds
 * Exit Action: Reset timer, Close DCDC-.
 * Exit State: PC_DCDC
 *
 * Exit Condition: Timer surpasses 5 seconds.
 * Exit Action: -
 * Exit State: FAULT
 */
void BMS_ready()
{
    if (timer_check(BMS_STARTUP_INTERVAL))
    {
        FSM_state = FAULT;
    }
    else if (HAL_GPIO_ReadPin(FLT_BMS_GPIO_Port, FLT_BMS_Pin) == LOW)
    {
        last_tick = HAL_GetTick();
        HAL_GPIO_WritePin(DCDC_MINUS_CTRL_GPIO_Port, DCDC_MINUS_CTRL_Pin, CONTACTOR_CLOSED);
        FSM_state = PC_DCDC;
    }
    return;
}

/**
 * @brief Waits for the DCDC- to fully pre-charge.
 *
 * Exit Condition: Timer surpasses 0.3 seconds.
 * Exit Action: Reset timer, Close DCDC+.
 * Exit State: DCDC_PLUS
 */
void DCDC_minus()
{
    if (timer_check(SHORT_INTERVAL))
    {
        HAL_GPIO_WritePin(DCDC_PLUS_CTRL_GPIO_Port, DCDC_PLUS_CTRL_Pin, CONTACTOR_CLOSED);
        FSM_state = DCDC_PLUS;
    }
    return;
}

/**
 * @brief After waiting for DCDC+ to turn on, turn on fans, switch to DCDC convertor, and reset discharge relay.
 *
 * Exit Condition: Timer surpasses 0.3 seconds.
 * Exit Action: Reset timer, enable battery fans, set discharge reset (DCH_RSTd) pin high.
 * Exit State: DISABLE_MDU_DCH
 */
void DCDC_plus()
{
    HAL_GPIO_WritePin(SWAP_CTRL_GPIO_Port, SWAP_CTRL_Pin, HIGH);
    HAL_GPIO_WritePin(FAN1_CTRL_GPIO_Port, FAN1_CTRL_Pin, HIGH);
    HAL_GPIO_WritePin(FAN2_CTRL_GPIO_Port, FAN2_CTRL_Pin, HIGH);
    HAL_GPIO_WritePin(FAN3_CTRL_GPIO_Port, FAN3_CTRL_Pin, HIGH);        
    HAL_GPIO_WritePin(FAN4_CTRL_GPIO_Port, FAN4_CTRL_Pin, HIGH);
    HAL_GPIO_WritePin(DCH_RST_GPIO_Port, DCH_RST_Pin, HIGH);
    FSM_state = DISABLE_MDU_DCH;
    return;
}

/**
 * @brief Waits for the discharge relay to fully reset.
 *
 * Exit Condition: Timer surpasses 0.3 seconds.
 * Exit Action: Set MDU Discharge pin low, close negative contactor.
 * Exit State: CHECK_LLIM;
 */
void disable_MDU_DCH()
{
    if (timer_check(SHORT_INTERVAL))
    {
        HAL_GPIO_WritePin(DCH_RST_GPIO_Port, DCH_RST_Pin, LOW);
        HAL_GPIO_WritePin(NEG_CTRL_GPIO_Port, NEG_CTRL_Pin, HIGH);
        FSM_state = CHECK_LLIM;
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
void check_LLIM()
{
    if (HAL_GPIO_ReadPin(LLIM_BMS_GPIO_Port, LLIM_BMS_Pin) == REQ_CONTACTOR_OPEN)
    {
        last_LLIM_status = CONTACTOR_OPEN;
        FSM_state = CHECK_HLIM;
    }
    else if (HAL_GPIO_ReadPin(LLIM_BMS_GPIO_Port, LLIM_BMS_Pin) == REQ_CONTACTOR_CLOSE)
    {
        HAL_GPIO_WritePin(PC_CTRL_GPIO_Port, PC_CTRL_Pin, CONTACTOR_CLOSED);
        last_tick = HAL_GetTick();
        FSM_state = WAIT_FOR_PC;
    }
    return;
}

/**
 * @brief Waits for motor pre-charge to complete.
 *
 * Exit Condition: Timer surpasses 0.705 seconds.
 * Exit Action: Reset timer, close LLIM.
 * Exit State: LLIM_CLOSED
 */
void PC_wait()
{
    if (timer_check(MDU_PC_INTERVAL))
    {
        HAL_GPIO_WritePin(LLIM_CTRL_GPIO_Port, LLIM_CTRL_Pin, CONTACTOR_CLOSED);
        last_LLIM_status = CONTACTOR_CLOSED;
        FSM_state = LLIM_CLOSED;
    }
    return;
}

/**
 * @brief Waits for LLIM contactor to close, opens pre-charge contactor.
 *
 * Exit Condition: Timer surpasses 0.3 seconds.
 * Exit Action: Open pre-charge contactor, reset timer.
 * Exit State: CHECK_HLIM
 */
void LLIM_closed()
{
    if (timer_check(SHORT_INTERVAL))
    {
        HAL_GPIO_WritePin(PC_CTRL_GPIO_Port, PC_CTRL_Pin, CONTACTOR_OPEN);
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
void check_HLIM()
{
    if (HAL_GPIO_ReadPin(HLIM_BMS_GPIO_Port, HLIM_BMS_Pin) == REQ_CONTACTOR_OPEN)
    {
        last_HLIM_status = CONTACTOR_OPEN;
    }
    else
    {
        HAL_GPIO_WritePin(HLIM_CTRL_GPIO_Port, HLIM_CTRL_Pin, CONTACTOR_CLOSED);
        last_HLIM_status = CONTACTOR_CLOSED;
    }

    if (LVS_ALREADY_ON == true)
    {
        FSM_state = MONITORING;
    }
    else
    {
        FSM_state = TELEM_ON;
    }
    return;
}

/**
 * @brief Turns on TELEM board.
 * Exit Condition: Timer surpasses 0.2 seconds.
 * Exit Action: Set TEL_CTRL pin high.
 * Exit State: SPAR1_ON
 */
void TELEM_on()
{
    if (timer_check(LVS_INTERVAL))
    {
        HAL_GPIO_WritePin(TEL_CTRL_GPIO_Port, TEL_CTRL_Pin, HIGH);
        FSM_state = DASH_ON;
    }
    return;
}

/**
 * @brief Turns on DASH board.
 *
 * Exit Condition: Timer surpasses 0.2 seconds.
 * Exit Action: Set DASH pin high.
 * Exit State: MCB_ON
 */
void DASH_on()
{
    if (timer_check(LVS_INTERVAL))
    {
        HAL_GPIO_WritePin(DID_CTRL_GPIO_Port, DID_CTRL_Pin, HIGH);
        FSM_state = MCB_ON;
    }
    return;
}

/**
 * @brief Turns on MCB board.
 * 
 * Exit Condition: Timer surpasses 0.2 seconds.
 * Exit Action: Set MCB pin high.
 * Exit State: MDU_ON
 */
void MCB_on()
{
    if (timer_check(LVS_INTERVAL))
    {
        HAL_GPIO_WritePin(MCB_CTRL_GPIO_Port, MCB_CTRL_Pin, HIGH);
        FSM_state = MDU_ON;
    }
    return;
}

/**
 * @brief Turns on MDU board.
 * Exit Condition: Timer surpasses 0.2 seconds.
 * Exit Action: Set MDU pin high.
 * Exit State: TELEM_ON
 */
void MDU_on()
{
    if (timer_check(LVS_INTERVAL))
    {
        HAL_GPIO_WritePin(MDI_CTRL_GPIO_Port, MDI_CTRL_Pin, HIGH);
        FSM_state = SPAR1_ON;
    }
    return;
}

/**
 * @brief Turns on SPAR1 pin.
 * Exit Condition: Timer surpasses 0.2 seconds.
 * Exit Action: Set SPAR1_CTRL pin high.
 * Exit State: AMB_ON
 */
void SPAR1_on()
{
    if (timer_check(LVS_INTERVAL))
    {
        HAL_GPIO_WritePin(SPAR1_CTRL_GPIO_Port, SPAR1_CTRL_Pin, HIGH);
        FSM_state = AMB_ON;
    }
    return;
}

/**
 * @brief Turns on AMB board and SPAR2 pin.
 * Exit Condition: Timer surpasses 0.2 seconds.
 * Exit Action: Set AMB/SPAR2 pin high.
 * Exit State: MONITORING
 */
void AMB_on()
{
    if (timer_check(LVS_INTERVAL))
    {
        HAL_GPIO_WritePin(AMB_CTRL_GPIO_Port, AMB_CTRL_Pin, HIGH);
        LVS_ALREADY_ON = true;
        FSM_state = MONITORING;
    }
    return;
}

/**
 * @brief Monitors ECU for any faults, monitors voltage levels.
 *
 * Exit Condition: Fault
 * Exit Action: Go to fault? Go to fault.
 * Exit State: FAULT
 */
void ECU_monitor()
{
    /*************************
    Current Fault Checking
    **************************/
    if (ecu_data.adc_data.ADC_batt_current >= COC_THRESHOLD || ecu_data.adc_data.ADC_batt_current <= DOC_THRESHOLD)
    {
        FSM_state = FAULT;
        return;
    }

    //TODO ADC DOC_COC check

    /*************************
    Other Fault Checking
    **************************/
    if (HAL_GPIO_ReadPin(FLT_BMS_GPIO_Port, FLT_BMS_Pin) == HIGH || \
        HAL_GPIO_ReadPin(COM_BMS_GPIO_Port, COM_BMS_Pin) == HIGH || \
        HAL_GPIO_ReadPin(OT_BMS_GPIO_Port, OT_BMS_Pin) == HIGH || \
        HAL_GPIO_ReadPin(ESTOP_5V_GPIO_Port, ESTOP_5V_Pin) == LOW)
    {
        FSM_state = FAULT;
        return;
    }

    /*************************
    Check Battery Capacity
    **************************/
    if (HAL_GPIO_ReadPin(HLIM_BMS_GPIO_Port, HLIM_BMS_Pin) == REQ_CONTACTOR_OPEN && last_HLIM_status == CONTACTOR_CLOSED)
    {
        HAL_GPIO_WritePin(HLIM_CTRL_GPIO_Port, HLIM_CTRL_Pin, CONTACTOR_OPEN);
        last_HLIM_status = CONTACTOR_OPEN;
    }
    else if (HAL_GPIO_ReadPin(HLIM_BMS_GPIO_Port, HLIM_BMS_Pin) == REQ_CONTACTOR_CLOSE && last_HLIM_status == CONTACTOR_OPEN)
    {
        HAL_GPIO_WritePin(HLIM_CTRL_GPIO_Port, HLIM_CTRL_Pin, CONTACTOR_CLOSED);
        last_HLIM_status = CONTACTOR_CLOSED;
    }
    else if (HAL_GPIO_ReadPin(LLIM_BMS_GPIO_Port, LLIM_BMS_Pin) == REQ_CONTACTOR_CLOSE && last_LLIM_status == CONTACTOR_OPEN)
    {
        HAL_GPIO_WritePin(PC_CTRL_GPIO_Port, PC_CTRL_Pin, CONTACTOR_CLOSED);
        last_tick = HAL_GetTick();
        FSM_state = WAIT_FOR_PC;
        return;
    }
    else if (HAL_GPIO_ReadPin(LLIM_BMS_GPIO_Port, LLIM_BMS_Pin) == REQ_CONTACTOR_OPEN && last_LLIM_status == CONTACTOR_CLOSED)
    {
        HAL_GPIO_WritePin(PC_CTRL_GPIO_Port, PC_CTRL_Pin, CONTACTOR_OPEN);
        last_LLIM_status = CONTACTOR_OPEN;
    }


    /*************************
    Send CAN Messages
    **************************/
    if (timer_check(MESSAGE_INTERVAL_0X3F4))
    {
        CAN_SendMessage3F4();
    }

    if (timer_check(MESSAGE_INTERVAL_0X450))
    {
        CAN_SendMessage450();
    }

    /*************************
    Check SUPP Voltage
    **************************/
    if (ecu_data.adc_data.ADC_supp_batt_volt < SUPP_LIMIT)
    {
        HAL_GPIO_WritePin(SUPP_LOW_GPIO_Port, SUPP_LOW_Pin, HIGH);
    }
    else
    {
        HAL_GPIO_WritePin(SUPP_LOW_GPIO_Port, SUPP_LOW_Pin, LOW);
    }
    return;
}

/**
 * @brief Fault state. Occurs when there is any fault (COM, BMS, ESTOP, OT, OC, OD).
 * Sends CAN message describing fault, switches to supplementa battery, flashes FLT light at 2Hz,
 * opens all contactors, turns off MDU.
 */
void fault()
{
    // send CAN message with current values with BMS
    if (timer_check(MESSAGE_INTERVAL_0X3F4))
    {
        CAN_SendMessage3F4();
    }

    if (timer_check(MESSAGE_INTERVAL_0X450))
    {
        CAN_SendMessage450();
    }

    // switch to SUPP
    HAL_GPIO_WritePin(SWAP_CTRL_GPIO_Port, SWAP_CTRL_Pin, LOW);

    // blink fault light at 2Hz minimum
    if (timer_check(FLT_BLINK_INTERVAL))
    {
        HAL_GPIO_TogglePin(FLT_OUT_GPIO_Port, FLT_OUT_Pin);
    }

    // MDU off
    HAL_GPIO_WritePin(MDI_CTRL_GPIO_Port, MDI_CTRL_Pin, LOW);

    // Open all contactors
    HAL_GPIO_WritePin(HLIM_CTRL_GPIO_Port, HLIM_CTRL_Pin, CONTACTOR_OPEN);
    HAL_GPIO_WritePin(LLIM_CTRL_GPIO_Port, LLIM_CTRL_Pin, CONTACTOR_OPEN);
    HAL_GPIO_WritePin(DCDC_MINUS_CTRL_GPIO_Port, DCDC_MINUS_CTRL_Pin, CONTACTOR_OPEN);
    HAL_GPIO_WritePin(DCDC_PLUS_CTRL_GPIO_Port, DCDC_PLUS_CTRL_Pin, CONTACTOR_OPEN);
    HAL_GPIO_WritePin(NEG_CTRL_GPIO_Port, NEG_CTRL_Pin, CONTACTOR_OPEN);
    HAL_GPIO_WritePin(PC_CTRL_GPIO_Port, PC_CTRL_Pin, CONTACTOR_OPEN);
    return;
}

/*============================================================================*/
/* HELPER FUNCTIONS */

/**
 * @brief returns true if we have passed the timer threshold
 */
bool timer_check(unsigned int millis)
{
    unsigned int current_tick = HAL_GetTick();
    if (current_tick - last_tick >= millis)
    {
        last_tick = current_tick;
        return true;
    }
    return false;
}
