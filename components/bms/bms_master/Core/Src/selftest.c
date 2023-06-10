/**
 * @file selftest.c
 * @brief BMS startup self tests
 *
 * @date 2020/10/03
 * @author matthewjegan
 */

#include "selftest.h"
#include <math.h>

/*============================================================================*/
/* DEFINITION */

#define SC_CELLS 2              // Pins C18 and C12 on LTC6813-1 Slave Board should be shorted.
#define EXPECTED_SC_VOLTAGE 0.0 // Voltage @ shorted pins should be 0
#define SC_REGS 2               // Voltages at pins C12 and C18 are stored in Registers D and F respectively

#define OVERLAP_TEST_REGS 2 // Number of registers overlap voltage is read from.
                            // One register to compare ADC 1 and ADC 2 (Group C),
                            // and another to compare ADC 2 and ADC 3 (Group E).

#define PDOWN_REPS 2 // Number of times CMD_ADOW_PDOWN command is called in ST_checkOpenWire,
                     // LTC-6813 data sheet recommends >= 2 repetitions
#define PUP_REPS 2

#define NUM_TEST_CELLS 2 // Overlap Voltage test reads cells 7 and 13

#define OVERLAP_READINGS_PER_REG 2   // A voltage reading from each ADC is stored in the same register
#define OVERLAP_READINGS_PER_BOARD 4 // 2 bytes combine to represent a single voltage reading

#define OPEN_WIRE_VOLTAGE -0.400 // If the difference between PUP and PDOWN voltage measurements
                                 // is less than -400 mV, there is an open wire at the measured cell.

#define VREF_LOWERBOUND 2.990 // Establishes range of acceptable voltages for VREF2 measurement.
#define VREF_UPPERBOUND 3.014 // (specified on p.30 of LTC6813-1 datasheet)

#define DISCHARGE_VERIFICATION_NUM_PATTERNS 7U
#define DISCHARGE_VERIFICATION_ALL_OFF_PATTERN_INDEX 0U

#define ADCV_DCP_MASK (0x1U << 4) // "Discharge permitted" option bit of LTC6813 ADCV command

/*============================================================================*/
/* PRIVATE DATA */

static bool dischargeVerificationPatterns[DISCHARGE_VERIFICATION_NUM_PATTERNS][PACK_NUM_BATTERY_MODULES] = {
    // All off
    {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
     false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false},
    // Discharge LTC6813 channels 1, 7, 13 (array skips unused channels)
    {true,  false, false, false, false, false, true,  false, false, false, false, true,  false, false, false, false,
     true,  false, false, false, false, false, true,  false, false, false, false, true,  false, false, false, false},
    // Discharge LTC6813 channels 2, 8, 14 (array skips unused channels)
    {false, true,  false, false, false, false, false, true,  false, false, false, false, true,  false, false, false,
     false, true,  false, false, false, false, false, true,  false, false, false, false, true,  false, false, false},
    // Discharge LTC6813 channels 3, 9, 15 (array skips unused channels)
    {false, false, true,  false, false, false, false, false, true,  false, false, false, false, true,  false, false,
     false, false, true,  false, false, false, false, false, true,  false, false, false, false, true,  false, false},
    // Discharge LTC6813 channels 4, 10, 16 (array skips unused channels)
    {false, false, false, true,  false, false, false, false, false, true,  false, false, false, false, true,  false,
     false, false, false, true,  false, false, false, false, false, true,  false, false, false, false, true,  false},
    // Discharge LTC6813 channels 5, 11, 17 (array skips unused channels)
    {false, false, false, false, true,  false, false, false, false, false, true,  false, false, false, false, true,
     false, false, false, false, true,  false, false, false, false, false, true,  false, false, false, false, true },
    // Discharge LTC6813 channel 6; 12 and 18 are unused (array skips unused channels)
    {false, false, false, false, false, true,  false, false, false, false, false, false, false, false, false, false,
     false, false, false, false, false, true,  false, false, false, false, false, false, false, false, false, false},
};

/*============================================================================*/
/* PRIVATE FUNCTION PROTOTYPES */

