/**
 * @file selftest.c
 * @brief BMS startup self tests
 *
 * @date 2020/10/03
 * @author matthewjegan
 */

#include "selftest.h"
#include "ltc6813_btm_bal.h"

void itmpConversion(uint16_t ITMP[], float temp_celsius[]);
BTM_Status_t readAllRegisters(BTM_Status_t status,
							  uint8_t ADC_data[NUM_CELL_VOLT_REGS][BTM_NUM_DEVICES][BTM_REG_GROUP_SIZE],
							  float moduleVoltage[BTM_NUM_DEVICES][BTM_NUM_MODULES]);
void shift_dch_status(BTM_module_bal_status_t module_dch[BTM_NUM_MODULES]);
void set_dch_pack(BTM_BAL_dch_setting_pack_t* dch_pack,
				  BTM_module_bal_status_t new_module_dch[BTM_NUM_MODULES]);
/**
* @brief Checks internal die temperature of LTC6813's for safe operating condition
*
* @return If at least one LTC6813 has a die temperature nearing thermal shutdown
* threshold, returns an error with the device index of the first overheating IC.
**/
BTM_Status_t ST_checkLTCtemp()
{
    BTM_Status_t status = {BTM_OK, 0};
    uint8_t registerSTATA[BTM_NUM_DEVICES][BTM_REG_GROUP_SIZE];
    uint16_t itmp[BTM_NUM_DEVICES];
    float temp_celsius[BTM_NUM_DEVICES];

    status = BTM_sendCmdAndPoll(CMD_ADSTAT_ITMP);
    if (status.error != BTM_OK) return status;

    //Retrieve register reading
    status = BTM_readRegisterGroup(CMD_RDSTATA, registerSTATA);
    if (status.error != BTM_OK) return status;

    for (int board = 0; board < BTM_NUM_DEVICES; board++){
        // Combine 2 bytes of die temperature reading
        itmp[board] = ( ((uint16_t) registerSTATA[board][1]) << 8)
            | registerSTATA[board][0];
    }

    itmpConversion(itmp, temp_celsius);

    for (int board = 0; board <  BTM_NUM_DEVICES; board++){
    	if (temp_celsius[board] >= ST_LTC_TEMPLIMIT){
    		status.error = BTM_ERROR_SELFTEST;
    		status.device_num = board + 1;
    	}
    }

    return status;
}

/**
 * @brief Measures independent reference voltage VREF2 to verify measurement of ADC1.
 * Readings outside the range 2.990V to 3.014V indicate the system is out of its specified tolerance.
 * Accuracy of ADC2 measurement is verified separately using ST_checkOverlapVoltage() command.
 *
 * @return OK if reading is within tolerance range. BTM_ERROR_SELFTEST if ADC1 measurement is outside
 * 		   tolerance range.
 */
BTM_Status_t ST_checkVREF2(){
	uint8_t registerAUXB[BTM_NUM_DEVICES][BTM_REG_GROUP_SIZE];
	uint16_t cell_voltage_raw = 0;
	float converted_voltage = 0;

	BTM_Status_t status = {BTM_OK, 0};

	status = BTM_sendCmdAndPoll(CMD_ADAX_VREF2);
	if (status.error != BTM_OK) return status;

	status = BTM_readRegisterGroup(CMD_RDAUXB, registerAUXB);
	if (status.error != BTM_OK) return status;


	// Each cell voltage is provided as a 16-bit value where
	// voltage = 0.0001V * raw value
	// Each 6-byte Cell Voltage Register Group holds 3 cell voltages
	// Last 2 bytes of Auxiliary Voltage Register Group B is VREF2
	for (int board = 0; board < BTM_NUM_DEVICES; board++)
		{
			// Combine the 2 bytes of each cell voltage together
			cell_voltage_raw =
				((uint16_t) (registerAUXB[board][5]) << 8)
				| (uint16_t) (registerAUXB[board][4]);

			// Convert to volts and store
			converted_voltage = BTM_regValToVoltage(cell_voltage_raw);

			if (converted_voltage < ST_VREF_LOWERBOUND ||
				converted_voltage > ST_VREF_UPPERBOUND)
			{
				status.error = BTM_ERROR_SELFTEST;
				status.device_num = board;
				return status;
			}
		}

	return status;
}


