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

/*============================================================================*/
/* PRIVATE FUNCTION PROTOTYPES */

bool timer_check(uint32_t interval, uint32_t *last_tick);
void check_supp_voltage(void);

/*============================================================================*/
/* PRIVATE TYPE DEFS */
typedef struct{

    uint32_t last_generic_tick;
    uint32_t last_tick_fault_led;
    uint32_t neg_tick;
    uint32_t pos_tick;
    
} hv_ticks_t;

/*============================================================================*/
/* PRIVATE VARIABLES */
static hv_ticks_t ticks;
static volatile FSM_state_t FSM_state;
static bool LVS_ALREADY_ON = false;
static bool startup_complete = false; // set true when first reach monitoring state
static bool last_HLIM_status;
static bool last_LLIM_status;
static volatile uint8_t count_awdg_faults = 0;

/*============================================================================*/
/* PRIVATE FUNCTION IMPLEMENTATIONS */

/**
 * @brief Initialization of FSM.
 */
void FSM_Init()
{
    uint32_t reset_flags = RCC->CSR;

    if (reset_flags & RCC_CSR_IWDGRSTF) {
        //IWDG triggered
        printf("watchdog-triggered software reset \r\n");
        ecu_data.status.bits.reset_from_watchdog = 1; //CAN_message now knows watchdog event has occured
        FSM_state = FAULT;
    }
    else {
        FSM_state = FSM_RESET;
    }

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
    printf("start of FSM reset\r\n");
    //  Turn fans off
    HAL_GPIO_WritePin(PACK_FANS_CTRL_GPIO_Port, PACK_FANS_CTRL_Pin, LOW);
    HAL_GPIO_WritePin(MDU_FAN_CTRL_GPIO_Port, MDU_FAN_CTRL_Pin, LOW);

    // Open all contactors
    HAL_GPIO_WritePin(HLIM_CTRL_GPIO_Port, HLIM_CTRL_Pin, LOW);
    HAL_GPIO_WritePin(LLIM_CTRL_GPIO_Port, LLIM_CTRL_Pin, LOW);
    HAL_GPIO_WritePin(POS_CTRL_GPIO_Port, POS_CTRL_Pin, LOW);
    HAL_GPIO_WritePin(NEG_CTRL_GPIO_Port, NEG_CTRL_Pin, LOW);
    HAL_GPIO_WritePin(PC_CTRL_GPIO_Port, PC_CTRL_Pin, LOW);

    // Read supplemental battery
    check_supp_voltage();

    FSM_state = WAIT_FOR_BMS_POWERUP;

    ticks.last_generic_tick = HAL_GetTick();
    printf("end of FSM reset \r\n");
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
    printf("start of BMS_powerup\r\n");
    if (timer_check(BMS_STARTUP_INTERVAL, & (ticks.last_generic_tick) ))
    {
        FSM_state = FAULT;
    }
    else if (HAL_GPIO_ReadPin(FLT_BMS_GPIO_Port, FLT_BMS_Pin) == HIGH)
    {
        ticks.last_generic_tick = HAL_GetTick();
        FSM_state = WAIT_FOR_BMS_READY;
    }

    return;
}

/**
 * @brief Waits for the BMS to be in a ready-to-operate condition. FLT pin going low means OK, if it doesn't within 5s, fault.
 *
 * Exit Condition: FLT Low && timer < 5 seconds
 * Exit Action: Reset timer
 * Exit State: HV_Connect
 *
 * Exit Condition: Timer surpasses 5 seconds.
 * Exit Action: -
 * Exit State: FAULT
 */
void BMS_ready()
{
    printf("beginning of BMS ready\r\n");
    if (timer_check(BMS_STARTUP_INTERVAL, &(ticks.last_generic_tick) ))
    {
        FSM_state = FAULT;
    }
    else if (HAL_GPIO_ReadPin(FLT_BMS_GPIO_Port, FLT_BMS_Pin) == LOW)
    {
        ticks.last_generic_tick = HAL_GetTick();
        FSM_state = HV_CONNECT;
    }
    printf("end of BMS ready\r\n");

    return;
}

/**
 * @brief Closes NEG and POS contactors.
 *
 * Exit Condition: Timer surpasses 0.3 seconds.
 * Exit Action: Close NEG and POS contactors.
 * Exit State: swap_DCDC
 */