static BTM_Status_t readAllRegisters(uint8_t ADC_data[NUM_CELL_VOLT_REGS][BTM_NUM_DEVICES][BTM_REG_GROUP_SIZE], float moduleVoltage[BTM_NUM_DEVICES][BTM_NUM_CELL_INPUTS_PER_DEVICE]);

// Testable private functions
#ifndef TEST
STATIC_TESTABLE void itmpConversion(uint16_t ITMP[], float temp_celsius[]);
#endif // TEST

/*============================================================================*/
/* PUBLIC FUNCTION DEFINITIONS */

/**
 * @brief Checks internal die temperature of LTC6813's for safe operating condition
 *
 * @return If at least one LTC6813 has a die temperature nearing thermal shutdown
 * 		  threshold, returns an error with the device index of the first overheating IC.
 **/
BTM_Status_t ST_checkLTCtemp()
{
    BTM_Status_t status = {BTM_OK, 0};
    uint8_t registerSTATA[BTM_NUM_DEVICES][BTM_REG_GROUP_SIZE];
    uint16_t itmp[BTM_NUM_DEVICES];
    float temp_celsius[BTM_NUM_DEVICES];

    status = BTM_sendCmdAndPoll(CMD_ADSTAT_ITMP);
    if (status.error != BTM_OK)
        return status;

    // Retrieve register reading
    status = BTM_readRegisterGroup(CMD_RDSTATA, registerSTATA);
    if (status.error != BTM_OK)
        return status;

    for (int board = 0; board < BTM_NUM_DEVICES; board++)
    {
        // Combine 2 bytes of die temperature reading
        itmp[board] = (((uint16_t)registerSTATA[board][1]) << 8) | registerSTATA[board][0];
    }

    itmpConversion(itmp, temp_celsius);

    for (int board = 0; board < BTM_NUM_DEVICES; board++)
    {
        if (temp_celsius[board] >= ST_LTC_TEMPLIMIT)
        {
            status.error = BTM_ERROR_SELFTEST;
            status.device_num = board + 1;
        }
    }

    return status;
}

/**
 * @brief Measures independent reference voltage VREF2 to verify measurement of ADC1 (LTC6813-1 p.30).
 * 		  Readings outside the range 2.990V to 3.014V indicate the system is out of its specified tolerance.
 * 		  Accuracy of ADC2 measurement is verified separately using ST_checkOverlapVoltage() command.
 *
 * @return OK if reading is within tolerance range. BTM_ERROR_SELFTEST if ADC1 measurement is outside
 * 		   tolerance range.
 */
BTM_Status_t ST_checkVREF2()
{
    uint8_t registerAUXB[BTM_NUM_DEVICES][BTM_REG_GROUP_SIZE];
    uint16_t cell_voltage_raw = 0;
    float converted_voltage = 0;

    BTM_Status_t status = {BTM_OK, 0};

    status = BTM_sendCmdAndPoll(CMD_ADAX_VREF2);
    if (status.error != BTM_OK)
        return status;

    status = BTM_readRegisterGroup(CMD_RDAUXB, registerAUXB);
    if (status.error != BTM_OK)
        return status;

    // Each cell voltage is provided as a 16-bit value where
    // voltage = 0.0001V * raw value
    // Each 6-byte Cell Voltage Register Group holds 3 cell voltages
    // Last 2 bytes of Auxiliary Voltage Register Group B is VREF2
    for (int board = 0; board < BTM_NUM_DEVICES; board++)
    {
        // Combine the 2 bytes of each cell voltage together
        cell_voltage_raw =
            ((uint16_t)(registerAUXB[board][5]) << 8) | (uint16_t)(registerAUXB[board][4]);

        // Convert to volts and store
        converted_voltage = BTM_regValToVoltage(cell_voltage_raw);

        if (converted_voltage < VREF_LOWERBOUND ||
            converted_voltage > VREF_UPPERBOUND)
        {
            status.error = BTM_ERROR_SELFTEST;
            status.device_num = board + 1;
            return status;
        }
    }

    return status;
}

/**
 * @brief Verifies that pin connections C12 and C18 are not measuring any voltage as neither
 * 		  pin will be connected to an active cell in the Battery Pack.
 *
 * @return OK if a voltage of 0 +/- 0.003 V is measured. BTM_ERROR_SELFTEST if the measured
 * 		   cell voltage is outside of this range.
 */
