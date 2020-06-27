#include <stdio.h>
#include <stdint.h>
#include "CANbus_TESTING_ONLY.h"
//#include "CANbus_functions.c"


void main()
{
	BTM_PackData_t PH_VS_PACKDATA;
	PH_VS_PACKDATA.pack_voltage = 111222333;
	printf("pack voltage: %i \r\n", PH_VS_PACKDATA.pack_voltage);

	for (int device_i = 0; device_i < BTM_NUM_DEVICES; ++device_i)
	{
		for (int module_i = 0; module_i < BTM_NUM_MODULES; ++module_i)
		{
			PH_VS_PACKDATA.stack[device_i].module[module_i].enable      = MODULE_ENABLED;
			PH_VS_PACKDATA.stack[device_i].module[module_i].voltage     = 2.2;
			PH_VS_PACKDATA.stack[device_i].module[module_i].temperature = 3;
			PH_VS_PACKDATA.stack[device_i].module[module_i].bal_status  = DISCHARGE_ON;
			uint16_t* pPD_enable = &PH_VS_PACKDATA.stack[device_i].module[module_i].enable;
			uint16_t* pPD_voltage = &PH_VS_PACKDATA.stack[device_i].module[module_i].voltage;
			uint16_t* pPD_temperature = &PH_VS_PACKDATA.stack[device_i].module[module_i].temperature;
			uint16_t* pPD_bal_status = &PH_VS_PACKDATA.stack[device_i].module[module_i].bal_status;
			printf("device: %2.1i  module: %2.1i  enable: %2.1i  voltage: %2.1i  temp.: %2.1i  bal_status: %2.1i\r\n", 
				device_i,
				module_i,
				*pPD_enable,
				*pPD_voltage,
				*pPD_temperature,
				*pPD_bal_status);
		};
	};


	printf("Hello world!\r\n\r\nBiscuits.\r\n\r\n");

	//simple functions calls to ensure the functions are linked properly in Visual Studio
	

	//function-wise testing

	//full function-chain testing


	//system("pause");
	return;

}

/*
	Copied from ltc6811_btm.c, created by UBC Solar.
*/

#define BTM_VOLTAGE_CONVERSION_FACTOR 0.0001

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
	End of copy.
*/

/*
	Copied from thermistor.h, created by UBC Solar.
/*

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

/*
	End of copy.
*/