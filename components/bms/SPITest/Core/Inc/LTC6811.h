/*
 * LTC6811.h
 *
 *  Created on: Feb. 14, 2020
 *      Author: Andrew Hanlon
 */

#ifndef SRC_LTC6811_H_
#define SRC_LTC6811_H_

#include "stm32f3xx_hal.h"

// Symbolic Constants
#define NUM_LTC 1 // Number of LTC6811's in the daisy chain

#define LTC_CMD_ADCV  0x0360 // Start Cell Voltage ADC Conversion and Poll Status
// Commands without options
#define LTC_CMD_WRCFGA 0x0001 // Write configuration register group A
#define LTC_CMD_RDCFGA 0x0002 // Read configuration register group A
#define LTC_CMD_RDCVA 0x0004 // Read Cell Voltage Register Group A
#define LTC_CMD_RDCVB 0x0006 // Read Cell Voltage Register Group B
#define LTC_CMD_RDCVC 0x0008 // Read Cell Voltage Register Group C
#define LTC_CMD_RDCVD 0x000A // Read Cell Voltage Register Group D
#define LTC_CMD_PLADC 0x0714 // Poll ADC Conversion Status

// Function prototypes
// LTC_pec15: returns the 2-byte CRC PEC generated from len bytes of data
uint16_t LTC_pec15(uint8_t* data, int len);

void LTC_wakeup(void);
void LTC_sendCmd(SPI_HandleTypeDef* SPI_handle, uint16_t command);
void LTC_writeRegisterGroup(SPI_HandleTypeDef* SPI_handle, uint16_t command, uint8_t TX_data[][6]);
int8_t LTC_readRegisterGroup(SPI_HandleTypeDef* SPI_handle, uint16_t command, uint8_t RX_data[][6]);
int8_t LTC_readBatt(SPI_HandleTypeDef* SPI_handle, float voltages[][12]);

#endif /* SRC_LTC6811_H_ */