BTM_Status_t ST_shortedCells()
{
    // 2x 6-byte sets (each from a different register group of the LTC6813) for each LTC6813
    uint8_t ADC_data[SC_REGS][BTM_NUM_DEVICES][BTM_REG_GROUP_SIZE];
    BTM_Status_t status = {BTM_OK, 0};

    uint16_t cell_voltage_raw = 0;
    float converted_voltage = 0;

    status = BTM_sendCmdAndPoll(CMD_ADCV);
    if (status.error != BTM_OK)
        return status;

    status = BTM_readRegisterGroup(CMD_RDCVD, ADC_data[0]);
    if (status.error != BTM_OK)
        return status;

    status = BTM_readRegisterGroup(CMD_RDCVF, ADC_data[1]);
    if (status.error != BTM_OK)
        return status;

    for (int ic_num = 0; ic_num < BTM_NUM_DEVICES; ic_num++)
    {
        for (int reg_group = 0; reg_group < SC_REGS; reg_group++)
        {
            int reading_num = 3; // C12 and C18 are the 3rd voltage value stored
                                 // in their respective registers.

            // Combine the 2 bytes of each cell voltage together
            cell_voltage_raw =
                ((uint16_t)(ADC_data[reg_group][ic_num][2 * reading_num + 1]) << 8) | (uint16_t)(ADC_data[reg_group][ic_num][2 * reading_num]);

            // Convert to volts
            converted_voltage = BTM_regValToVoltage(cell_voltage_raw);

            float delta = fabs(converted_voltage - EXPECTED_SC_VOLTAGE);
            if (delta > ST_VOLTAGE_ERROR)
            {
                status.error = BTM_ERROR_SELFTEST;
                status.device_num = ic_num + 1;
                return status;
            }
        }
    }

    return status;
}

/**
 * @brief Checks for any open wires between the ADCs of the LTC6813-1 and the external cells, making use of the ADOW
 *	 	  command (see datasheet p.32). Returns 3-digit error code:
 *	 	  1st digit is the board where the open wire is found, other 2 indicate the module number of the open wire.
 *	 	  (e.g. returns an device_num of 214 for an error on module 14 of the 2nd board).
 *
 * @return OK if no open wires detected. Error code as described above if detected.
 **/
BTM_Status_t ST_checkOpenWire()
{
    // 6x 6-byte sets (each from a different register group of the LTC6813) for each LTC6813
    uint8_t ADC_data[NUM_CELL_VOLT_REGS][BTM_NUM_DEVICES][BTM_REG_GROUP_SIZE];

    // Stores converted voltage values measured at each pin on the LTC6813-1 for both slave boards
    float moduleVoltage_PUP[BTM_NUM_DEVICES][BTM_NUM_CELL_INPUTS_PER_DEVICE];
    float moduleVoltage_PDOWN[BTM_NUM_DEVICES][BTM_NUM_CELL_INPUTS_PER_DEVICE];
    float moduleVoltage_DELTA = 0;

    BTM_Status_t status = {BTM_OK, 0};

    // Send open wire check command at least twice for PUP to allow capacitors to fully charge before
    // reading voltage register data.
    for (int i = 0; i < PUP_REPS; i++)
    {
        status = BTM_sendCmdAndPoll(CMD_ADOW_PUP);
        if (status.error != BTM_OK)
            return status;
    }

    // Read cell voltages from register after pull-up current is applied.
    status = readAllRegisters(ADC_data, moduleVoltage_PUP);
    if (status.error != BTM_OK)
        return status;

    // Send open wire check command PDOWN_REPS times for PDOWN to allow capacitors to fully charge before
    // reading voltage register data.
    for (int i = 0; i < PDOWN_REPS; i++)
    {
        status = BTM_sendCmdAndPoll(CMD_ADOW_PDOWN);
        if (status.error != BTM_OK)
            return status;
    }

    // Read cell voltages from register after pull-down current is applied.
    status = readAllRegisters(ADC_data, moduleVoltage_PDOWN);
    if (status.error != BTM_OK)
        return status;

    // Take the difference between pull-up and pull-down measurements for cells 2 to 18. If this difference
    // is < -400mV at module = n, then module = n-1 is open.

    for (int board = 0; board < BTM_NUM_DEVICES; board++)
    {
        for (int module = 1; module < BTM_NUM_CELL_INPUTS_PER_DEVICE; module++)
        {
            moduleVoltage_DELTA = moduleVoltage_PUP[board][module] - moduleVoltage_PDOWN[board][module];
            if (moduleVoltage_DELTA < OPEN_WIRE_VOLTAGE)
            {
                status.error = BTM_ERROR_SELFTEST;
                status.device_num = 100 * (board + 1) + module;
                return status;
            }
        }
    }

    // Check for open wire at pin C0
    for (int board = 0; board < BTM_NUM_DEVICES; board++)
    {
        if (moduleVoltage_PUP[board][0] == 0.0000)
        {
            status.error = BTM_ERROR_SELFTEST;
            status.device_num = 100 * (board + 1);
            return status;
        }
    }

    // Check for open wire at pin C18
    for (int board = 0; board < BTM_NUM_DEVICES; board++)
    {
        if (moduleVoltage_PUP[board][BTM_NUM_CELL_INPUTS_PER_DEVICE - 1] == 0.0000)
        {
            status.error = BTM_ERROR_SELFTEST;
            status.device_num = 100 * (board + 1);
            return status;
        }
    }

    return status;
}

