/**
 * 	@file ltc6811_btm.h
 *  @brief Header file for driver for the LTC6811-1 battery monitor IC.
 *
 *  All functions and many symbolic constants associated with this driver are
 *  prefixed with "BTM"
 *
 *  Created on: Feb. 14, 2020
 *  @author Andrew Hanlon (a2k-hanlon)
 *	@author Laila Khan (lailakhankhan)
 *
 */

#ifndef SRC_LTC6811_BTM_H_
#define SRC_LTC6811_BTM_H_

#include "stm32f3xx_hal.h"

#define BTM_NUM_DEVICES 1U // Number of LTC6811-1 ICs daisy chained
#define BTM_NUM_MODULES 36U
#define BTM_NUM_STACKS 3U

#define BTM_TIMEOUT_VAL 100 // ms safety timeout threshold for BTM functions
#define BTM_MAX_READ_ATTEMPTS 3 // maximum number of times to try to perform a
                                // read operation from the LTC6811's

typedef enum {                           // These are named assuming ADCOPT(CFGR0[0]) = 0
    ADC_MODE_422HZ = 0x0,
    ADC_MODE_27KHZ = 0x1,   // fast
    ADC_MODE_7KHZ = 0x2,    // normal
    ADC_MODE_26HZ = 0x3     // filtered
} BTM_ADC_Mode_t;

/* Cell Selection for ADC Conversion */
#define CH 0 // set to 0 = all cells // TODO: Could add enumeration

/* Discharge Permitted */
#define DCP 0 // 0 = Discharge Not Permitted, 1 = Discharge Permitted

/* ADC Mode */
#define MD ADC_MODE_7KHZ // Normal mode

/* Self Test Mode Selection */
#define ST 1 // TODO: Add enumeration

/* Pull-Up/Pull-Down Current for Open Wire Conversions */
#define PUP 0 // TODO: Add enumeration

/* Status Group Selection */
#define CHST 0 // TODO: Add enumeration

/* GPIO Selection for ADC Conversion */
#define CHG 0 // TODO: Add enumeration

#define ADCV_VAL    0x0260 | (MD << 7) | (DCP << 4) | CH
#define ADOW_VAL    0x0228 | (MD << 7) | (PUP << 6) | (DCP << 4) | CH
#define CVST_VAL    0x0207 | (MD << 7) | (ST << 5)
#define ADOL_VAL    0x0201 | (MD << 7) | (DCP << 4)
#define ADAX_VAL    0x0460 | (MD << 7) | CHG
#define ADAXD_VAL   0x0400 | (MD << 7) | CHG
#define AXST_VAL    0x0407 | (MD << 7) | (ST << 5)
#define ADSTAT_VAL  0x0468 | (MD << 7) | CHST
#define ADSTATD_VAL 0x0408 | (MD << 7) | CHST
#define STATST_VAL  0x040F | (MD << 7) | (ST << 5)
#define ADCVAX_VAL  0x046F | (MD << 7) | (DCP << 4)
#define ADCVSC_VAL  0x0467 | (MD << 7) | (DCP << 4)

typedef enum {
    BTM_OK = 0,
    BTM_ERROR_PEC = 1,
    BTM_ERROR_TIMEOUT = 2
} BTM_status_t;

typedef enum {
    CMD_WRCFGA  = 0x0001,       // Write Configuration Register Group A
    CMD_RDCFGA  = 0x0002,       // Read Configuration Register Group A
    CMD_RDCVA   = 0x0004,       // Read Cell Voltage Register Group A
    CMD_RDCVB   = 0x0006,       // Read Cell Voltage Register Group B
    CMD_RDCVC   = 0x0008,       // Read Cell Voltage Register Group C
    CMD_RDCVD   = 0x000A,       // Read Cell Voltage Register Group D
    CMD_RDAUXA  = 0x000C,       // Read Auxilliary Register Group A
    CMD_RDAUXB  = 0x000E,       // Read Auxilliary Register Group B
    CMD_RDSTATA = 0x0010,       // Read Status Register Group A
    CMD_RDSTATB = 0x0012,       // Read Status Register Group B
    CMD_WRSCTRL = 0x0014,       // Write S Control Register Group
    CMD_WRPWM   = 0x0020,       // Write PWM Register Group
    CMD_RDSCTRL = 0x0016,       // Read S Control Register Group
    CMD_RDPWM   = 0x0022,       // Read PWM Register Group
    CMD_STSCTRL = 0x0019,       // Start S Control Pulsing and Poll Status
    CMD_CLRSCTRL= 0x0018,       // Clear S Control Register Group
    CMD_ADCV    = ADCV_VAL,     // Start Cell Voltage ADC Conversion and Poll Status
    CMD_ADOW    = ADOW_VAL,     // Start Open Wire ADC Conversion and Poll Status
    CMD_CVST    = CVST_VAL,     // Start Self Test Cell Voltage Conversion and Poll Status
    CMD_ADOL    = ADOL_VAL,     // Start Overlap Measurement of Cell 7 Voltage
    CMD_ADAX    = ADAX_VAL,     // Start GPIOs ADC Conversion and Poll Status
    CMD_ADAXD   = ADAXD_VAL,    // Start GPIOs ADC Conversion With Digital Redundancy and Poll Status
    CMD_AXST    = AXST_VAL,     // Start Self Test GPIOs Conversion and Poll Status
    CMD_ADSTAT  = ADSTAT_VAL,   // Start Status Group ADC Conversion and Poll Status
    CMD_ADSTATD = ADSTATD_VAL,  // Start Status Group ADC Conversion With Digital Redundancy and Poll Status
    CMD_STATST  = STATST_VAL,   // Start Self Test Status Group Conversion and Poll Status
    CMD_ADCVAX  = ADCVAX_VAL,   // Start Combined Cell Voltage and GPIO1, GPIO2 Conversion and Poll Status
    CMD_ADCVSC  = ADCVSC_VAL,   // Start Combined Cell Voltage and SC Conversion and Poll Status
    CMD_CLRCELL = 0x0711,       // Clear Cell Voltage Register Groups
    CMD_CLRAUX  = 0x0712,       // Clear Auxiliary Register Groups
    CMD_CLRSTAT = 0x0713,       // Clear Status Register Groups
    CMD_PLADC   = 0x0714,       // Poll ADC Conversion Status
    CMD_DIAGN   = 0x0715,       // Diagnose MUX and Poll Status
    CMD_WRCOMM  = 0x07211,      // Write COMM Register Group
    CMD_RDCOMM  = 0x0722,       // Read COMM Register Group
    CMD_STCOMM  = 0x0723        // Start I2C /SPI Communication
} BTM_command_t;

// Public variables
// BTM_SPI_handle - must set this variable to the HAL SPI handle corresponding to
// the SPI peripheral to which the LTC devices are connected
SPI_HandleTypeDef* BTM_SPI_handle;

// Function prototypes
uint16_t BTM_calculatePec15(uint8_t* data, int len);
void BTM_wakeup(void);
void BTM_sendCmd(BTM_command_t command);
void BTM_writeRegisterGroup(BTM_command_t command, uint8_t tx_data[][6]);
BTM_status_t BTM_readRegisterGroup(BTM_command_t command, uint8_t rx_data[][6]);
BTM_status_t BTM_readBatt(uint16_t voltages[][12]);
float BTM_regValToVoltage(uint16_t raw_reading);

#endif /* SRC_LTC6811_BTM_H_ */
