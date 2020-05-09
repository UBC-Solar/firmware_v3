#include "thermistor.h"
#include <math.h>


void read_and_switch_mux_channels(
	uint8_t mux_address,
	uint16_t thermistor_volt_series[MUX_CHANNELS][BTM_NUM_DEVICES]
);
void thermistor_reading(uint16_t GPIO1voltages[BTM_NUM_DEVICES]);

//assumption: a single reading from each slave board's LTC6811
// ^ That's not what you're doing here... is this an old note?

/*
Function name: BTM_TEMP_measure_state
Purpose: Algorithm for reading thermistors across multiple mux (2 mux's)

input:
No user defined input.
Internal functions pull readings from hardware registers

Output:
uint8_t MUX_thermistor_readings[ONE_THIRD_OF_COMM][MUX_CHANNELS][BTM_NUM_DEVICES]
    ONE_THIRD_OF_COMM - first two bytes of 6 bytes of LTC6811 COMM register
    MUX_CHANNELS      - LTC1380 MUX have 8 channels
    BTM_NUM_DEVICES   - Number of LTC6811 connected to STM32F3 mcu.

Internal functions:
read_and_switch_mux_channels()
disableMux()
read_thermistorVoltage()

Algorithm:
0) Initialize
0.1) disable all mux for known reset state

1) read mux1
1.1) set mux_address or COMM1 bits for mux1
1.2) call read_and_switch_mux_channels()
1.2.1) call read_thermistorVoltage() six times
1.2.2) disable mux1

2) read mux2
2.1) set mux_address or COMM1 bits for mux2
2.2) call read_and_switch_mux_channels()
2.2.1) call read_thermistorVoltage() six times
2.2.2) disable mux2

3) done
*/
void BTM_TEMP_measure_state()
{
	uint16_t MUX_thermistor_readings[ONE_THIRD_OF_COMM][MUX_CHANNELS][BTM_NUM_DEVICES] = { 0 };


	//known reset state
	disableMux(MUX1_ADDRESS);
	disableMux(MUX2_ADDRESS);

	//mux channel-switching per mux
	read_and_switch_mux_channels(MUX1_ADDRESS, MUX_thermistor_readings[0]);
	read_and_switch_mux_channels(MUX2_ADDRESS, MUX_thermistor_readings[1]);
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
void read_and_switch_mux_channels(uint8_t mux_address,
	uint16_t MUX_thermistor_readings[MUX_CHANNELS][BTM_NUM_DEVICES])
{
	uint8_t MUX_S[8] = { MUX_S0, MUX_S1, MUX_S2, MUX_S3, MUX_S4, MUX_S5, MUX_S6, MUX_S7 };
	uint8_t NULL_message[6] = { 0 };
	uint8_t comm_val[6] =
	{
		(I2C_START << 4) | (mux_address >> 4), // 1.1.1) - 1.1.2)
		(mux_address << 4) | I2C_MASTER_NACK,
		0, // ((I2C_BLANK << 4) | 0x00) = 0, First nibble of mux command is 0
		I2C_MASTER_NACK_STOP, // mux command in upper nibble is set in loop
		0, // There's no 3rd byte to transmit
		0
	};
	uint8_t mux_message[BTM_NUM_DEVICES][6] = { 0 };

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
			for(int byte_num = 0; byte_num < 6; byte_num++)
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
		read_thermistorVoltage(MUX_thermistor_readings[n]);
	}

	disableMux(mux_address);
}

/*
Function name: read_thermistorVoltage
Purpose: get voltage readings of thermistors and output into a register
input: internal functions pull readings from hardware registers
output: uint16_t GPIO1_voltage, two bytes long because that's how long it is.
*/

/** EXAMPLE OF DOXYGEN
 * @brief Get voltage reading of thermistor GPIO pin on LTC6811
 *
 * @param GPIO1_voltage array to store voltages in
 */