/**
 * @brief Verifies that measurements taken using ADC1, ADC2 and ADC3 all agree within a certain
 * 		  range defined by ST_VOLTAGE_ERROR. Uses ADOL command to measure Cell 7 with ADC1 and ADC2.
 * 		  Then it simultaneously measures Cell 13 with both ADC2 and ADC3. This function compares
 * 		  the results of these measurements and reports any inconsistency as an error. (LTC6813-1 p. 30)
 *
 * @return OK if overlapping measurements are in agreement. Error status if ADCs do not produce the same
 * 		   voltage reading for each cell measured.
 */
BTM_Status_t ST_checkOverlapVoltage()
{
    // 2x 6-byte sets (each from a different register group of the LTC6813) for each LTC6813
    uint8_t ADC_data[OVERLAP_TEST_REGS][BTM_NUM_DEVICES][BTM_REG_GROUP_SIZE];
    float overlapVoltage[BTM_NUM_DEVICES][OVERLAP_READINGS_PER_BOARD];
    BTM_Status_t status = {BTM_OK, 0};

    uint16_t cell_voltage_raw = 0;
    float converted_voltage = 0;

    status = BTM_sendCmdAndPoll(CMD_ADOL);
    if (status.error != BTM_OK)
        return status;

    status = BTM_readRegisterGroup(CMD_RDCVC, ADC_data[0]);
    if (status.error != BTM_OK)
        return status;

    status = BTM_readRegisterGroup(CMD_RDCVE, ADC_data[1]);
    if (status.error != BTM_OK)
        return status;

    // Each cell voltage is provided as a 16-bit value where
    // voltage = 0.0001V * raw value
    // Each 6-byte Cell Voltage Register Group holds 3 cell voltages
    // First 2 bytes of Cell Voltage Register Group C is C7V
    // First 2 bytes of Cell Voltage Register Group E is C13V
    for (int board = 0; board < BTM_NUM_DEVICES; board++)
    {
        for (int reg_group = 0; reg_group < OVERLAP_TEST_REGS; reg_group++)
        {
            for (int reading_num = 0; reading_num < OVERLAP_READINGS_PER_REG; reading_num++)
            {
                // Combine the 2 bytes of each cell voltage together
                cell_voltage_raw =
                    ((uint16_t)(ADC_data[reg_group][board][2 * reading_num + 1]) << 8) | (uint16_t)(ADC_data[reg_group][board][2 * reading_num]);

                // Convert to volts
                converted_voltage = BTM_regValToVoltage(cell_voltage_raw);

                overlapVoltage[board][reading_num + 2 * reg_group] = converted_voltage;
            }
        }
    }

    // Check if overlap readings agree within delta
    for (int board = 0; board < BTM_NUM_DEVICES; board++)
    {
        for (int cell = 0; cell < NUM_TEST_CELLS; cell++)
        {

            // ANDREW: If you want to, you can do away with floats here and stick to integers
            // because the storage format of the raw data as integers isn't actually
            // hard to deal with - I can explain if you're curious

            float ADC1_voltage = overlapVoltage[board][2 * cell];
            float ADC2_voltage = overlapVoltage[board][2 * cell + 1];
            float delta = fabs(ADC1_voltage - ADC2_voltage);

            if (delta > ST_VOLTAGE_ERROR)
            {
                status.error = BTM_ERROR_SELFTEST;
                status.device_num = board + 1;
            }
        }
    }

    return status;
}

