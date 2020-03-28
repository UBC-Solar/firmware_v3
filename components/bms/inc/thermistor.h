//header file for thermistor.c
#ifndef THERMISTOR_H
#define THERMISTOR_H

//TO MOVE TO 6811.h
#define LTC_CMD_WRCOMM      0b00000_111_0010_0001
#define LTC_CMD_RDCOMM      0b00000_111_0010_0010
#define LTC_CMD_STCOMM      0b00000_111_0010_0011

//Normal mode, gpio1
#define LTC_CMD_ADAX_GPIO1  0b00000_01_10_1100_001
#define LTC_CMD_RDAUXA      0b00000_000_0000_1100

//for I2C communication
//Table 19: Write Codes for ICOMn[3:0] and FCOMn[3:0] on I2C Master
//DOUBLE CHECK if cubeMX generates these already
#define I2C_START       0b0110
#define I2C_STOP        0b0001
#define I2C_BLANK       0b0000
#define I2C_NO_TRANSMIT 0b1111

//MUX Control Codes for I2C control of multiplexer
//Table 2. Multiplexer Control Bits Truth Table
//sidenote: could drop 4th bit (first from left) of MUX_Sn,
//then concatanate with MUX_ENABLE set to 1 bit 0b0
#define MUX_ENABLE  0b0000
#define MUX_S0      0b1000
#define MUX_S1      0b1001
#define MUX_S2      0b1010
#define MUX_S3      0b1011
#define MUX_S4      0b1100
#define MUX_S5      0b1101
#define MUX_S6      0b1110
#define MUX_S7      0b1111

//function prototypes
void thermistor_reading(uint8_t GPIO1voltages[2]);

#endif // THERMISTOR_H
