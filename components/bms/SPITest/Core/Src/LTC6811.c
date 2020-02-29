/*
 * LTC6811_SPI.c
 *
 *  Created on: Feb. 14, 2020
 *      Author: Andrew Hanlon
 *
 * Note that the "char" type is really just an unsigned integer byte
 */

#include "main.h"
#include "LTC6811.h"

unsigned char LTC_CMD_ADCV[] = {0x03, 0x60}; // char is just an unsigned integer byte
unsigned char LTC_CMD_RDCVA[] = {0x00, 0x04};
unsigned char LTC_CMD_RDCVB[] = {0x00, 0x06};
unsigned char LTC_CMD_RDCVC[] = {0x00, 0x08};
unsigned char LTC_CMD_RDCVD[] = {0x00, 0x0A};

// Lookup table for PEC (Packet Error Code) CRC calculation
// See note at end for details of its origin
const unsigned short int pec15Table[] = {
    0x0000, 0xC599, 0xCEAB, 0x0B32, 0xD8CF, 0x1D56, 0x1664, 0xD3FD,
    0xF407, 0x319E, 0x3AAC, 0xFF35, 0x2CC8, 0xE951, 0xE263, 0x27FA,
    0xAD97, 0x680E, 0x633C, 0xA6A5, 0x7558, 0xB0C1, 0xBBF3, 0x7E6A,
    0x5990, 0x9C09, 0x973B, 0x52A2, 0x815F, 0x44C6, 0x4FF4, 0x8A6D,
    0x5B2E, 0x9EB7, 0x9585, 0x501C, 0x83E1, 0x4678, 0x4D4A, 0x88D3,
    0xAF29, 0x6AB0, 0x6182, 0xA41B, 0x77E6, 0xB27F, 0xB94D, 0x7CD4,
    0xF6B9, 0x3320, 0x3812, 0xFD8B, 0x2E76, 0xEBEF, 0xE0DD, 0x2544,
    0x02BE, 0xC727, 0xCC15, 0x098C, 0xDA71, 0x1FE8, 0x14DA, 0xD143,
    0xF3C5, 0x365C, 0x3D6E, 0xF8F7, 0x2B0A, 0xEE93, 0xE5A1, 0x2038,
    0x07C2, 0xC25B, 0xC969, 0x0CF0, 0xDF0D, 0x1A94, 0x11A6, 0xD43F,
    0x5E52, 0x9BCB, 0x90F9, 0x5560, 0x869D, 0x4304, 0x4836, 0x8DAF,
    0xAA55, 0x6FCC, 0x64FE, 0xA167, 0x729A, 0xB703, 0xBC31, 0x79A8,
    0xA8EB, 0x6D72, 0x6640, 0xA3D9, 0x7024, 0xB5BD, 0xBE8F, 0x7B16,
    0x5CEC, 0x9975, 0x9247, 0x57DE, 0x8423, 0x41BA, 0x4A88, 0x8F11,
    0x057C, 0xC0E5, 0xCBD7, 0x0E4E, 0xDDB3, 0x182A, 0x1318, 0xD681,
    0xF17B, 0x34E2, 0x3FD0, 0xFA49, 0x29B4, 0xEC2D, 0xE71F, 0x2286,
    0xA213, 0x678A, 0x6CB8, 0xA921, 0x7ADC, 0xBF45, 0xB477, 0x71EE,
    0x5614, 0x938D, 0x98BF, 0x5D26, 0x8EDB, 0x4B42, 0x4070, 0x85E9,
    0x0F84, 0xCA1D, 0xC12F, 0x04B6, 0xD74B, 0x12D2, 0x19E0, 0xDC79,
    0xFB83, 0x3E1A, 0x3528, 0xF0B1, 0x234C, 0xE6D5, 0xEDE7, 0x287E,
    0xF93D, 0x3CA4, 0x3796, 0xF20F, 0x21F2, 0xE46B, 0xEF59, 0x2AC0,
    0x0D3A, 0xC8A3, 0xC391, 0x0608, 0xD5F5, 0x106C, 0x1B5E, 0xDEC7,
    0x54AA, 0x9133, 0x9A01, 0x5F98, 0x8C65, 0x49FC, 0x42CE, 0x8757,
    0xA0AD, 0x6534, 0x6E06, 0xAB9F, 0x7862, 0xBDFB, 0xB6C9, 0x7350,
    0x51D6, 0x944F, 0x9F7D, 0x5AE4, 0x8919, 0x4C80, 0x47B2, 0x822B,
    0xA5D1, 0x6048, 0x6B7A, 0xAEE3, 0x7D1E, 0xB887, 0xB3B5, 0x762C,
    0xFC41, 0x39D8, 0x32EA, 0xF773, 0x248E, 0xE117, 0xEA25, 0x2FBC,
    0x0846, 0xCDDF, 0xC6ED, 0x0374, 0xD089, 0x1510, 0x1E22, 0xDBBB,
    0x0AF8, 0xCF61, 0xC453, 0x01CA, 0xD237, 0x17AE, 0x1C9C, 0xD905,
    0xFEFF, 0x3B66, 0x3054, 0xF5CD, 0x2630, 0xE3A9, 0xE89B, 0x2D02,
    0xA76F, 0x62F6, 0x69C4, 0xAC5D, 0x7FA0, 0xBA39, 0xB10B, 0x7492,
    0x5368, 0x96F1, 0x9DC3, 0x585A, 0x8BA7, 0x4E3E, 0x450C, 0x8095
};