/**
 * @brief Tests functionality of discharge circuits by conducting cell measurements and comparing measured
 * 		  voltages when discharge is on and off. When measuring a cell with discharge enabled, the CH bit of
 * 		  CMD_ADCV is used to isolate for 3 cells at a time, ensuring that only the cells to be measured and
 * 		  their adjacent cells are able to discharge. (LTC6813-1 Data Sheet p.75-77)
 *
 * 		  With discharge enabled, each cell voltage measurement should have decreased by a fixed percentage
 * 		  defined by the discharge resistance values Rdischarge1 and Rdischarge 2 (Values still TBD).
 *
 * 		  Returns 3-digit error code:
 *	 	  1st digit is the board where the open wire is found, other 2 indicate the module number of the open wire.
 *	 	  (e.g. returns an device_num of 214 for an error on module 14 of the 2nd board).
 *
 * @return BTM_OK if the ratio of all initial and discharge enabled readings are equal to the
 * 		   expected ST_DCH_COMPARE_PCT +/- ST_DCH_PCT_DELTA. BTM_ERROR_SELFTEST if the measured
 * 		   cell voltage ratio is outside of this range.
 */
BTM_Status_t ST_verifyDischarge(Pack_t *pack)
{
    // 6x 6-byte sets (each from a different register group of the LTC6813)
    uint8_t ADC_data[NUM_CELL_VOLT_REGS][BTM_NUM_DEVICES][BTM_REG_GROUP_SIZE];

    // Stores converted voltage values measured at each pin on the LTC6813-1 for both slave boards
    // for initial measurement will all discharge pins disabled.
    float initial_voltage[BTM_NUM_DEVICES][BTM_NUM_CELL_INPUTS_PER_DEVICE];
    float discharge_voltage[BTM_NUM_DEVICES][BTM_NUM_CELL_INPUTS_PER_DEVICE];

    BTM_Status_t status = {BTM_OK, 0};

    // Write discharge settings to LTC6813s
    // All modules have discharging off to begin
    BTM_BAL_setDischarge(pack, dischargeVerificationPatterns[DISCHARGE_VERIFICATION_ALL_OFF_PATTERN_INDEX]);

    // Read voltage with discharging disabled
    status = BTM_sendCmdAndPoll(CMD_ADCV);
    if (status.error != BTM_OK)
        return status;

    status = readAllRegisters(ADC_data, initial_voltage);
    if (status.error != BTM_OK)
        return status;

    // Run ADC Cell Voltage Measurement command. Bitwise OR selects for
    // 3 cells to measure, CH1 measures cells 1, 7 and 13,
    // CH2 measures cells 2, 8, 14
    // ... CH6 measures cells 6, 12, 18.
    for (int i = 1; i < DISCHARGE_VERIFICATION_NUM_PATTERNS; i++)
    {
        BTM_BAL_setDischarge(pack, dischargeVerificationPatterns[i]);

        status = BTM_sendCmdAndPoll(CMD_ADCV | ADCV_DCP_MASK | i);
        if (status.error != BTM_OK)
            return status;
    }

    status = readAllRegisters(ADC_data, discharge_voltage);
    if (status.error != BTM_OK)
        return status;

    // Check if ratio of initial and discharge readings agree with expected percentage
    // defined by Rfilter and Rdischarge.
    for (int ic_num = 0; ic_num < BTM_NUM_DEVICES; ic_num++)
    {
        for (int module = 0; module < BTM_NUM_CELL_INPUTS_PER_DEVICE; module++)
        {
            // Skip unused modules
            if (module != 11 && module != 17)
            {
                float v_init_next = initial_voltage[ic_num][module];
                float v_dch_next = discharge_voltage[ic_num][module];
                float ratio = v_dch_next / v_init_next;
                float delta = fabs(ratio - ST_DCH_COMPARE_PCT);

                if (delta > ST_DCH_PCT_DELTA)
                {
                    status.error = BTM_ERROR_SELFTEST;
                    status.device_num = 100 * (ic_num + 1) + module;
                }
            }
        }
    }

    return status;
}

