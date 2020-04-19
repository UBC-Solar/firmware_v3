/**
 * 	@file ltc6811_btm.c
 *  @brief Driver for the LTC6811-1 battery monitor IC.
 *
 *  All functions associated with this driver are prefixed with "BTM"
 *
 *
 *  Created on: Feb. 14, 2020
 *  @author Andrew Hanlon (a2k-hanlon)
 *	@author Laila Khan (lailakhankhan)
 *
 */

#include "LTC6811_btm.h"

#define BTM_VOLTAGE_CONVERSION_FACTOR 0.0001

// Lookup table for PEC (Packet Error Code) CRC calculation
// See note at end of file for details of its origin
const uint16_t pec15Table[256] =
{
    0x0000, 0xC599, 0xCEAB, 0x0B32, 0xD8CF, 0x1D56, 0x1664, 0xD3FD, 0xF407,
    0x319E, 0x3AAC, 0xFF35, 0x2CC8, 0xE951, 0xE263, 0x27FA, 0xAD97, 0x680E,
    0x633C, 0xA6A5, 0x7558, 0xB0C1, 0xBBF3, 0x7E6A, 0x5990, 0x9C09, 0x973B,
    0x52A2, 0x815F, 0x44C6, 0x4FF4, 0x8A6D, 0x5B2E, 0x9EB7, 0x9585, 0x501C,
    0x83E1, 0x4678, 0x4D4A, 0x88D3, 0xAF29, 0x6AB0, 0x6182, 0xA41B, 0x77E6,
    0xB27F, 0xB94D, 0x7CD4, 0xF6B9, 0x3320, 0x3812, 0xFD8B, 0x2E76, 0xEBEF,
    0xE0DD, 0x2544, 0x02BE, 0xC727, 0xCC15, 0x098C, 0xDA71, 0x1FE8, 0x14DA,
    0xD143, 0xF3C5, 0x365C, 0x3D6E, 0xF8F7, 0x2B0A, 0xEE93, 0xE5A1, 0x2038,
    0x07C2, 0xC25B, 0xC969, 0x0CF0, 0xDF0D, 0x1A94, 0x11A6, 0xD43F, 0x5E52,
    0x9BCB, 0x90F9, 0x5560, 0x869D, 0x4304, 0x4836, 0x8DAF, 0xAA55, 0x6FCC,
    0x64FE, 0xA167, 0x729A, 0xB703, 0xBC31, 0x79A8, 0xA8EB, 0x6D72, 0x6640,
    0xA3D9, 0x7024, 0xB5BD, 0xBE8F, 0x7B16, 0x5CEC, 0x9975, 0x9247, 0x57DE,
    0x8423, 0x41BA, 0x4A88, 0x8F11, 0x057C, 0xC0E5, 0xCBD7, 0x0E4E, 0xDDB3,
    0x182A, 0x1318, 0xD681, 0xF17B, 0x34E2, 0x3FD0, 0xFA49, 0x29B4, 0xEC2D,
    0xE71F, 0x2286, 0xA213, 0x678A, 0x6CB8, 0xA921, 0x7ADC, 0xBF45, 0xB477,
    0x71EE, 0x5614, 0x938D, 0x98BF, 0x5D26, 0x8EDB, 0x4B42, 0x4070, 0x85E9,
    0x0F84, 0xCA1D, 0xC12F, 0x04B6, 0xD74B, 0x12D2, 0x19E0, 0xDC79, 0xFB83,
    0x3E1A, 0x3528, 0xF0B1, 0x234C, 0xE6D5, 0xEDE7, 0x287E, 0xF93D, 0x3CA4,
    0x3796, 0xF20F, 0x21F2, 0xE46B, 0xEF59, 0x2AC0, 0x0D3A, 0xC8A3, 0xC391,
    0x0608, 0xD5F5, 0x106C, 0x1B5E, 0xDEC7, 0x54AA, 0x9133, 0x9A01, 0x5F98,
    0x8C65, 0x49FC, 0x42CE, 0x8757, 0xA0AD, 0x6534, 0x6E06, 0xAB9F, 0x7862,
    0xBDFB, 0xB6C9, 0x7350, 0x51D6, 0x944F, 0x9F7D, 0x5AE4, 0x8919, 0x4C80,
    0x47B2, 0x822B, 0xA5D1, 0x6048, 0x6B7A, 0xAEE3, 0x7D1E, 0xB887, 0xB3B5,
    0x762C, 0xFC41, 0x39D8, 0x32EA, 0xF773, 0x248E, 0xE117, 0xEA25, 0x2FBC,
    0x0846, 0xCDDF, 0xC6ED, 0x0374, 0xD089, 0x1510, 0x1E22, 0xDBBB, 0x0AF8,
    0xCF61, 0xC453, 0x01CA, 0xD237, 0x17AE, 0x1C9C, 0xD905, 0xFEFF, 0x3B66,
    0x3054, 0xF5CD, 0x2630, 0xE3A9, 0xE89B, 0x2D02, 0xA76F, 0x62F6, 0x69C4,
    0xAC5D, 0x7FA0, 0xBA39, 0xB10B, 0x7492, 0x5368, 0x96F1, 0x9DC3, 0x585A,
    0x8BA7, 0x4E3E, 0x450C, 0x8095
};