// pec15 Function:
// Calculates the PEC for "len" bytes of data
// Function adapted from code on pg. 76 of LTC6811 Datasheet
uint16_t LTC_pec15(uint8_t* data, int len) {
    uint16_t remainder, address;
    remainder = 16; // PEC seed (initial value for calculation (16 = 0x10)
    for (int i = 0; i < len; i++) {
        address = ((remainder >> 7) ^ data[i]) & 0xff; // calculate PEC table address
        remainder = (remainder << 8) ^ pec15Table[address];
    }
    return (remainder * 2); // The CRC15 has a 0 in the LSB so the final value must be multiplied by 2
}

void LTC_sendCMD(SPI_HandleTypeDef SPI_handle, uint8_t* command) {
	uint16_t pecValue = LTC_pec15(command, 2);
	uint8_t TX_message[4];

	TX_message[0] = command[0];
	TX_message[1] = command[1];
	TX_message[2] = (uint8_t) ((pecValue & 0xff00) >> 8);
	TX_message[3] = (uint8_t) (pecValue & 0x00ff);

	// size parameter is number of bytes to transmit - here it's 4 8bit frames
	HAL_SPI_Transmit(&SPI_handle, TX_message, 4, 250);
	return;
}

void LTC_readRegisterGroup(SPI_HandleTypeDef SPI_handle, uint8_t* command, uint8_t* RX_data) {
	uint16_t pecValue;

	// Send command to read register group
	HAL_GPIO_WritePin(CS_LTC_GPIO_Port, CS_LTC_Pin, GPIO_PIN_RESET); // pull CS line low
	LTC_sendCMD(SPI_handle, command);

	// Read back the data
	HAL_SPI_Receive(&SPI_handle, RX_data, 8, 250); // 6 data bytes and 2 PEC bytes
	pecValue = LTC_pec15(RX_data, 8); // Should be 0; TODO: add error handling
	HAL_GPIO_WritePin(CS_LTC_GPIO_Port, CS_LTC_Pin, GPIO_PIN_SET); // pull CS line back high
	return;
}

void LTC_readADC(SPI_HandleTypeDef SPI_handle) {
    uint8_t RX_message = 0; // Initialize the buffer before using it, or the garbage it contains will be
    						// sent as dummy data - see definition of HAL_SPI_Receive
    uint8_t ADC_data[4][8] = {{0}}; // Four 8-byte sets (each from a different register group of the LTC6811

    HAL_GPIO_WritePin(CS_LTC_GPIO_Port, CS_LTC_Pin, GPIO_PIN_RESET); // pull CS line low
    LTC_sendCMD(SPI_handle, LTC_CMD_ADCV);

    // wait until the MISO line goes high - this signifies that the LTC6811 is done
    // reading its ADCs
    HAL_SPI_Receive(&SPI_handle, &RX_message, 1, 250);
    while(!RX_message) {
    	HAL_SPI_Receive(&SPI_handle, &RX_message, 1, 250);
    }
    // Cycle the Chip-select line before the next command is sent
    HAL_GPIO_WritePin(CS_LTC_GPIO_Port, CS_LTC_Pin, GPIO_PIN_SET); // pull CS line back high

    LTC_readRegisterGroup(SPI_handle, LTC_CMD_RDCVA, ADC_data[0]);
    LTC_readRegisterGroup(SPI_handle, LTC_CMD_RDCVB, ADC_data[1]);
    LTC_readRegisterGroup(SPI_handle, LTC_CMD_RDCVC, ADC_data[2]);
    LTC_readRegisterGroup(SPI_handle, LTC_CMD_RDCVD, ADC_data[3]);
    return;
}
