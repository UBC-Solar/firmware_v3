/**
 * @file selftest.c
 * @brief BMS startup self tests
 *
 * @date 2020/10/03
 * @author matthewjegan
 */

#include "selftest.h"

/**
 * @brief Checks internal die temperature of LTC6813's for safe operating condition
 *
 * @return If at least one LTC6813 has a die temperature nearing thermal shutdown
 * threshold, returns an error with the device index of the first overheating IC
**/

void itmpConversion(uint16_t ITMP[], float temp_celsius[]);

BTM_Status_t ST_checkLTCtemp()
{
    BTM_Status_t status = {BTM_OK, 0};

    // ...
    // Maybe use CMD_ADSTAT_ITMP

    // Remember most things with the LTC6813 are accomplished in 2 steps:
    // a) Send a command to have it perform on operation
    // b) Read a register to get the results from that operation

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
    	if (temp_celsius[board] >= LTC_TEMPLIMIT){
    		status.error = BTM_ERROR_SELFTEST;
    		status.device_num = board;
    	}
    }
    
    return status;
}


/**
 * @brief Checks for any open wires between the ADCs of the LTC6813-1 and the external cells, making use of the ADOW
 *	 	  command (see data sheets p.31).
 *
 * @return void
**/
BTM_Status_t ST_checkOpenWire()
{
	// 4x 6-byte sets (each from a different register group of the LTC6813)
	uint8_t ADC_data[NUM_CELL_VOLT_REGS][BTM_NUM_DEVICES][BTM_REG_GROUP_SIZE];
	uint16_t cell_voltage_raw = 0;
	float converted_voltage = 0;

	// Stores converted voltage values measured at each pin on the LTC6813-1 for both slave boards
	float moduleVoltage_PUP[BTM_NUM_DEVICES][BTM_NUM_MODULES];
	float moduleVoltage_PDOWN[BTM_NUM_DEVICES][BTM_NUM_MODULES];
	float moduleVoltage_DELTA = 0;

	int module_count = 0;
	BTM_Status_t status = {BTM_OK, 0};

	// Send open wire check command twice for PDOWN to allow capacitors to fully charge before
	// reading voltage register data.
	status = BTM_sendCmdAndPoll(CMD_ADOW_PDOWN);
	if (status.error != BTM_OK) return status;

	status = BTM_sendCmdAndPoll(CMD_ADOW_PDOWN);
	if (status.error != BTM_OK) return status;

	// Read cell voltages from register after pull-down current is applied.

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
					converted_voltage = BTM_VOLTAGE_CONVERSION_FACTOR * cell_voltage_raw;

					// Add module voltage to array for specified ic_num
					moduleVoltage_PDOWN[ic_num][module_count] = converted_voltage;
					module_count++;
				}
			}
		}

	// Send open wire check command twice for PUP to allow capacitors to fully charge before
	// reading voltage register data.
	status = BTM_sendCmdAndPoll(CMD_ADOW_PUP);
	if (status.error != BTM_OK) return status;

	status = BTM_sendCmdAndPoll(CMD_ADOW_PUP);
	if (status.error != BTM_OK) return status;

	// Read cell voltages from register after pull-down current is applied.

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
					converted_voltage = BTM_VOLTAGE_CONVERSION_FACTOR * cell_voltage_raw;

					// Add module voltage to array for specified ic_num
					moduleVoltage_PUP[ic_num][module_count] = converted_voltage;
					module_count++;
				}
			}
		}

	// Take the difference between pull-up and pull-down measurements for cells 2 to 18. If this difference
	// is < -400mV at module = n, then module = n-1 is open.
	for (int board = 0; board < BTM_NUM_DEVICES; board++){
		for (int module = 1; module < BTM_NUM_MODULES; module++){
			moduleVoltage_DELTA = moduleVoltage_PUP[board][module] - moduleVoltage_PDOWN[board][module];
			if (moduleVoltage_DELTA < OPEN_WIRE_VOLTAGE_THRESHOLD){
				status.error = BTM_ERROR_SELFTEST;
				status.device_num = board;
			}
		}
	}

	// Check for open wire at pin C0
	for (int board = 0; board < BTM_NUM_DEVICES; board++){
		if (moduleVoltage_PUP[board][0] == 0.0000){
			status.error = BTM_ERROR_SELFTEST;
			status.device_num = board;
		}
	}

	// Check for open wire at pin C18
	for (int board = 0; board < BTM_NUM_DEVICES; board++){
		if (moduleVoltage_PUP[board][BTM_NUM_MODULES - 1] == 0.0000){
			status.error = BTM_ERROR_SELFTEST;
			status.device_num = board;
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

void itmpConversion(uint16_t ITMP[], float temp_celsius[])
{
	const float itmp_coefficient = 0.013158;
	const float conversion_const = 276.0;
	
	unsigned int raw_reading;
	float celsiusTemp;
	
	for (int board = 0; board < BTM_NUM_DEVICES; board++){
		 raw_reading = ITMP[board];
		 celsiusTemp = itmp_coefficient * raw_reading - conversion_const;
		 
		 temp_celsius[board] = celsiusTemp;
	}
}