void HV_Connect()
{
    static bool first_delay_tick = false;
    static bool second_delay_tick = false; 
    
    printf("beginning of HV connect\r\n");
    
    // delay before closing contactors
    if (timer_check(SHORT_INTERVAL, &(ticks.last_generic_tick) ) && first_delay_tick == false)
    {
        first_delay_tick = true;
        ticks.neg_tick = HAL_GetTick(); 
    }

    if(first_delay_tick == true)
    {
        // close NEG and delay
        HAL_GPIO_WritePin(NEG_CTRL_GPIO_Port, NEG_CTRL_Pin, CONTACTOR_CLOSED);

        if(timer_check(SHORT_INTERVAL, &(ticks.neg_tick) ) && second_delay_tick == false) 
        {
            // close POS
            HAL_GPIO_WritePin(POS_CTRL_GPIO_Port, POS_CTRL_Pin, CONTACTOR_CLOSED);
            second_delay_tick = true;
            ticks.pos_tick = HAL_GetTick();
        }

        if(second_delay_tick == true)
        {
            // delay to allow everything to settle
            if( timer_check(SHORT_INTERVAL, &(ticks.pos_tick))) 
            {
                ticks.last_generic_tick = HAL_GetTick();
                FSM_state = SWAP_DCDC;
            }
        }
    }

    return;
}

/**
 * @brief After HV is connected, swap 12V supply to DCDC converter, turn on fans, reset discharge relay.
 *
 * Exit Condition: None
 * Exit Action: Reset timer, enable battery fans, set discharge reset (DCH_RSTd) pin high.
 * Exit State: DISABLE_MDU_DCH
 */
void swap_DCDC()
{
    
    printf("swap DCDC\r\n");
    
    HAL_GPIO_WritePin(SWAP_CTRL_GPIO_Port, SWAP_CTRL_Pin, HIGH);
    HAL_GPIO_WritePin(PACK_FANS_CTRL_GPIO_Port, PACK_FANS_CTRL_Pin, HIGH);
    HAL_GPIO_WritePin(DCH_RST_GPIO_Port, DCH_RST_Pin, HIGH);

    ticks.last_generic_tick = HAL_GetTick();
    FSM_state = DISABLE_MDU_DCH;

    return;
}

/**
 * @brief Waits for the discharge relay to fully reset.
 *
 * Exit Condition: Timer surpasses 0.3 seconds.
 * Exit Action: Set MDU Discharge pin low (reset discharge).
 * Exit State: CHECK_LLIM;
 */
void disable_MDU_DCH()
{
    if (timer_check(SHORT_INTERVAL, &(ticks.last_generic_tick) ))
    {
        HAL_GPIO_WritePin(DCH_RST_GPIO_Port, DCH_RST_Pin, LOW);
        ticks.last_generic_tick = HAL_GetTick();
        FSM_state = CHECK_LLIM;
    }
    printf("end of MDU dch\r\n");

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
        ticks.last_generic_tick = HAL_GetTick();
        FSM_state = WAIT_FOR_PC;
    }

    printf("end of check LLIM\r\n");

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
    if (timer_check(MDU_PC_INTERVAL, &(ticks.last_generic_tick) ))
    {
        HAL_GPIO_WritePin(LLIM_CTRL_GPIO_Port, LLIM_CTRL_Pin, CONTACTOR_CLOSED);
        last_LLIM_status = CONTACTOR_CLOSED;
        ticks.last_generic_tick = HAL_GetTick();
        FSM_state = LLIM_CLOSED;
    }
    printf("end of PC wait\r\n");

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
    if (timer_check(SHORT_INTERVAL, &(ticks.last_generic_tick) ))
    {
        HAL_GPIO_WritePin(PC_CTRL_GPIO_Port, PC_CTRL_Pin, CONTACTOR_OPEN);
        ticks.last_generic_tick = HAL_GetTick();
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
 * Exit State: TELEM_ON
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
        ticks.last_generic_tick = HAL_GetTick();
        FSM_state = MONITORING;
    }
    else
    {
        ticks.last_generic_tick = HAL_GetTick();
        FSM_state = TELEM_ON;
    }

    printf("end of check HLIM\r\n");
    return;
}

/**
 * @brief Turns on TELEM board.
 * Exit Condition: Timer surpasses LVS_INTERVAL.
 * Exit Action: Set TEL_CTRL pin high.
 * Exit State: MEM_ON
 */
void TELEM_on()
{
    if (timer_check(LVS_INTERVAL, &(ticks.last_generic_tick) ))
    {
        HAL_GPIO_WritePin(TEL_CTRL_GPIO_Port, TEL_CTRL_Pin, HIGH);
        ticks.last_generic_tick = HAL_GetTick();
        FSM_state = MEM_ON;
    }

    printf("end of TELEM on\r\n");
    return;
}

/**
 * @brief Turns on MEMORATOR (SPAR on ECU, VDS on DIST).
 * Exit Condition: Timer surpasses LVS_INTERVAL.
 * Exit Action: Set SPAR_CTRL pin high.
 * Exit State: DASH_ON
 */
