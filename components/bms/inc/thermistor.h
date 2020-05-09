#ifndef THERMISTOR_H_
#define THERMISTOR_H_

#include "stm32f3xx_hal.h"
#include "ltc6811_btm.h"

//for I2C communication
#define MUX1_ADDRESS 0x90
#define MUX2_ADDRESS 0x92
// these include a 0 as LSB to signify I2C "write" operation

//Table 19: Write Codes for ICOMn[3:0] and FCOMn[3:0] on I2C Master
//ICOMn[3:0]
typedef enum {
	I2C_START = 0b0110,
	I2C_STOP  = 0b0001,
	I2C_BLANK = 0b0000,
	I2C_NO_TRANSMIT = 0b1111
} BTM_ICOM_t;

//FCOMn[3:0]
typedef enum {
	I2C_MASTER_ACK = 0b0000,
	I2C_MASTER_NACK = 0b1000,
	I2C_MASTER_NACK_STOP = 0b1001
} BTM_FCOM_t;
//MUX Control Codes for I2C control of multiplexer
//Table 2. Multiplexer Control Bits Truth Table
//sidenote: could drop 4th bit (first from left) of MUX_Sn,
//then concatanate with MUX_ENABLE set to 1 bit 0b0
#define MUX_DISABLE 0b0000
#define MUX_S0      0b1000
#define MUX_S1      0b1001
#define MUX_S2      0b1010
#define MUX_S3      0b1011
#define MUX_S4      0b1100
#define MUX_S5      0b1101
#define MUX_S6      0b1110
#define MUX_S7      0b1111

#define MUX_CHANNELS 6
#define NUMBER_OF_MUX 2
#define ONE_THIRD_OF_COMM 2

// Function prototypes
void BTM_TEMP_measure_state(void);

void volts2temp(int ADC_val);

#endif /* THERMISTOR_H_ */
