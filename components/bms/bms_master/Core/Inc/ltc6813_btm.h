/**
 *  @file ltc6813_btm.h
 *  @brief Header file for driver for the LTC6813-1 battery monitor IC.
 *
 *  All functions and many symbolic constants associated with this driver are
 *  prefixed with "BTM" which is short for "Battery Monitoring"
 *
 *  STEPS TO SET UP THIS DRIVER:
 *  1.   Ensure that BTM_NUM_DEVICES is set correctly for the hardware setup
 *  2.   After initializing the STM32's SPI peripheral in the application code,
 *	    pass the BTM_SPI_handle pointer to BTM_Init()
 
 *  NOTE: "Cell" and "Module," as they appear in this code, have slightly
 *  different connotations. "Module" refers to a physical set of parallel
 *  battery cells present in the battery pack. "Cell" or "cell input"
 *  refers to a measurement input pair (and balancing control pin) present on
 *  an LTC6813 IC. The distinction is made since not all measurement inputs of
 *  the IC may actually be in use in the hardware design.
 * 
 *  The mapping of physical modules to LTC6813 cell inputs is given by pack.h
 *
 *  NOTE: The "Self Test" (ST) diagnostic functions and associated values have
 *  been commented out since the command variations have not yet been implemented
 *  Just to be clear, in some cases "ST" refers to "start" not "self test"
 *
 *  @date 2020/08/18
 *  @author Andrew Hanlon (a2k-hanlon)
 *  @author Laila Khan (lailakhankhan)
 */

#ifndef INC_LTC6813_BTM_H_
#define INC_LTC6813_BTM_H_

#include "stm32f1xx_hal.h"
#include <stdint.h>
#include "pack.h"

/*============================================================================*/
/* ENUMERATIONS */

enum BTM_Error {
    BTM_OK = 0,
    BTM_ERROR_PEC,
    BTM_ERROR_TIMEOUT,
    BTM_ERROR_SELFTEST,
    BTM_ERROR_HAL,
    BTM_ERROR_HAL_BUSY,
    BTM_ERROR_HAL_TIMEOUT
};
#define BTM_HAL_ERROR_OFFSET (BTM_ERROR_HAL - HAL_ERROR)

// LTC6813 ADC mode options
// First freq applies when ADCOPT == 0, second when ADCOPT == 1
// Descriptions "fast," "normal," 'filtered" apply when ADCOPT == 0
enum BTM_MD_e {
    MD_422HZ_1KHZ  = 0x0,
    MD_27KHZ_14KHZ = 0x1,	// fast
    MD_7KHZ_3KHZ   = 0x2,	// normal
    MD_26HZ_2KHZ   = 0x3	// filtered
};

// LTC6813 ADC Cell Measurement Options
enum BTM_CH_e {
    CH_ALL = 0x0,
    CH_1 = 0x1, // Measure Cells 1, 7, 13
    CH_2 = 0x2, // Measure Cells 2, 8, 14
    CH_3 = 0x3, // Measure Cells 3, 9, 15
    CH_4 = 0x4, // Measure Cells 4, 10, 16
    CH_5 = 0x5, // Measure Cells 5, 11, 17
    CH_6 = 0x6  // Measure Cells 6, 12, 18
};

// LTC6813 GPIO selection for ADC conversion
enum BTM_CHG_e {
    CHG_ALL     = 0x0,    // GPIO 1 through 5, VREF2 and GPIO 6 through 9
    CHG_GPIO1_6 = 0x1,  // GPIO 1 and 6
    CHG_GPIO2_7 = 0x2,  // GPIO 2 and 7
    CHG_GPIO3_8 = 0x3,  // GPIO 3 and 8
    CHG_GPIO4_9 = 0x4,  // GPIO 4 and 9
    CHG_GPIO5   = 0x5,
    CHG_VREF2   = 0x6
};

// LTC6813 Status Group selection
enum BTM_CHST_e {
    CHST_ALL = 0x0, // Measure all 4 parameters below:
    CHST_SC  = 0x1, // Sum of all Cells
    CHST_ITMP= 0x2, // Internal Die Temperature
    CHST_VA  = 0x3, // Analog Power Supply
    CHST_VD  = 0x4  // Digital Power Supply
};

