#include "ltc6811_btm_temp.h"
#include "stm32f3xx_hal.h"
#include <math.h>

// Function prototypes:
BTM_Status_t read_and_switch_mux_channels(
	uint8_t mux_address,
	uint16_t MUX_thermistor_readings[MUX_CHANNELS][BTM_NUM_DEVICES]
);
BTM_Status_t readThermistorVoltage(uint16_t GPIO1_voltage[BTM_NUM_DEVICES]);
void disableMux(uint8_t mux_address);


/*
Function name: BTM_TEMP_measureState
Purpose: Algorithm for reading thermistors across multiple mux (2 mux's)

input:
No user defined input.
Internal functions pull readings from hardware registers

Output:
Voltages of thermistors stored in the given pack data structure
ie. pack.stack[].module[].temperature attribute filled for all modules.

Returns:
BTM_Status_t Status from communication functions.
Note: nothing new written to pack if return value != BTM_OK

Internal functions:
read_and_switch_mux_channels()
disableMux()
readThermistorVoltage()

Algorithm:
0) Initialize
0.1) disable all mux for known reset state

1) read mux1
1.1) set mux_address or COMM1 bits for mux1
1.2) call read_and_switch_mux_channels()
1.2.1) call readThermistorVoltage() six times
1.2.2) disable mux1

2) read mux2
2.1) set mux_address or COMM1 bits for mux2
2.2) call read_and_switch_mux_channels()
2.2.1) call readThermistorVoltage() six times
2.2.2) disable mux2

3) done
*/
BTM_Status_t BTM_TEMP_measureState(BTM_PackData_t* pack)
{
	uint16_t MUX_thermistor_readings[NUMBER_OF_MUX][MUX_CHANNELS][BTM_NUM_DEVICES] = { 0 };
	int module_num = 0;
	BTM_Status_t status = {BTM_OK, 0};

	//known reset state
	disableMux(MUX1_ADDRESS);
	disableMux(MUX2_ADDRESS);

	//mux channel-switching per mux
	status = read_and_switch_mux_channels(MUX1_ADDRESS, MUX_thermistor_readings[0]);
	if (status.error != BTM_OK) return status; // There was a communication problem at some point.

	status = read_and_switch_mux_channels(MUX2_ADDRESS, MUX_thermistor_readings[1]);
	if (status.error != BTM_OK) return status; // There was a communication problem at some point.

	// Copy gathered temperature data to pack data structure
	for (int ic_num = 0; ic_num < BTM_NUM_DEVICES; ic_num++)
	{
		for(int mux_num = 0; mux_num < NUMBER_OF_MUX; mux_num++)
		{
			for(int mux_channel = 0; mux_channel < MUX_CHANNELS; mux_channel++)
			{
				module_num = 6 * mux_num + mux_channel;
				pack->stack[ic_num].module[module_num].temperature =
					MUX_thermistor_readings[mux_num][mux_channel][ic_num];
			}
		}
	}


	// //updated pack data struct loop for descending order modules
	// for(int ic_num = 0; ic_num < BTM_NUM_DEVICES; ++ic_num)
	// {
	// 	for(int mux_num = 0; mux_num < NUMBER_OF_MUX; ++mux_num)
	// 	{
	// 		for(int mux_channel = 0; mux_channel < MUX_CHANNELS; ++mux_channel)
	// 		{
	// 			module_num = (MUX_CHANNELS - 1) * (mux_num + 1) - mux_channel; //or (MUX_CHANNELS - 1)
	// 			pack->stack[ic_num].module[module_num].temperature =
	// 				MUX_thermistor_readings[mux_num][mux_channel][ic_num];
	// 		}
	// 	}
	// }

	//for 12 modules


	return status;
}

