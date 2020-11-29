/**
 * @file fsm.c
 * @brief Finite state machine for the ECU
 * 
 * @date 2020/11/28
 * @author Blake Shular (blake-shular)
 */






// There is probably an FSM initialization function that goes here.

/**
 * @brief Main loop of the FSM.
 */
void FSM_run () {
    //FSM State table here
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
    // TODO Implement this
    return;
}

/**
 * @brief Wait until the condition for BMS being powered on is met // TODO Hash out the details of when BMS is considered 'ON'
 * 
 * Exit Condition: FLT High.
 * Exit Action: Start timer
 * Exit State: WAIT_FOR_BMS_READY
 */
void BMS_powerup () {
    // TODO Implement this
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
    // TODO Implement this
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
    // TODO Implement this
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
    // TODO Implement this
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
    // TODO Implement this
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
    // TODO Implement this
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
    // TODO Implement this
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
    // TODO Implement this
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
    // @TODO Implement this
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
    // TODO Implement this
    return;
}

/**
 * @brief Monitors ECU for any faults, voltage levels.
 * 
 * Exit Condition: Fault // TODO Figure out the details here
 * Exit Action: Go to fault?
 * Exit State: FAULT
 */
void ECU_Monitor () {
    // TODO Implement this
    return;
}

/**
 * @brief FAULT! OH NO!
 */
void fault () {
    // TODO Implement this
    return;
}
