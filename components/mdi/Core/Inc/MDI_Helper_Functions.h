/**
 * This is the header file for the MDI helper functions.
 *
 **/

#include "stm32f1xx_hal.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

uint8_t getBit(uint8_t msb, uint8_t two, uint8_t three, uint8_t four, uint8_t five, uint8_t six, uint8_t seven, uint8_t lsb);

void CopyRxData(uint8_t* globalRxData, uint8_t* localRxData);

void Send_Voltage(float parsed_voltage, uint8_t DAC_ADDR, I2C_HandleTypeDef *hi2c2);

uint16_t Parse_Acceleration(uint32_t pedal_data);

void split_float_number(float number, uint8_t* bytes);