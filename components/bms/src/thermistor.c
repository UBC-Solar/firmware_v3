#include "thermistor.h"
#include <math.h>

//assumption: a single reading from each slave board's LTC6811
// ^ That's not what you're doing here... is this an old note?

/*
Function name: thermistor_series
Purpose: Algorithm for reading thermistors across multiple mux (2 mux's)
input:
No user defined input.
Internal functions pull readings from hardware registers
Output:
uint8_t thermistor_volt_series[2][6], 2 sets of 6 voltages
Internal functions:
thermistor_mux()
thermistor_reading()
*/
void thermistor_series()
{
	uint16_t thermistor_volt_series[ONE_THIRD_OF_COMM][MUX_CHANNELS][BTM_NUM_DEVICES] = { 0 };
	/*
	Algorithm:
	0) Initialize
	1) read mux1
	1.1) set mux_address or COMM1 bits for mux1
	1.2) call thermistor_mux()
	1.3) call thermistor_reading() SIX TIMES
	2) read mux2
	1.1) set mux_address or COMM1 bits for mux2
	1.2) call thermistor_mux()
	1.3) call thermistor_reading() SIX TIMES
	3) done
	*/

	// TODO: disable inactive mux before using the active one
	thermistor_mux(MUX1_ADDRESS, thermistor_volt_series[0]);
	thermistor_mux(MUX2_ADDRESS, thermistor_volt_series[1]);
}

/*
Function name: thermistor_mux
Purpose:
switch between all 12 channels OF A SINGLE MUX
and gather the associated themistor reading
input:
User defined:
COMM1, the exact address bits
internal functions pull readings from hardware registers
output: uint8_t thermistor_volt_array[12][2], 12 sets of data of 2 bytes
*/

/* Algorithm of function (Note: this function is for a single mux)
steps:
0) known reset state: send to mux the enable signal?????
1) control MUX with COMM registers
1.1) WRCOMM: write to COMM register:
Address half:
1.1.1) ICOMn[3:0] = I2C_START
1.1.2) D0[7:4] = 0x9, (Upper 4 bits of mux I2C address)
1.1.3) D0[3:0] = Lower 3 bits of mux I2C address and a 0 for write
1.1.4) FCOMn[3:0] = I2C_NACK
Note: you release the bus on the acknowledge bit, so the mux can acknowledge
A NACK is a high bit, and an open drain output going high is the same as releasing the bus
Command half:
1.1.5) ICOMn[3:0] = I2C_BLANK
1.1.6) D1[7:4] = 0x0
1.1.7) D1[3:0] = MUX_Sn
1.1.8) FCOMn[3:0] = I2C_MASTER_NACK_STOP
1.2) STCOMM: send the COMM register content
1.2.1) read acknowledge bit???????? skip
1.2.2) wait for appropriate clock cycles to pass using HAL_SPI_Transmit(NULLxsomething, sweet_nothings, idk)
2) read GPIO1 for thermistor reading FOR each mux channel
3) repeat until n = MUX_CHANNELS
*/
void thermistor_mux(uint8_t mux_address,
	uint16_t thermistor_volt_series[MUX_CHANNELS][BTM_NUM_DEVICES])
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

	//1.1) write to COMM register
	for (int n = 0; n < MUX_CHANNELS; n++)
	{
		// Address half (same every loop, set above):
		// 1.1.1) ICOMn[3:0] = I2C_START
		// 1.1.2) D0[7:4] = 0x9, (Upper 4 bits of mux I2C address)
		// 1.1.3) D0[3:0] = Lower 3 bits of mux I2C address and a 0 for write
		// 1.1.4) FCOMn[3:0] = I2C_NACK
		// Command half:
		// 1.1.5) ICOMn[3:0] = I2C_BLANK (same every loop, set above)
		// 1.1.6) D1[7:4] = 0x0 (same every loop, set above)

		//mux channel switching
		//1.1.7) D1[3:0] = MUX_Sn
		//1.1.8) FCOMn[3:0] = I2C_MASTER_NACK_STOP
		comm_val[3] = MUX_S[n] << 4; // bit shifted value of MUX_S[n] == 0bXXXX_0000
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

		//gather thermistor readings
		thermistor_reading(thermistor_volt_series[n]);
	}
}

/*
Function name: thermistor_reading
Purpose: get voltage readings of thermistors and output into a register
input: internal functions pull readings from hardware registers
output: uint16_t GPIO1voltages, two bytes long because that's how long it is.
*/
void thermistor_reading(uint16_t GPIO1voltages[BTM_NUM_DEVICES])
{
	uint8_t GPIO123voltages[BTM_NUM_DEVICES][6];
	uint16_t voltage_reading = 0;

	//start conversion
	//ADAX, ADc AuXillary start-conversion command
	BTM_sendCmdAndPoll(CMD_ADAX_GPIO1);

	//retrieve register readings
	BTM_readRegisterGroup(CMD_RDAUXA, GPIO123voltages);

	//output reading by assigning to pointed array the first two bytes of GPIO123voltages
	for (int board = 0; board < BTM_NUM_DEVICES; board++)
	{
		// Combine 2 bytes of voltage reading
		voltage_reading = ( ((uint16_t) GPIO123voltages[board][1]) << 8)
			| GPIO123voltages[board][0];
		// Store in given array
		GPIO1voltages[board] = voltage_reading;
	}
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
