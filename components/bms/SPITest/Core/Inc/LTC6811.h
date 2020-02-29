/*
 * LTC6811_SPI.h
 *
 *  Created on: Feb. 14, 2020
 *      Author: Andrew Hanlon
 */

#ifndef SRC_LTC6811_H_
#define SRC_LTC6811_H_

#include "stm32f3xx_hal.h"

// Function prototypes
uint16_t LTC_pec15(uint8_t*, int);
void LTC_readADC(SPI_HandleTypeDef);

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