/**
 * @brief Checks for any open wires between the ADCs of the LTC6813-1 and the external cells, making use of the ADOW
 *	 	  command (see data sheets p.31). Returns 3-digit error code:
 *	 	  1st digit is the board where the open wire is found, other 2 indicate the module number of the open wire.
 *	 	  (e.g. returns an device_num of 214 for an error on module 14 of the 2nd board).
 *
 * @return void
**/
BTM_Status_t ST_checkOpenWire()
{
	// 4x 6-byte sets (each from a different register group of the LTC6813)
	uint8_t ADC_data[NUM_CELL_VOLT_REGS][BTM_NUM_DEVICES][BTM_REG_GROUP_SIZE];

	// Stores converted voltage values measured at each pin on the LTC6813-1 for both slave boards
	float moduleVoltage_PUP[BTM_NUM_DEVICES][BTM_NUM_MODULES];
	float moduleVoltage_PDOWN[BTM_NUM_DEVICES][BTM_NUM_MODULES];
	float moduleVoltage_DELTA = 0;

	BTM_Status_t status = {BTM_OK, 0};

	// Send open wire check command twice for PDOWN to allow capacitors to fully charge before
	// reading voltage register data.
	status = BTM_sendCmdAndPoll(CMD_ADOW_PDOWN);
	if (status.error != BTM_OK) return status;

	status = BTM_sendCmdAndPoll(CMD_ADOW_PDOWN);
	if (status.error != BTM_OK) return status;

	// Read cell voltages from register after pull-down current is applied.
	status = readAllRegisters(status, ADC_data, moduleVoltage_PDOWN);
	if (status.error != BTM_OK) return status;

	// Send open wire check command twice for PUP to allow capacitors to fully charge before
	// reading voltage register data.
	status = BTM_sendCmdAndPoll(CMD_ADOW_PUP);
	if (status.error != BTM_OK) return status;

	status = BTM_sendCmdAndPoll(CMD_ADOW_PUP);
	if (status.error != BTM_OK) return status;

	// Read cell voltages from register after pull-up current is applied.
	status = readAllRegisters(status, ADC_data, moduleVoltage_PUP);
	if (status.error != BTM_OK) return status;

	// Take the difference between pull-up and pull-down measurements for cells 2 to 18. If this difference
	// is < -400mV at module = n, then module = n-1 is open.

	for (int board = 0; board < BTM_NUM_DEVICES; board++){
		for (int module = 1; module < BTM_NUM_MODULES; module++){
			moduleVoltage_DELTA = moduleVoltage_PUP[board][module] - moduleVoltage_PDOWN[board][module];
			if (moduleVoltage_DELTA < ST_OPEN_WIRE_VOLTAGE){
				status.error = BTM_ERROR_SELFTEST;
				status.device_num = 100 * (board + 1) + module;
				return status;
			}
		}
	}

	// Check for open wire at pin C0
	for (int board = 0; board < BTM_NUM_DEVICES; board++){
		if (moduleVoltage_PUP[board][0] == 0.0000){
			status.error = BTM_ERROR_SELFTEST;
			status.device_num = 100 * (board + 1);
			return status;
		}
	}

	// Check for open wire at pin C18
	for (int board = 0; board < BTM_NUM_DEVICES; board++){
		if (moduleVoltage_PUP[board][BTM_NUM_MODULES - 1] == 0.0000){
			status.error = BTM_ERROR_SELFTEST;
			status.device_num = 100 * (board + 1);
			return status;
		}
	}

	return status;
}


/**
 * @brief Uses ADOL command to measure Cell 7 with ADC1 and ADC2. Then it simultaneously measures
 *		  Cell 13 with both ADC2 and ADC3. This function compares the results of these measurements
 *		  and reports any inconsistency as an error.
 *
 * @return OK if overlapping measurements are in agreement. Error status if ADCs do not produce the same
 * 		   voltage reading for each cell measured.
 */
