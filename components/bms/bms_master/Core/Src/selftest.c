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
        temp_reading = ( ((uint16_t) registerSTATA[board][1]) << 8)
            | registerSTATA[board][0];
        
        // Store in given array
        itmp[board] = temp_reading;
    }
    
    itmpConversion(itmp, temp_celsius);
    
    for (int board = 0; board <  BTM_NUM_DEVICES; board++){
    	if (temp_celsius[board] >= LTC_TEMPLIMIT){
    		status = {BTM_ERROR_SELFTEST} // check with Andrew if this is proper syntax
    	}
    }
    
    return status;
}



BTM_Status_t ST_checkOpenWire(){
	
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