void read_thermistorVoltage(uint16_t GPIO1_voltage[BTM_NUM_DEVICES])
{
	uint8_t registerAUXA_voltages[BTM_NUM_DEVICES][6];
	uint16_t voltage_reading = 0;

	//start conversion
	//ADAX, ADc AuXillary start-conversion command
	BTM_sendCmdAndPoll(CMD_ADAX_GPIO1);

	//retrieve register readings
	BTM_readRegisterGroup(CMD_RDAUXA, registerAUXA_voltages);

	//output reading by assigning to pointed array the first two bytes of registerAUXA_voltages
	for (int board = 0; board < BTM_NUM_DEVICES; board++)
	{
		// Combine 2 bytes of voltage reading
		voltage_reading = ( ((uint16_t) registerAUXA_voltages[board][1]) << 8)
			| registerAUXA_voltages[board][0];
		// Store in given array
		GPIO1_voltage[board] = voltage_reading;
	}
}

/*
*Function Name: disableMux
*Input: mux_address - address of MUX to disable
*NOTE: this takes code from another function, read_and_switch_mux_channels().
*
*/
void disableMux(uint8_t mux_address)
{
	uint8_t comm_val[6] =
	{
		(I2C_START << 4) | (mux_address >> 4), // 1.1.1) - 1.1.2)
		(mux_address << 4) | I2C_MASTER_NACK,
		0, // ((I2C_BLANK << 4) | 0x00) = 0, First nibble of mux command is 0
		I2C_MASTER_NACK_STOP, // mux command in upper nibble is set in loop
		0, // There's no 3rd byte to transmit
		0
	};

	uint8_t mux_message[BTM_NUM_DEVICES][6] = { 0 };


	//setting disable command in the correct byte
	comm_val[3] = MUX_DISABLE << 4;
	comm_val[3] |= I2C_MASTER_NACK_STOP;

	// Make enough copies of this data to send to all the LTC6811's
	for(int ic_num = 0; ic_num < BTM_NUM_DEVICES; ic_num++)
	{
		for(int byte_num = 0; byte_num < 6; byte_num++)
		{
			mux_message[ic_num][byte_num] = comm_val[byte_num];
		}
	}
	//sends from STM32 to 6811 for 6811 to send to MUX the I2C command
	BTM_writeRegisterGroup(CMD_WRCOMM, mux_message);

	// 1.2) STCOMM: send the COMM register content
	BTM_writeCS(CS_LOW);
	BTM_sendCmd(CMD_STCOMM);
	// driving SPI clock needed for I2C, 3 clock cycles per bit sent
	// can be done by sending null data
	// BTM_SPI_handle is a global variable,
	// 	and BTM_TIMEOUT_VAL is a symbolic constant
	HAL_SPI_Transmit(BTM_SPI_handle, NULL_message, 6, BTM_TIMEOUT_VAL);
	BTM_writeCS(CS_HIGH);
}

/*
Comment on function volts2temp()
This function does not conform to the actual format of the input values it will likely get.
The format of the measured values from the LTC6811 are 16-bit unsigned integer values,
but are not raw ADC readings in the range 0 to (2^16 - 1). Instead, they are actual voltages
with unit 10ths of a mV. In other words, to get the voltage in volts from the reading value,
you just have to multiply it by 0.0001.

Also, perhaps you should return something... */
void volts2temp(int ADC_val)
{
	const double beta = 3435.0;
	const double max_ADC = 65535.0; //assuming ADC @ 16 bit
	const double room_temp = 298.15;
	const double R_balance = 10000.0; //from LTC6811 datasheet p.85. note: this doesn't account for tolerance. to be exact, measure the 10k resistor with a multimeter
	const double R_room_temp = 10000.0; //resistance at room temperature (25C)
	//const int sample_num = 10;
	double R_therm = 0;
	double temp_kelvin = 0;
	double temp_celsius = 0;
	//double adc_average = 0; // we will do multiple measurements and average them to eliminate errors
	//int adc_samples[sample_num];

	R_therm = R_balance * ((max_ADC / ADC_val) - 1);
	temp_kelvin = (beta * room_temp)
		/ (beta + (room_temp * log(R_therm / R_room_temp)));
	temp_celsius = temp_kelvin - 273.15;
}