void MEM_on()
{
    if (timer_check(LVS_INTERVAL, &(ticks.last_generic_tick) ))
    {
        HAL_GPIO_WritePin(SPAR1_CTRL_GPIO_Port, SPAR1_CTRL_Pin, HIGH);
        ticks.last_generic_tick = HAL_GetTick();
        FSM_state = DASH_ON;
    }

    printf("end of MEM on\r\n");
    return;
}

/**
 * @brief Turns on DASH board.
 *
 * Exit Condition: Timer surpasses LVS_INTERVAL.
 * Exit Action: Set DASH pin high.
 * Exit State: MCB_ON
 */
void DASH_on()
{
    if (timer_check(LVS_INTERVAL, &(ticks.last_generic_tick) ))
    {
        HAL_GPIO_WritePin(DID_CTRL_GPIO_Port, DID_CTRL_Pin, HIGH);
        ticks.last_generic_tick = HAL_GetTick();
        FSM_state = MCB_ON;
    }

    printf("end of DASH on\r\n");
    return;
}

/**
 * @brief Turns on MCB board.
 *
 * Exit Condition: Timer surpasses LVS_INTERVAL.
 * Exit Action: Set MCB pin high.
 * Exit State: MDU_ON
 */
void MCB_on()
{
    if (timer_check(LVS_INTERVAL, &(ticks.last_generic_tick) ))
    {
        HAL_GPIO_WritePin(MCB_CTRL_GPIO_Port, MCB_CTRL_Pin, HIGH);
        ticks.last_generic_tick = HAL_GetTick();
        FSM_state = MDU_ON;
    }

    printf("end of MCB on\r\n");

    return;
}

/**
 * @brief Turns on MDU board.
 * Exit Condition: Timer surpasses LVS_INTERVAL.
 * Exit Action: Set MDU pin high.
 * Exit State: AMB_ON
 */
void MDU_on()
{
    // Don't turn on MDU if ESTOP pressed
    if(ecu_data.status.bits.estop == true){
        FSM_state = AMB_ON;
        return;
    }

    if (timer_check(LVS_INTERVAL, &(ticks.last_generic_tick) ))
    {
        HAL_GPIO_WritePin(MDU_CTRL_GPIO_Port, MDU_CTRL_Pin, HIGH);
        ticks.last_generic_tick = HAL_GetTick();
        FSM_state = AMB_ON;
    }

    printf("end of MDU on\r\n");
    return;
}

/**
 * @brief Turns on AMB board and SPAR2 pin.
 * Exit Condition: Timer surpasses LVS_INTERVAL.
 * Exit Action: Set AMB/SPAR2 pin high.
 * Exit State: MONITORING
 */
void AMB_on()
{

    if (timer_check(LVS_INTERVAL, &(ticks.last_generic_tick) ))
    {
        HAL_GPIO_WritePin(AMB_CTRL_GPIO_Port, AMB_CTRL_Pin, HIGH);
        LVS_ALREADY_ON = true;
        ticks.last_generic_tick = HAL_GetTick();
        FSM_state = MONITORING;
    }

    printf("end of AMB on \r\n");
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

    printf("Monitoring Start\r\n");
    startup_complete = true; // Indicates all LV boards are up

    // Additional ESTOP check to catch case where ESTOP is pressed during startup (see note in fault state)
    if(ecu_data.status.bits.estop == true){
        FSM_state = FAULT;
        return;
    }
    
    // Current Status Checks
    if(ecu_data.adc_data.ADC_pack_current >= DOC_WARNING_THRESHOLD){
        ecu_data.status.bits.warning_pack_overdischarge_current = true;
        ecu_data.status.bits.warning_pack_overcharge_current = false;
    }
    else if(ecu_data.adc_data.ADC_pack_current <= COC_WARNING_THRESHOLD){
        ecu_data.status.bits.warning_pack_overdischarge_current = false;
        ecu_data.status.bits.warning_pack_overcharge_current = true;
    }
    else{
        ecu_data.status.bits.warning_pack_overdischarge_current = false;
        ecu_data.status.bits.warning_pack_overcharge_current = false;
    }

    /*************************
    BMS Fault Checking
    **************************/
    if (HAL_GPIO_ReadPin(FLT_BMS_GPIO_Port, FLT_BMS_Pin) == HIGH && HAL_GPIO_ReadPin(BAL_BMS_GPIO_Port, BAL_BMS_Pin) == LOW)
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
        ticks.last_generic_tick = HAL_GetTick();
        FSM_state = WAIT_FOR_PC;
        return;
    }
    else if (HAL_GPIO_ReadPin(LLIM_BMS_GPIO_Port, LLIM_BMS_Pin) == REQ_CONTACTOR_OPEN && last_LLIM_status == CONTACTOR_CLOSED)
    {
        HAL_GPIO_WritePin(LLIM_CTRL_GPIO_Port, LLIM_CTRL_Pin, CONTACTOR_OPEN);
        last_LLIM_status = CONTACTOR_OPEN;
    }

    /*************************
    Send CAN Messages
    **************************/
    //Retrieve received messages
    CAN_CheckRxMessages(CAN_RX_FIFO0);

    // Only send charger message if it is connected
    if (CAN_CheckRxChargerMessage())
    {
        CAN_SendMessage1806E5F4();
    }

    if (timer_check(MESSAGE_INTERVAL_0X450, &(ticks.last_generic_tick) ))
    {
        CAN_SendMessage450();
    }

    /*************************
    Check SUPP Voltage
    **************************/
    check_supp_voltage();

    return;
}