// Pull-Up/Pull-Down Current for Open Wire Conversions
enum BTM_PUP_e {
    PUP_PULLDOWN = 0x0,
    PUP_PULLUP   = 0x1
};

typedef enum {
    CS_LOW  = 0,
    CS_HIGH = 1
} CS_state_t;

/*============================================================================*/
/* CONFIGURABLE PARAMETERS */

#define BTM_NUM_DEVICES 2U // Number of LTC6813-1 ICs daisy chained

#define BTM_TIMEOUT_VAL 30U // ms - safety timeout threshold for BTM functions
#define BTM_MAX_READ_ATTEMPTS 3U // maximum number of times to try to perform a
                                 // read operation from the LTC6813's

/* Configuration Register Group Parameters */

// Keep voltage references on between ADC reads (significantly speeds up reads,
//   increases power consumption)
#define REFON 1
// 0 = References Shut Down After Conversions,
// 1 = References Remain Powered Up Until Watchdog Timeout

// Under-voltage threshold for LTC6813
#define VUV 1687U // (2.7V / (16 * 0.0001V)) - 1 = 1687
// Over-voltage threshold for LTC6813
#define VOV 2624U // (4.2V / (16 * 0.0001V)) - 1 = 2624
// Note that these thresholds are internal to the LTC6813; they only
//   impact the behaviour of the UV and OV bit flags in the status registers

// ADCOPT selects the ADC mode together with MD, but is in the CFG register
#define ADCOPT 0
/* End Configuration Register Group Parameters */

// Discharge Permitted during cell measurement
#define DCP 0 // 0 = Discharge Not Permitted 1 = Discharge Permitted
// ADC Mode (speed)
#define MD MD_7KHZ_3KHZ // Normal mode
// Self Test Mode Selection
// #define ST 1 // TODO: Add enumeration if ST commands are needed

/*============================================================================*/
/* COMMAND DEFINITIONS */