/**
 * @brief Calculates the PEC for "len" bytes of data (as a group).
 * Function adapted from code on pg. 76 of LTC6811 Datasheet.
 *
 * @param data The bytes to calculate a PEC for
 * @param len The number of bytes of data to calculate the PEC for
 * @return Returns the 2-byte CRC PEC generated
 */
uint16_t BTM_calculatePec15(uint8_t *data, int len)
{
	uint16_t remainder, address;
	remainder = 16; // initial value for PEC computation
	for (int i = 0; i < len; i++)
	{
		address = ((remainder >> 7) ^ data[i]) & 0xff; // lookup table address
		remainder = (remainder << 8) ^ pec15Table[address];
	}
	return (remainder << 1); // The CRC15 has a 0 in the LSB so the final value
							 // must be leftshifted 1 bit
}

/**
 * @brief Toggles the CS line to wake up the entire chain of LTC6811's.
 * Wakes up the daisy chain as per method described on pg. 52 of datasheet
 * (method 2)
 * Using HAL_Delay() for this is not particularly ideal, since the minimum delay
 * is 1ms and the delays required are 300us and 10us or something (shorter than 1 ms)
 */
void BTM_wakeup()
{
	for (int i = 0; i < BTM_NUM_DEVICES; i++)
	{
		HAL_GPIO_WritePin(BTM_CS_GPIO_PORT, BTM_CS_GPIO_PIN, 0); // pull CS line low
		HAL_Delay(1); // wait 1ms
		HAL_GPIO_WritePin(BTM_CS_GPIO_PORT, BTM_CS_GPIO_PIN, 1); // pull CS line high
		// Then delay at least 10us
		HAL_Delay(1); // wait 1ms - the minimum with this timer setup
	}
	return;
}

void BTM_init(BTM_PackData_t * pack)
{
    uint8_t cfgr_to_write[BTM_NUM_DEVICES][BTM_REG_GROUP_SIZE];
    uint8_t config_val[BTM_REG_GROUP_SIZE] =
    {
        0xF8 | (REFON << 2) | ADCOPT, // GPIO 1-5 = 1, REFON, ADCOPT
        (VUV & 0xFF), // VUV[7:0]
        ((uint8_t) (VOV << 4)) | (((uint8_t) (VUV >> 8)) & 0x0F), // VOV[4:0] | VUV[11:8]
        (VOV >> 4), // VOV[11:5]
        0x00, // Discharge off for cells 1 through 8
        0x00  // Discharge off for cells 9 through 12, Discharge timer disabled
    };

    // Initialize given PackData structure
    pack->packVoltage = 0;
    for(int ic_num = 0; ic_num < BTM_NUM_DEVICES; ic_num++)
    {
        for(int reg_num = 0; reg_num > BTM_REG_GROUP_SIZE; reg_num++)
        {
            pack->stack[ic_num].cfgr[reg_num] = config_val[reg_num];
            cfgr_to_write[ic_num][reg_num] = config_val[reg_num]; // prepare tx data
        }

        pack->stack[ic_num].stack_voltage = 0;
        for(int module_num = 0; module_num < BTM_NUM_MODULES; module_num++)
        {
            pack->stack[ic_num].module[module_num].enable = MODULE_ENABLED;
            pack->stack[ic_num].module[module_num].voltage = 0;
            pack->stack[ic_num].module[module_num].temperature = 0;
        }

    }

    BTM_wakeup(); // Wake up all LTC6811's in the chain
    BTM_writeRegisterGroup(CMD_WRCFGA, cfgr_to_write); // Write to Config. Reg. Group
    return;
}

