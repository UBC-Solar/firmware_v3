/*
 * LTC6811_SPI.h
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

// LTC_wakeup: toggles the CS line to wake up the entire chain of LTC6811's
void LTC_wakeup(void);

// LTC_sendCmd: sends the 2-byte command followed by the 2-byte PEC over SPI
void LTC_sendCmd(SPI_HandleTypeDef SPI_handle, uint16_t command);

// LTC_writeRegisterGroup: writes the 6 bytes of a configuration register group in the LTC6811
// Which register group is written to depends on the command provided (should be a WRxxxx command)
void LTC_writeRegisterGroup(SPI_HandleTypeDef SPI_handle, uint16_t command, uint8_t TX_data[][6]);

// LTC_readRegisterGroup: reads the 6 bytes of a configuration register group in the LTC6811
// Which register group is read depends on the command provided (should be a RDxxxx command)
int8_t LTC_readRegisterGroup(SPI_HandleTypeDef SPI_handle, uint16_t command, uint8_t RX_data[][6]);

// LTC_readBatt: Poll ADCs and read registers for cell voltages
// Returns -1 if there is a PEC mismatch, 0 otherwise
// voltages is a 2 dimensional array of size NUM_LTC by 12 containing the cell voltages C1-C12 of
// of each LTC6811 in the chain (nearest to MCU first)
int8_t LTC_readBatt(SPI_HandleTypeDef SPI_handle, float voltages[][12]);

#endif /* SRC_LTC6811_H_ */

/*
 * The pec15Table lookup table was generated using the following code, modified from the example code
 * on pg. 76 of the LTC6811 Datasheet:

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