BTM_Status_t ST_checkOverlapVoltage(void){
	// 4x 6-byte sets (each from a different register group of the LTC6813)
	uint8_t ADC_data[OVERLAP_TEST_REGS][BTM_NUM_DEVICES][BTM_REG_GROUP_SIZE];
	float overlapVoltage[BTM_NUM_DEVICES][OVERLAP_READINGS_PER_BOARD];
	BTM_Status_t status = {BTM_OK, 0};

	uint16_t cell_voltage_raw = 0;
	float converted_voltage = 0;

	status = BTM_sendCmdAndPoll(CMD_ADOL);
	if (status.error != BTM_OK) return status;

	status = BTM_readRegisterGroup(CMD_RDCVC, ADC_data[0]);
	if (status.error != BTM_OK) return status;

	status = BTM_readRegisterGroup(CMD_RDCVE, ADC_data[1]);
	if (status.error != BTM_OK) return status;

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
						((uint16_t) (ADC_data[reg_group][board][2 * reading_num + 1]) << 8)
						| (uint16_t) (ADC_data[reg_group][board][2 * reading_num]);

					// Convert to volts
					converted_voltage = BTM_regValToVoltage(cell_voltage_raw);

					overlapVoltage[board][reading_num + 2 * reg_group] = converted_voltage;
				}
			}
		}

	// Check if overlap readings agree within delta
	for (int board = 0; board < BTM_NUM_DEVICES; board++){
		for (int cell = 0; cell < NUM_TEST_CELLS; cell++){

			// ANDREW: If you want to, you can do away with floats here and stick to integers
			// because the storage format of the raw data as integers isn't actually
			// hard to deal with - I can explain if you're curious

			float ADC1_voltage = overlapVoltage[board][2 * cell];
			float ADC2_voltage = overlapVoltage[board][2 * cell + 1];
			float delta = abs(ADC1_voltage - ADC2_voltage);

			if (delta > ST_OVERLAP_DELTA){
				status.error = BTM_ERROR_SELFTEST;
				status.device_num = board + 1;
			}
		}
	}

	return status;
}

/**
 *
 * @brief TODO
 */
BTM_Status_t ST_verifyDischarge(BTM_PackData_t* pack){
	// 4x 6-byte sets (each from a different register group of the LTC6813)
	uint8_t ADC_data[NUM_CELL_VOLT_REGS][BTM_NUM_DEVICES][BTM_REG_GROUP_SIZE];

	// Stores converted voltage values measured at each pin on the LTC6813-1 for both slave boards
	// for initial measurement will all discharge pins disabled.
	float initial_voltage[BTM_NUM_DEVICES][BTM_NUM_MODULES];
	float discharge_voltage[BTM_NUM_DEVICES][BTM_NUM_MODULES];

	BTM_Status_t status = {BTM_OK, 0};

	BTM_BAL_dch_setting_pack_t ST_dch_pack;

	// Initialize discharge pack with all discharge (S) pins set to OFF
	BTM_BAL_initDchPack(&ST_dch_pack);

	// Write discharge settings to LTC6813s
	BTM_BAL_setDischarge(pack, &ST_dch_pack);

	// Read voltage with discharging disabled
	status = BTM_sendCmdAndPoll(CMD_ADCV);
	if (status.error != BTM_OK) return status;

	status = readAllRegisters(status, ADC_data, initial_voltage);
	if (status.error != BTM_OK) return status;

	BTM_module_bal_status_t dch_off = {DISCHARGE_OFF};
	BTM_module_bal_status_t dch_on = {DISCHARGE_ON};

	// Set initial discharge pin settings (see p.77 on LTC6813-1 Datasheet)
	// Start by turning on S1, S7, S13
	BTM_module_bal_status_t s_pin_set[BTM_NUM_MODULES] = {dch_on, dch_off, dch_off,
														  dch_off, dch_off, dch_off,
									         		      dch_on, dch_off, dch_off,
														  dch_off, dch_off, dch_off,
														  dch_on, dch_off, dch_off,
														  dch_off, dch_off, dch_off};

	// Run ADC Cell Voltage Measurement command. Bitwise OR selects for
	// 3 cells to measure, CH1 measures cells 1, 7 and 13,
	// CH2 measures cells 2, 8, 14
	// ... CH6 measures cells 6, 12, 18.
	for (int i = 1; i <= 6; i++){
		set_dch_pack(&ST_dch_pack, s_pin_set);
		BTM_BAL_setDischarge(pack, &ST_dch_pack);

		status = BTM_sendCmdAndPoll(CMD_ADCV | i);
		if (status.error != BTM_OK) return status;

		shift_dch_status(s_pin_set);
	}

	status = readAllRegisters(status, ADC_data, discharge_voltage);
	if (status.error != BTM_OK) return status;

	// Check if ratio of initial and discharge readings agree with expected percentage
	// defined by Rfilter and Rdischarge.

	for (int ic_num = 0; ic_num < BTM_NUM_DEVICES; ic_num++){
		for (int module = 0; module < BTM_NUM_MODULES; module++){

			float v_init_next = initial_voltage[ic_num][module];
			float v_dch_next = discharge_voltage[ic_num][module];
			float ratio =  v_dch_next / v_init_next;
			float delta = abs(ratio - ST_DCH_COMPARE_PCT);

			if (delta > ST_DCH_PCT_DELTA){
				status.error = BTM_ERROR_SELFTEST;
				status.device_num = ic_num + 1;
			}
		}
	}

    return status;
}