/**
 * @brief sends a 2-byte command followed by the 2-byte PEC for that command
 * over SPI.
 * @attention The caller must ensure the CS line is pulled low prior to calling
 * this function.
 *
 * @param command The 2-byte command to send
 */
void BTM_sendCmd(BTM_command_t command)
{
	uint16_t pecValue;
	uint8_t tx_message[4];

	tx_message[0] = (uint8_t) (command >> 8);
	tx_message[1] = (uint8_t) command;
	pecValue = BTM_calculatePec15(tx_message, 2);
	tx_message[2] = (uint8_t) (pecValue >> 8);
	tx_message[3] = (uint8_t) pecValue;

	// size parameter is number of bytes to transmit - here it's 4 8bit frames
	HAL_SPI_Transmit(BTM_SPI_handle, tx_message, 4, BTM_TIMEOUT_VAL);
	return;
}

/**
 * @brief sends a polling-type command (eg. ADCV) and then polls the LTC6811
 * This function is blocking. It will wait for the LTC6811 to signal it is
 * finished; however, there is a timeout feature in case something goes wrong.
 * The timeout threshold is BTM_TIMEOUT_VAL.
 *
 * @param command The 2-byte (polling) command to send
 * @return 	Returns BTM_OK once LTC6811s have completed their conversions,
 			or BTM_ERROR_TIMEOUT upon timeout.
 */
BTM_Status_t BTM_sendCmdAndPoll(BTM_command_t command)
{
	uint8_t rx_buffer = 0;
	uint32_t start_tick;

	HAL_GPIO_WritePin(BTM_CS_GPIO_PORT, BTM_CS_GPIO_PIN, 0); // Pull CS low

	BTM_sendCmd(command);

	start_tick = HAL_GetTick(); // Start timeout timer

	// Poll for conversion completion; see "Polling Methods" in datasheet pg. 55-57)
	// Make sure MISO goes low...
	do
	{
		if (HAL_GetTick() - start_tick > BTM_TIMEOUT_VAL)
		{
			return BTM_ERROR_TIMEOUT; // LTC didn't respond before timeout
		}
		rx_buffer = 0;
		HAL_SPI_Receive(BTM_SPI_handle, &rx_buffer, 1, BTM_TIMEOUT_VAL);
	} while (0xff == rx_buffer);

	// ... then wait for MISO to go high;
	// this signifies that the LTC6811s are done reading their ADCs.

	// Must send at least BTM_NUM_DEVICES clock pulses before response is valid
	// That's why there's an extra read initially - it's slight overkill but that's ok
	rx_buffer = 0;
	HAL_SPI_Receive(BTM_SPI_handle, &rx_buffer, 1, BTM_TIMEOUT_VAL);
	do
	{
		if (HAL_GetTick() - start_tick > BTM_TIMEOUT_VAL)
		{
			return BTM_ERROR_TIMEOUT; // LTC didn't respond before timeout
		}
		rx_buffer = 0;
		HAL_SPI_Receive(BTM_SPI_handle, &rx_buffer, 1, BTM_TIMEOUT_VAL);
	} while (!rx_buffer);

	HAL_GPIO_WritePin(BTM_CS_GPIO_PORT, BTM_CS_GPIO_PIN, 1); // pull CS back high

	return BTM_OK;
}