/*
Function name: read_and_switch_mux_channels
Purpose:
switch between all 8 channels of a single mux per lTC6811
and gather the associated themistor readings per LTC6811

input:
uint8_t mux_address
	- the exact address bits
uint16_t MUX_thermistor_readings[MUX_CHANNELS][BTM_NUM_DEVICES]
	- the array of all thermistor voltage readings from all mux channels from all LTC6811 devices
internal functions pull readings from hardware registers

output: None. Readings are put into an array. See above.


Algorithm:
1) Define comm_val array of 6 bytes to be copied to COMM register of LTC6811.
	The LTC1380 MUX needs 2 bytes for a single Send Byte protocol.
	The two bytes are enveloped by I2C commands for LTC6811.
	The full message totals to 4 bytes.
	Last two bytes of COMM are not needed.
2) In a loop for MUX_CHANNELS cycles, set the MUX channel and measure its voltage:
2.1) Set the appropriate comm_val bits to the mux channel to switch to.
2.2) Copy comm_val to mux_message. Mux message has additional dimension for LTC6811 device specified.
2.3) WRCOMM: mux_message from STM32 is written to the COMM register of all lTC6811 devices.
2.4) Chip Select is pulled Low (it signals to allow SPI communication from the STM32 to the 6811)
2.5) STCOMM: LTC6811 is sent the command to send an I2C message to the MUX. The message is stored in COMM register.
2.6) the STM32 is purposefully stalled with a NULL_message SPI transmission going from STM32 to LTC6811
	 for exactly 3 clock cycles per bit of I2C transmission going from LTC6811 to MUX.
			 (needs testing but we suspect the I2C output of the 6811 uses SPI as a clock input)
2.7) Chip Select is pulled high (it signals to block SPI communication from the STM32 to the 6811)
2.8) Measure the voltage.

COMM register details:

See LTC6811 data sheet table 49 for the COMM Register Group table.
See LTC1380 data sheet section titled "serial interface" for the "send byte protocol" on page 8.

WRCOMM: command for writing to COMM register of LTC6811
STCOMM: send the COMM register content of LTC6811 across I2C

Address half of COMM
1.1) ICOMn[3:0] = I2C_START
1.2) D0[7:4] = 0x9, (Upper 4 bits of mux I2C address)
1.3) D0[3:0] = Lower bits 3 to 1 of mux I2C address and bit 0 is equal to 0 for write
1.4) FCOMn[3:0] = I2C_MASTER_NACK
Note: you release the bus on the acknowledge bit, so the mux can acknowledge.
A NACK is a high bit, and an open drain output going high is the same as releasing the bus.

Command half of COMM
1.5) ICOMn[3:0] = I2C_BLANK
1.6) D1[7:4] = 0x0
1.7) D1[3:0] = MUX_Sn
1.8) FCOMn[3:0] = I2C_MASTER_NACK_STOP


Debugging uncertainty:
there is uncertainty about if the acknowledge bits (detailed in LTC1380 data sheet) has to be processed by our code. I assume not,
but in the case where I2C is not working, this is a potential cause.
*/
BTM_Status_t read_and_switch_mux_channels(uint8_t mux_address,
	uint16_t MUX_thermistor_readings[MUX_CHANNELS][BTM_NUM_DEVICES])
{
	uint8_t MUX_S[8] = { MUX_S0, MUX_S1, MUX_S2, MUX_S3, MUX_S4, MUX_S5, MUX_S6, MUX_S7 };
	uint8_t NULL_message[6] = { 0 };

	uint8_t comm_val[BTM_REG_GROUP_SIZE] =
	{
		(I2C_START << 4) | (mux_address >> 4), // 1.1.1) - 1.1.2)
		(mux_address << 4) | I2C_MASTER_NACK,
		0, // ((I2C_BLANK << 4) | 0x00) = 0, First nibble of mux command is 0
		I2C_MASTER_NACK_STOP, // mux command in upper nibble is set in loop
		0, // There's no 3rd byte to transmit
		0
	};

	uint8_t mux_message[BTM_NUM_DEVICES][BTM_REG_GROUP_SIZE] = { 0 };
	BTM_Status_t status = {BTM_OK, 0};

	//2)
	for (int n = 0; n < MUX_CHANNELS; n++)
	{
		//2.1)mux channel switching
		//    1.7) D1[3:0] = MUX_Sn
		//    1.8) FCOMn[3:0] = I2C_MASTER_NACK_STOP
		comm_val[3] = MUX_S[n] << 4; // bit shifted value of MUX_S[n] == 0bXXXX_0000
		comm_val[3] |= I2C_MASTER_NACK_STOP;

		//2.2)
		// Make enough copies of this data to send to all the LTC6811's
		for(int ic_num = 0; ic_num < BTM_NUM_DEVICES; ic_num++)
		{
			for(int byte_num = 0; byte_num < BTM_REG_GROUP_SIZE; byte_num++)
			{
				mux_message[ic_num][byte_num] = comm_val[byte_num];
			}
		}
		//2.3) WRCOMM: mux_message from STM32 is written to the COMM register of all lTC6811 devices.
		BTM_writeRegisterGroup(CMD_WRCOMM, mux_message);

		//2.4)
		BTM_writeCS(CS_LOW);

		//2.5) STCOMM: LTC6811 is sent the command to send an I2C message to the MUX.
		BTM_sendCmd(CMD_STCOMM);

		// 2.6) driving SPI clock needed for I2C, 3 clock cycles per bit sent
		//     can be done by sending null data, i.e. NULL_message
		//     BTM_SPI_handle is a global variable,
		//     and BTM_TIMEOUT_VAL is a symbolic constant
		HAL_SPI_Transmit(BTM_SPI_handle, NULL_message, 6, BTM_TIMEOUT_VAL);

		//2.7)
		BTM_writeCS(CS_HIGH);

		//2.8) gather thermistor readings
		status = readThermistorVoltage(MUX_thermistor_readings[n]);
		if (status.error != BTM_OK)
		{
			// There's a communication problem
			disableMux(mux_address); // Just to be safe. May not do anything if communication is down.
			return status;
		}
	}

	disableMux(mux_address);
	return status;
}