/**
 * @brief Converts unsigned int from register ADSTATA to a die temperature value in degrees Celsius.
 *        Conversion constants sourced from LTC6813 Data Sheets p.26
 *
 * @return void
**/
void itmpConversion(uint16_t itmp[], float temp_celsius[])
{
	const float itmp_coefficient = 0.013158;
	const float conversion_const = 276.0;

	unsigned int raw_reading;
	float celsiusTemp;

	for (int board = 0; board < BTM_NUM_DEVICES; board++){
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
BTM_Status_t readAllRegisters(BTM_Status_t status,
							  uint8_t ADC_data[NUM_CELL_VOLT_REGS][BTM_NUM_DEVICES][BTM_REG_GROUP_SIZE],
							  float moduleVoltage[BTM_NUM_DEVICES][BTM_NUM_MODULES]){

	uint16_t cell_voltage_raw = 0;
	float converted_voltage = 0;
	int module_count = 0;

	status = BTM_readRegisterGroup(CMD_RDCVA, ADC_data[0]);
	if (status.error != BTM_OK) return status;

	status = BTM_readRegisterGroup(CMD_RDCVB, ADC_data[1]);
	if (status.error != BTM_OK) return status;

	status = BTM_readRegisterGroup(CMD_RDCVC, ADC_data[2]);
	if (status.error != BTM_OK) return status;

	status = BTM_readRegisterGroup(CMD_RDCVD, ADC_data[3]);
	if (status.error != BTM_OK) return status;

	status = BTM_readRegisterGroup(CMD_RDCVE, ADC_data[4]);
	if (status.error != BTM_OK) return status;

	status = BTM_readRegisterGroup(CMD_RDCVF, ADC_data[5]);
	if (status.error != BTM_OK) return status;

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
						((uint16_t) (ADC_data[reg_group][ic_num][2 * reading_num + 1]) << 8)
						| (uint16_t) (ADC_data[reg_group][ic_num][2 * reading_num]);

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

/**
 * @brief Takes a BTM_module_bal_status_t structure and shifts all discharge pin settings
 * 		  in the array to the right such that module_dch[n] = module_dch[n-1]. The last
 * 		  discharge bit in the input array is shifted to the 0 index.
 *
 * @param module_dch
 */
void shift_dch_status(BTM_module_bal_status_t module_dch[BTM_NUM_MODULES]){
	BTM_module_bal_status_t end_status = module_dch[BTM_NUM_MODULES - 1];

	for (int module = 1; module < BTM_NUM_MODULES; module++){
		module_dch[module] = module_dch[module - 1];
	}

	module_dch[0] = end_status;

	return;
}

/**
 * @brief Takes a pointer to an existing BTM_BAL_dch_setting_pack_t structure and
 * 		  and an array of discharge pin settings provided by new_module_dch. Sets
 * 		  discharge pins on each LTC6813 board according to the provided settings.
 *
 * @param dch_pack
 * @param new_module_dch
 */
void set_dch_pack(BTM_BAL_dch_setting_pack_t* dch_pack,
				  BTM_module_bal_status_t new_module_dch[BTM_NUM_MODULES]){
    for(int stack_num = 0; stack_num < BTM_NUM_DEVICES; stack_num++)
    {
        for(int module_num = 0; module_num < BTM_NUM_MODULES; module_num++)
        {
            dch_pack->stack[stack_num].module_dch[module_num] = new_module_dch[module_num];
        }
    }
    return;
}