/*============================================================================*/
/* PRIVATE FUNCTION DEFINITIONS */

/**
 * @brief Converts unsigned int from register ADSTATA to a die temperature value in degrees Celsius.
 *        Conversion constants sourced from LTC6813 Data Sheets p.27
 *
 * @return void
 **/
STATIC_TESTABLE void itmpConversion(uint16_t itmp[], float temp_celsius[])
{
    const float itmp_coefficient = 0.013158;
    const float conversion_const = 276.0;

    float raw_reading;
    float celsiusTemp;

    for (int board = 0; board < BTM_NUM_DEVICES; board++)
    {
        raw_reading = itmp[board];
        celsiusTemp = itmp_coefficient * raw_reading - conversion_const;

        temp_celsius[board] = celsiusTemp;
    }
}

/**
 * @brief Converts all 2-byte raw cell voltage readings stored in registers A-F to a float voltage value and stores them
 * 		  in the moduleVoltage array for their respective board.
 *
 * @return OK if reading successful, otherwise returns error status.
 **/
static BTM_Status_t readAllRegisters(uint8_t ADC_data[NUM_CELL_VOLT_REGS][BTM_NUM_DEVICES][BTM_REG_GROUP_SIZE],
                                     float moduleVoltage[BTM_NUM_DEVICES][BTM_NUM_CELL_INPUTS_PER_DEVICE])
{
    BTM_Status_t status = {BTM_OK, 0};
    uint16_t cell_voltage_raw = 0;
    float converted_voltage = 0;
    int module_count = 0;

    status = BTM_readRegisterGroup(CMD_RDCVA, ADC_data[0]);
    if (status.error != BTM_OK)
        return status;

    status = BTM_readRegisterGroup(CMD_RDCVB, ADC_data[1]);
    if (status.error != BTM_OK)
        return status;

    status = BTM_readRegisterGroup(CMD_RDCVC, ADC_data[2]);
    if (status.error != BTM_OK)
        return status;

    status = BTM_readRegisterGroup(CMD_RDCVD, ADC_data[3]);
    if (status.error != BTM_OK)
        return status;

    status = BTM_readRegisterGroup(CMD_RDCVE, ADC_data[4]);
    if (status.error != BTM_OK)
        return status;

    status = BTM_readRegisterGroup(CMD_RDCVF, ADC_data[5]);
    if (status.error != BTM_OK)
        return status;

    // Each cell voltage is provided as a 16-bit value where
    // voltage = 0.0001V * raw value
    // Each 6-byte Cell Voltage Register Group holds 3 cell voltages
    // First 2 bytes of Cell Voltage Register Group A is C1V
    // Last 2 bytes of Cell Voltage Register Group D is C12V
    for (int ic_num = 0; ic_num < BTM_NUM_DEVICES; ic_num++)
    {
        module_count = 0;
        for (int reg_group = 0; reg_group < NUM_CELL_VOLT_REGS; reg_group++)
        {
            for (int reading_num = 0; reading_num < READINGS_PER_REG; reading_num++)
            {
                // Combine the 2 bytes of each cell voltage together
                cell_voltage_raw =
                    ((uint16_t)(ADC_data[reg_group][ic_num][2 * reading_num + 1]) << 8) | (uint16_t)(ADC_data[reg_group][ic_num][2 * reading_num]);

                // Convert to volts
                converted_voltage = BTM_regValToVoltage(cell_voltage_raw);

                // Add module voltage to array for specified ic_num
                moduleVoltage[ic_num][module_count] = converted_voltage;
                module_count++;
            }
        }
    }

    return status;
}