// LTC6813 commands enumeration
// note: not all of these have been checked for correctness
typedef enum {
    CMD_WRCFGA  = 0x0001,       // Write Configuration Register Group A
    CMD_WRCFGB  = 0x0024,       // Write Configuration Register Group B
    CMD_RDCFGA  = 0x0002,       // Read Configuration Register Group A
    CMD_RDCFGB  = 0x0026,       // Read Configuration Register Group B
    CMD_RDCVA   = 0x0004,       // Read Cell Voltage Register Group A
    CMD_RDCVB   = 0x0006,       // Read Cell Voltage Register Group B
    CMD_RDCVC   = 0x0008,       // Read Cell Voltage Register Group C
    CMD_RDCVD   = 0x000A,       // Read Cell Voltage Register Group D
    CMD_RDCVE   = 0x0009,       // Read Cell Voltage Register Group E
    CMD_RDCVF   = 0x000B,       // Read Cell Voltage Register Group F
    CMD_RDAUXA  = 0x000C,       // Read Auxilliary Register Group A
    CMD_RDAUXB  = 0x000E,       // Read Auxilliary Register Group B
    CMD_RDAUXC  = 0x000D,       // Read Auxilliary Register Group C
    CMD_RDAUXD  = 0x000F,       // Read Auxilliary Register Group D
    CMD_RDSTATA = 0x0010,       // Read Status Register Group A
    CMD_RDSTATB = 0x0012,       // Read Status Register Group B
    CMD_WRSCTRL = 0x0014,       // Write S Control Register Group
    CMD_WRPWM   = 0x0020,       // Write PWM Register Group
    CMD_WRPSB   = 0x001C,       // Write PWM/S Control Register Group B
    CMD_RDSCTRL = 0x0016,       // Read S Control Register Group
    CMD_RDPWM   = 0x0022,       // Read PWM Register Group
    CMD_RDPSB   = 0x001E,       // Read PWM/S Control Register Group B
    CMD_STSCTRL = 0x0019,       // Start S Control Pulsing and Poll Status
    CMD_CLRSCTRL= 0x0018,       // Clear S Control Register Group

    // Start Cell Voltage ADC Conversion and Poll Status
    CMD_ADCV     = 0x0260 | (MD << 7) | (DCP << 4) | CH_ALL,
    CMD_ADCV_CH1 = 0x0260 | (MD << 7) | (DCP << 4) | CH_1,
    CMD_ADCV_CH2 = 0x0260 | (MD << 7) | (DCP << 4) | CH_2,
    CMD_ADCV_CH3 = 0x0260 | (MD << 7) | (DCP << 4) | CH_3,
    CMD_ADCV_CH4 = 0x0260 | (MD << 7) | (DCP << 4) | CH_4,
    CMD_ADCV_CH5 = 0x0260 | (MD << 7) | (DCP << 4) | CH_5,
    CMD_ADCV_CH6 = 0x0260 | (MD << 7) | (DCP << 4) | CH_6,

    // Start Open Wire ADC Conversion and Poll Status
    CMD_ADOW_PUP    = 0x0228 | (MD << 7) | (PUP_PULLUP << 6)   | (DCP << 4), // CH set to 0
    CMD_ADOW_PDOWN  = 0x0228 | (MD << 7) | (PUP_PULLDOWN << 6) | (DCP << 4), // CH set to 0

    // Start Self Test Cell Voltage Conversion and Poll Status
    //CMD_CVST    = 0x0207 | (MD << 7) | (ST << 5),

    // Start Overlap Measurement of Cell 7 Voltage
    CMD_ADOL    = 0x0201 | (MD << 7) | (DCP << 4),

    // Start GPIOs ADC Conversion and Poll Status
    CMD_ADAX_ALL      =  0x0460 | (MD << 7) | CHG_ALL,
    CMD_ADAX_GPIO1_6  =  0x0460 | (MD << 7) | CHG_GPIO1_6,
    CMD_ADAX_GPIO2_7  =  0x0460 | (MD << 7) | CHG_GPIO2_7,
    CMD_ADAX_GPIO3_8  =  0x0460 | (MD << 7) | CHG_GPIO3_8,
    CMD_ADAX_GPIO4_9  =  0x0460 | (MD << 7) | CHG_GPIO4_9,
    CMD_ADAX_GPIO5    =  0x0460 | (MD << 7) | CHG_GPIO5,
    CMD_ADAX_VREF2    =  0x0460 | (MD << 7) | CHG_VREF2,
    // Start GPIOs ADC Conversion With Digital Redundancy and Poll Status
    CMD_ADAXD_ALL     =  0x0400 | (MD << 7) | CHG_ALL,
    CMD_ADAXD_GPIO1_6 =  0x0400 | (MD << 7) | CHG_GPIO1_6,
    CMD_ADAXD_GPIO2_7 =  0x0400 | (MD << 7) | CHG_GPIO2_7,
    CMD_ADAXD_GPIO3_8 =  0x0400 | (MD << 7) | CHG_GPIO3_8,
    CMD_ADAXD_GPIO4_9 =  0x0400 | (MD << 7) | CHG_GPIO4_9,
    CMD_ADAXD_GPIO5   =  0x0400 | (MD << 7) | CHG_GPIO5,
    CMD_ADAXD_VREF2   =  0x0400 | (MD << 7) | CHG_VREF2,

    // Start Self Test GPIOs Conversion and Poll Status
    //CMD_AXST = 0x0407 | (MD << 7) | (ST << 5),

    // Start Status Group ADC Conversion and Poll Status
    CMD_ADSTAT_ALL  = 0x0468 | (MD << 7) | CHST_ALL,
    CMD_ADSTAT_SC   = 0x0468 | (MD << 7) | CHST_SC,
    CMD_ADSTAT_ITMP = 0x0468 | (MD << 7) | CHST_ITMP,
    CMD_ADSTAT_VA   = 0x0468 | (MD << 7) | CHST_VA,
    CMD_ADSTAT_VD   = 0x0468 | (MD << 7) | CHST_VD,
    // Start Status Group ADC Conversion With Digital Redundancy and Poll Status
    CMD_ADSTATD_ALL = 0x0408 | (MD << 7) | CHST_ALL,
    CMD_ADSTATD_SC  = 0x0408 | (MD << 7) | CHST_SC,
    CMD_ADSTATD_ITMP= 0x0408 | (MD << 7) | CHST_ITMP,
    CMD_ADSTATD_VA  = 0x0408 | (MD << 7) | CHST_VA,
    CMD_ADSTATD_VD  = 0x0408 | (MD << 7) | CHST_VD,

    // Start Self Test Status Group Conversion and Poll Status
    //CMD_STATST  = 0x040F | (MD << 7) | (ST << 5),

    // Start Combined Cell Voltage and GPIO1, GPIO2 Conversion and Poll Status
    CMD_ADCVAX  = 0x046F | (MD << 7) | (DCP << 4),
    // Start Combined Cell Voltage and SC Conversion and Poll Status
    CMD_ADCVSC  = 0x0467 | (MD << 7) | (DCP << 4),

    CMD_CLRCELL = 0x0711,       // Clear Cell Voltage Register Groups
    CMD_CLRAUX  = 0x0712,       // Clear Auxiliary Register Groups
    CMD_CLRSTAT = 0x0713,       // Clear Status Register Groups
    CMD_PLADC   = 0x0714,       // Poll ADC Conversion Status
    CMD_DIAGN   = 0x0715,       // Diagnose MUX and Poll Status
    CMD_WRCOMM  = 0x0721,       // Write COMM Register Group
    CMD_RDCOMM  = 0x0722,       // Read COMM Register Group
    CMD_STCOMM  = 0x0723,       // Start I2C /SPI Communication
    CMD_MUTE    = 0x0028,       // Mute discharge
    CMD_UNMUTE  = 0x0029        // Unmute discharge
} BTM_command_t;