/**
 * @brief Writes the 6 bytes of a configuration register group in the LTC6811
 *
 * @param command 	A write command to specify which register group to write.
 * 					Write commands start with "WR"
 * @param tx_data 	Pointer to a 2-dimensional array of size
 *					BTM_NUM_DEVICES x BTM_REG_GROUP_SIZE containing the data to write
 */
void BTM_writeRegisterGroup(BTM_command_t command, uint8_t tx_data[][BTM_REG_GROUP_SIZE])
{
	uint16_t pecValue = 0;
	uint8_t tx_message[8];

	HAL_GPIO_WritePin(BTM_CS_GPIO_PORT, BTM_CS_GPIO_PIN, 0); // pull CS line low
	BTM_sendCmd(command);
	for (int i = 0; i < BTM_NUM_DEVICES; i++)
	{
		for (int j = 0; j < BTM_REG_GROUP_SIZE; j++)
		{
			tx_message[j] = tx_data[i][j];
		}
		pecValue = BTM_calculatePec15(tx_message, BTM_REG_GROUP_SIZE);
		tx_message[6] = (uint8_t) (pecValue >> 8);
		tx_message[7] = (uint8_t) pecValue;
		HAL_SPI_Transmit(BTM_SPI_handle, tx_message, 8, BTM_TIMEOUT_VAL);
	}
	HAL_GPIO_WritePin(BTM_CS_GPIO_PORT, BTM_CS_GPIO_PIN, 1); // pull CS line back high
	return;
}

/**
 * @brief Writes the 6 bytes of a configuration register group in the LTC6811
 *
 * The data received will only be written to rx_data if the PEC matches.
 *
 * @param command 	A read command to specify which register group to read.
 * 					Read commands start with "RD"
 * @param rx_data 	Pointer to a 2-dimensional array of size
 *					BTM_NUM_DEVICES x 6 to copy received data to
 * @return 	Returns BTM_OK if the received PEC is valid, or BTM_ERROR_PEC if
 			a full set of valid data could not be obtained after
			BTM_MAX_READ_ATTEMPTS tries
 */
BTM_Status_t BTM_readRegisterGroup(BTM_command_t command, uint8_t rx_data[][BTM_REG_GROUP_SIZE])
{
	uint16_t pecValue = 0;
	BTM_Status_t status = BTM_OK;
	// Initialize rx_message before using it, or the garbage it contains will be
	// sent as dummy data - see definition of HAL_SPI_Receive
	uint8_t rx_message[8] = {0};
	int ic_num = 0;
	int error_counter = 0;

	// Try a maximum of BTM_MAX_READ_ATTEMPTS times to read register group
	do
	{
		// Send command to read register group
		HAL_GPIO_WritePin(BTM_CS_GPIO_PORT, BTM_CS_GPIO_PIN, 0); // pull CS line low
		BTM_sendCmd(command);

		// Read back the data, but stop between device data groups on error
		ic_num = 0;
		status = BTM_OK; // reset status before a new try
		while ((ic_num < BTM_NUM_DEVICES) && (BTM_OK == status))
		{
			// 6 data bytes + 2 PEC bytes = 8 bytes
			HAL_SPI_Receive(BTM_SPI_handle, rx_message, 8, BTM_TIMEOUT_VAL);

			pecValue = BTM_calculatePec15(rx_message, 8); // 0 if transfer was clean
			if (pecValue)
			{
				status = BTM_ERROR_PEC;
				for(int i = 0; i < 8; i++)
				{
					rx_message[i] = 0; // Clear buffer for next try
				}
			}
			else
			{
				for (int j = 0; j < 8; j++)
				{
					if (j < BTM_REG_GROUP_SIZE)
					{
						rx_data[ic_num][j] = rx_message[j]; // Copy the data (no PEC)
					}
					rx_message[j] = 0; // Clear rx_message for next loop
				}
			}
			ic_num++;
		}

		// pull CS line back high
		HAL_GPIO_WritePin(BTM_CS_GPIO_PORT, BTM_CS_GPIO_PIN, 1);
		error_counter++;
	} while ((BTM_OK != status) && (error_counter < BTM_MAX_READ_ATTEMPTS));

	return status;
}