/**
 * @brief Fault state. Occurs when there is any fault (COM, BMS, ESTOP, OT, OC, OD).
 * Sends CAN message describing fault, switches to supplementa battery, flashes FLT light at 2Hz,
 * opens all contactors, turns off MDU.
 */
void fault()
{
    printf("Fault Start\r\n");
    /*************************
    Put Pack in Safe State
    **************************/
    HAL_GPIO_WritePin(SWAP_CTRL_GPIO_Port, SWAP_CTRL_Pin, LOW);
    HAL_GPIO_WritePin(HLIM_CTRL_GPIO_Port, HLIM_CTRL_Pin, CONTACTOR_OPEN);
    HAL_GPIO_WritePin(LLIM_CTRL_GPIO_Port, LLIM_CTRL_Pin, CONTACTOR_OPEN);
    HAL_GPIO_WritePin(POS_CTRL_GPIO_Port, POS_CTRL_Pin, CONTACTOR_OPEN);
    HAL_GPIO_WritePin(NEG_CTRL_GPIO_Port, NEG_CTRL_Pin, CONTACTOR_OPEN);
    HAL_GPIO_WritePin(PC_CTRL_GPIO_Port, PC_CTRL_Pin, CONTACTOR_OPEN);

    // If ESTOP pressed during startup, start all LV boards
    if(!startup_complete){
        FSM_state = TELEM_ON;
        return;
    }

    HAL_GPIO_WritePin(MDU_CTRL_GPIO_Port, MDU_CTRL_Pin, LOW);


    /****************************
    Preform Perpetual Fault Tasks
    *****************************/
    check_supp_voltage();

    // send CAN status message

    if (timer_check(MESSAGE_INTERVAL_0X450, &(ticks.last_generic_tick) ))
    {
        CAN_SendMessage450();
    }

    // blink fault light at 2Hz minimum (see regs)
    if (timer_check(FLT_BLINK_INTERVAL, &(ticks.last_tick_fault_led) ))
    {
        HAL_GPIO_TogglePin(FLT_OUT_GPIO_Port, FLT_OUT_Pin);
    }

    return;
}
 
/*============================================================================*/
/* INTERRUPT CALLBACK FUNCTIONS */

// Windowed AWDG Fault Check, triggered on TIM2 interrupt
void FSM_ADC_WindowedAWDGCallback(){
    
    if(count_awdg_faults >= AWDG_TRIP_THRESHOLD){
        ecu_data.status.bits.fault_charge_overcurrent = true;
        ecu_data.status.bits.fault_discharge_overcurrent = true;

        HAL_GPIO_WritePin(DOC_COC_LED_GPIO_Port, DOC_COC_LED_Pin, HIGH);

        FSM_state = FAULT;
        FSM_run(); // Immediately transition to fault state
    }

    count_awdg_faults = 0;
}

// Analog Watchdog (AWDG) Callback
void FSM_ADC_LevelOutOfWindowCallback()
{
    count_awdg_faults++;
}

void FSM_ESTOPActivedCallback()
{
    ecu_data.status.bits.estop = true;
    HAL_GPIO_WritePin(ESTOP_LED_GPIO_Port, ESTOP_LED_Pin, HIGH);
    
    FSM_state = FAULT;
    FSM_run(); // Immediately transition to fault state
}

/*============================================================================*/
/* HELPER FUNCTIONS */

/**
 * @brief returns true if we have passed the timer interval
 */
bool timer_check(uint32_t interval, uint32_t *last_tick)
{
    uint32_t current_tick = HAL_GetTick();
    if (current_tick - *last_tick >= interval)
    {
        *last_tick = current_tick;
        return true;
    }
    return false;
}

void check_supp_voltage(void)
{
    if (ecu_data.adc_data.ADC_supp_batt_volt < SUPP_LIMIT)
    {
        HAL_GPIO_WritePin(SUPP_LOW_GPIO_Port, SUPP_LOW_Pin, HIGH);
    }
    else
    {
        HAL_GPIO_WritePin(SUPP_LOW_GPIO_Port, SUPP_LOW_Pin, LOW);
    }
}