/*============================================================================*/
/* PUBLIC CONSTANTS */
// Do not change

#define BTM_NUM_CELL_INPUTS_PER_DEVICE 18
#define BTM_REG_GROUP_SIZE 6 // All of the LTC6813 register groups consist of 6 bytes
#define NUM_CELL_VOLT_REGS 6
#define READINGS_PER_REG 3

/*============================================================================*/
/* STRUCTURES */

struct BTM_RawDeviceVoltages_s {
    uint16_t voltage[BTM_NUM_CELL_INPUTS_PER_DEVICE];
};

typedef struct {
    struct BTM_RawDeviceVoltages_s device[BTM_NUM_DEVICES];
} BTM_RawVoltages_t;

// Status type for error reporting
typedef struct {
    enum BTM_Error error;
    unsigned int device_num; // Device at which error occurred, if applicable.
    // 0 = N/A, 1 = first device in chain, 2 = second device...
    // If there is no error (error == BTM_OK), device_num should be 0.
    // The reason for the N/A option is not all operations have a means of
    // differentiating responses of different LTC6813's in the chain.
} BTM_Status_t;

#define BTM_STATUS_DEVICE_NA 0  // "device number not applicable" value
                                // for device_num attribute of BTM_Status_t

typedef struct {
    SPI_HandleTypeDef *SPI_handle;

    uint8_t cfgra[BTM_NUM_DEVICES][BTM_REG_GROUP_SIZE]; // Record of Configuration Register Group A for each device
    uint8_t cfgrb[BTM_NUM_DEVICES][BTM_REG_GROUP_SIZE]; // Record of Configuration Register Group B for each device
} BTM_Data_t;

/*============================================================================*/
/* PUBLIC VARIABLES */

// BTM_data is for use by LTC6813 driver files ONLY
extern BTM_Data_t BTM_data;

/*============================================================================*/
/* FUNCTION PROTOTYPES */
void BTM_init(SPI_HandleTypeDef *SPI_handle);
void BTM_wakeup(void);
void BTM_sendCmd(BTM_command_t command);
BTM_Status_t BTM_sendCmdAndPoll(BTM_command_t command);
void BTM_writeRegisterGroup(BTM_command_t command, uint8_t tx_data[BTM_NUM_DEVICES][BTM_REG_GROUP_SIZE]);
BTM_Status_t BTM_readRegisterGroup(BTM_command_t command, uint8_t rx_data[BTM_NUM_DEVICES][BTM_REG_GROUP_SIZE]);
BTM_Status_t BTM_getVoltagesRaw(BTM_RawVoltages_t *voltageData);
BTM_Status_t BTM_getVoltages(Pack_t *pack);
float BTM_regValToVoltage(uint16_t raw_reading);

#endif /* INC_LTC6813_BTM_H_ */