/**
 * @brief Poll ADCs and read registers of LTC6811 for cell voltages
 *
 * @param voltages 	Pointer to a 2-dimensional array of size
 *					BTM_NUM_DEVICES x 12 to fill with cell voltages C1-C12 of
 * 					of each LTC6811 in the chain (nearest to MCU first)
 * @return 	Returns BTM_OK if all the received PECs are correct,
 * 			BTM_ERROR_PEC if any PEC doesn't match, or BTM_ERROR_TIMEOUT
 *			if a timeout occurs while polling.
 */
BTM_Status_t BTM_readBatt(BTM_PackData_t * packData)
{
	// 6-byte sets (each from a different register group of the LTC6811)
	uint8_t ADC_data[4][BTM_NUM_DEVICES][BTM_REG_GROUP_SIZE];
	uint16_t cell_voltage_raw = 0;
	int cell_num = 0;
	BTM_Status_t status = BTM_OK;

	status = BTM_sendCmdAndPoll(CMD_ADCV);
	if (status != BTM_OK) return status;

	status = BTM_readRegisterGroup(CMD_RDCVA, ADC_data[0]);
	if (status != BTM_OK) return status;

	status = BTM_readRegisterGroup(CMD_RDCVB, ADC_data[1]);
	if (status != BTM_OK) return status;

	status = BTM_readRegisterGroup(CMD_RDCVC, ADC_data[2]);
	if (status != BTM_OK) return status;

	status = BTM_readRegisterGroup(CMD_RDCVD, ADC_data[3]);
	if (status != BTM_OK) return status;

	// Each cell voltage is provided as a 16-bit value where
	// voltage = 0.0001V * raw value
	// Each 6-byte Cell Voltage Register Group holds 3 cell voltages
	// 1st 2 bytes of Cell Voltage Register Group A is C1V
	// Last 2 bytes of Cell Voltage Register Group D is C12V
	for (int ic_num = 0; ic_num < BTM_NUM_DEVICES; ic_num++)
	{
		for (int reg_group = 0; reg_group < 4; reg_group++)
		{
			for (int reading_num = 0; reading_num < 3; reading_num++)
			{
				// Combine the 2 bytes of each cell voltage together
				cell_voltage_raw =
					((uint16_t) (ADC_data[reg_group][ic_num][2 * reading_num + 1]) << 8)
					| (uint16_t) (ADC_data[reg_group][ic_num][2 * reading_num]);
				// Store in pack data structure
				cell_num = 3 * reg_group + reading_num;
				//voltages[ic_num][cell_num] = cell_voltage_raw;
				packData->stack[ic_num].module[cell_num].voltage = cell_voltage_raw;
			}
		}
	}
	return status;
}

/**
 * @brief Converts a voltage reading from a register in the LTC6811 to a float
 * Each cell voltage is provided as a 16-bit value where
 * voltage = 0.0001V * raw value
 *
 * @param raw_reading The 16-bit reading from an LTC6811
 * @return Returns a properly scaled floating-point version of raw_reading
 */
float BTM_regValToVoltage(uint16_t raw_reading)
{
	return raw_reading * BTM_VOLTAGE_CONVERSION_FACTOR;
}

/*
The pec15Table lookup table was generated using the following code, modified
from the example code on pg. 76 of the LTC6811 Datasheet:

#define CRC15_POLY 0x4599

void init_PEC15_Table() {
    unsigned short int remainder = 0;
    for (int i = 0; i < 256; i++) {
        remainder = i << 7;
        for (int bit = 8; bit > 0; --bit) {
            if (remainder & 0x4000) {
                remainder = ((remainder << 1));
                remainder = (remainder ^ CRC15_POLY);
            }
            else {
                remainder = ((remainder << 1));
            }
        }
        pec15Table[i] = remainder & 0xFFFF;
    }
}
*/
