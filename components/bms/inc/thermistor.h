//header file for thermistor.c
#ifndef THERMISTOR_H
#define THERMISTOR_H

//TO MOVE TO 6811.h
#define LTC_CMD_STCOMM      0b00000_111_0010_0011

//Normal mode, gpio1
#define LTC_CMD_ADAX_GPIO1  0b00000_01_10_1100_001
#define LTC_CMD_RDAUXA      0b00000_000_0000_1100

void thermistor_reading(uint8_t GPIO1voltages[2]);

#endif // THERMISTOR_H