/*
Function name: readThermistorVoltage
Purpose: get voltage reading of thermistors directly from GPIO pin per LTC6811, and output into a register

input:
uint16_t GPIO1_voltage[BTM_NUM_DEVICES] - two bytes long as it is the concatenation of two register bytes.
For each LTC6811 device, internal functions pull voltage readings of GPIO pin from hardware registers.

output:
- Returns status value indicating success of register read.
- Readings stored in array. See input.
*/
BTM_Status_t readThermistorVoltage(uint16_t GPIO1_voltage[BTM_NUM_DEVICES])
{
	uint8_t registerAUXA_voltages[BTM_NUM_DEVICES][BTM_REG_GROUP_SIZE];
	uint16_t voltage_reading = 0;
	BTM_Status_t status = {BTM_OK, 0};
	//start conversion
	//ADAX, ADc AuXillary start-conversion command
	status = BTM_sendCmdAndPoll(CMD_ADAX_GPIO1);
	if (status.error != BTM_OK) return status; // There's a communication problem

	//retrieve register readings
	status = BTM_readRegisterGroup(CMD_RDAUXA, registerAUXA_voltages);
	if (status.error != BTM_OK) return status; // There's a communication problem

	//output reading by assigning to pointed array the first two bytes of registerAUXA_voltages
	for (int board = 0; board < BTM_NUM_DEVICES; board++)
	{
		// Combine 2 bytes of voltage reading
		voltage_reading = ( ((uint16_t) registerAUXA_voltages[board][1]) << 8)
			| registerAUXA_voltages[board][0];
		// Store in given array
		GPIO1_voltage[board] = voltage_reading;
	}

	return status;
}

/*
Function Name: disableMux
Input: mux_address - address of MUX to disable
NOTE: this takes code directly from another function, read_and_switch_mux_channels().
*/
void disableMux(uint8_t mux_address)
{
	uint8_t comm_val[BTM_REG_GROUP_SIZE] =
	{
		(I2C_START << 4) | (mux_address >> 4), // 1.1.1) - 1.1.2)
		(mux_address << 4) | I2C_MASTER_NACK,
		0, // ((I2C_BLANK << 4) | 0x00) = 0, First nibble of mux command is 0
		(MUX_DISABLE << 4) | I2C_MASTER_NACK_STOP,
		0, // There's no 3rd byte to transmit
		0
	};

	uint8_t mux_message[BTM_NUM_DEVICES][BTM_REG_GROUP_SIZE] = { 0 };
	uint8_t null_message[6] = { 0 };

	// Make enough copies of this data to send to all the LTC6811's
	for(int ic_num = 0; ic_num < BTM_NUM_DEVICES; ic_num++)
	{
		for(int byte_num = 0; byte_num < BTM_REG_GROUP_SIZE; byte_num++)
		{
			mux_message[ic_num][byte_num] = comm_val[byte_num];
		}
	}
	//sends from STM32 to 6811 for 6811 to send to MUX the I2C command
	BTM_writeRegisterGroup(CMD_WRCOMM, mux_message);


	BTM_writeCS(CS_LOW);
	//STCOMM: send the COMM register content
	BTM_sendCmd(CMD_STCOMM);
	// driving SPI clock needed for I2C, 3 clock cycles per bit sent
	// can be done by sending null data
	// BTM_SPI_handle is a global variable,
	//  and BTM_TIMEOUT_VAL is a symbolic constant
	HAL_SPI_Transmit(BTM_SPI_handle, null_message, 6, BTM_TIMEOUT_VAL);
	BTM_writeCS(CS_HIGH);
}

/**
 * @brief Converts a raw thermistor voltage reading from an LTC6811 into a temperature
 *
 * @param[in] Vout the thermistor voltage reading to convert
 * @return the temperature of the thermistor in degrees celcius
 */
double BTM_TEMP_volts2temp(double Vout)
{
	const double Vs = 5.0; // assuming the supply is 5V - measure Vref2 to check
	const double beta = 3435.0;
	const double room_temp = 298.15;
	const double R_balance = 10000.0; //from LTC6811 datasheet p.85. note: this doesn't account for tolerance. to be exact, measure the 10k resistor with a multimeter
	const double R_room_temp = 10000.0; //resistance at room temperature (25C)
	double R_therm = 0;
	double temp_kelvin = 0;
	double temp_celsius = 0;

	// to get the voltage in volts from the LTC6811's value,
	// multiply it by 0.0001 as below.
	R_therm = R_balance * ((Vs / (Vout * 0.0001)) - 1);
	temp_kelvin = (beta * room_temp)
		/ (beta + (room_temp * log(R_therm / R_room_temp)));
	return temp_celsius = temp_kelvin - 273.15;
}
