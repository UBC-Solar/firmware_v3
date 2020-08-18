/**
 * 	@file ltc6811_btm.h
 *  @brief Header file for driver for the LTC6811-1 battery monitor IC.
 *
 *  All functions and many symbolic constants associated with this driver are
 *  prefixed with "BTM"
 *
 *	STEPS TO SET UP THIS DRIVER:
 *	1.	Ensure that BTM_NUM_DEVICES is set correctly for the hardware setup
 *	2. 	Make sure BTM_CS_GPIO_PORT and BTM_CS_GPIO_PIN correspond to the correct
 *		port and pin for the "Chip Select" (CS) SPI output
 *	3.	After initializing the STM32's SPI peripheral in the application code,
 *		set the BTM_SPI_handle pointer to the HAL SPI handle
 *		eg. BTM_SPI_handle = &hspi1;
 *
 *  NOTE: "Cell" and "Module," as they appear in this code, should be treated
 *  as synonymous. The inconsistency in the use of the 2 terms is due to
 *  conflicting nomenclature of the team's battery pack architecture and the
 *  LTC6811 documentation.
 *
 *  NOTE: The "Self Test" (ST) diagnostic functions and associated values have
 *  been commented out since the command variations have not yet been implemented
 *  Just to be clear, in some cases "ST" refers to "start" not "self test"
 *
 *  @date 2020/02/14
 *  @author Andrew Hanlon (a2k-hanlon)
 *	@author Laila Khan (lailakhankhan)
 *
 */

#ifndef LTC6811_BTM_H_
#define LTC6811_BTM_H_

#include "stm32f3xx_hal.h"

/*============================================================================*/
/* ENUMERATIONS */

enum BTM_Error {
    BTM_OK                  = 0,
    BTM_ERROR_PEC           = 1,
    BTM_ERROR_TIMEOUT       = 2,
    BTM_ERROR_HAL           = 3,
    BTM_ERROR_HAL_BUSY      = 4,
    BTM_ERROR_HAL_TIMEOUT   = 5
};
#define BTM_HAL_ERROR_OFFSET 2 // BTM_ERROR_HAL - HAL_ERROR

// LTC6811 ADC mode options
// First freq applies when ADCOPT == 0, second when ADCOPT == 1
// Descriptions "fast," "normal," 'filtered" apply when ADCOPT == 0
enum BTM_MD_e {
    MD_422HZ_1KHZ  = 0x0,
    MD_27KHZ_14KHZ = 0x1,	// fast
    MD_7KHZ_3KHZ   = 0x2,	// normal
    MD_26HZ_2KHZ   = 0x3	// filtered
};

// LTC6811 GPIO selection for ADC conversion
enum BTM_CHG_e {
    CHG_ALL   = 0x0, // GPIO 1 through 5 and VREF2
    CHG_GPIO1 = 0x1,
    CHG_GPIO2 = 0x2,
    CHG_GPIO3 = 0x3,
    CHG_GPIO4 = 0x4,
    CHG_GPIO5 = 0x5,
    CHG_VREF2 = 0x6
};

// LTC6811 Status Group selection
enum BTM_CHST_e {
    CHST_ALL = 0x0, // Measure all 4 parameters below:
    CHST_SC  = 0x1, // Sum of all Cells
    CHST_ITMP= 0x2, // Internal Die Temperature
    CHST_VA  = 0x3, // Analog Power Supply
    CHST_VD  = 0x4  // Digital Power Supply
};

typedef enum {
    MODULE_DISABLED = 0,
    MODULE_ENABLED = 1
} BTM_module_enable_t;

typedef enum {
    CS_LOW = 0,
    CS_HIGH = 1
} CS_state_t;

/*============================================================================*/
/* CONFIGURABLE PARAMETERS */

#define BTM_NUM_DEVICES 3U // Number of LTC6811-1 ICs daisy chained

#define BTM_TIMEOUT_VAL 30U // ms - safety timeout threshold for BTM functions
#define BTM_MAX_READ_ATTEMPTS 3U // maximum number of times to try to perform a
                                // read operation from the LTC6811's
#define BTM_CS_GPIO_PORT GPIOA
#define BTM_CS_GPIO_PIN GPIO_PIN_4

/* Configuration Register Group Parameters */

// Keep voltage references on between ADC reads (significantly speeds up reads,
//   increases power consumption)
#define REFON 1
// 0 = References Shut Down After Conversions,
// 1 = References Remain Powered Up Until Watchdog Timeout

// Under-voltage threshold for LTC6811
#define VUV 1687U // (2.7V / (16 * 0.0001V)) - 1 = 1687
// Over-voltage threshold for LTC6811
#define VOV 2624U // (4.2V / (16 * 0.0001V)) - 1 = 2624

// ADCOPT selects the ADC mode together with MD, but is in the CFG register
#define ADCOPT 0
/* End Configuration Register Group Parameters */

// Discharge Permitted during cell measurement
#define DCP 0 // 0 = Discharge Not Permitted, 1 = Discharge Permitted
// ADC Mode (speed)
#define MD MD_7KHZ_3KHZ // Normal mode
// Self Test Mode Selection
// #define ST 1 // TODO: Add enumeration if ST commands are needed
/* Pull-Up/Pull-Down Current for Open Wire Conversions */
#define PUP 0 // 0 = Pull down, 1 = Pull up

/*============================================================================*/
/* COMMAND DEFINITIONS */

// LTC6811 commands enumeration
// note: not all of these have been checked for correctness
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

    // Start Cell Voltage ADC Conversion and Poll Status
    CMD_ADCV    = 0x0260 | (MD << 7) | (DCP << 4), // CH set to 0 = all cells
    // Start Open Wire ADC Conversion and Poll Status
    CMD_ADOW    = 0x0228 | (MD << 7) | (PUP << 6) | (DCP << 4), // CH set to 0

    // Start Self Test Cell Voltage Conversion and Poll Status
    //CMD_CVST    = 0x0207 | (MD << 7) | (ST << 5),

    // Start Overlap Measurement of Cell 7 Voltage
    CMD_ADOL    = 0x0201 | (MD << 7) | (DCP << 4),

    // Start GPIOs ADC Conversion and Poll Status
    CMD_ADAX_ALL   =  0x0460 | (MD << 7) | CHG_ALL,
    CMD_ADAX_GPIO1 =  0x0460 | (MD << 7) | CHG_GPIO1,
    CMD_ADAX_GPIO2 =  0x0460 | (MD << 7) | CHG_GPIO2,
    CMD_ADAX_GPIO3 =  0x0460 | (MD << 7) | CHG_GPIO3,
    CMD_ADAX_GPIO4 =  0x0460 | (MD << 7) | CHG_GPIO4,
    CMD_ADAX_GPIO5 =  0x0460 | (MD << 7) | CHG_GPIO5,
    CMD_ADAX_VREF2 =  0x0460 | (MD << 7) | CHG_VREF2,
    // Start GPIOs ADC Conversion With Digital Redundancy and Poll Status
    CMD_ADAXD_ALL   =  0x0400 | (MD << 7) | CHG_ALL,
    CMD_ADAXD_GPIO1 =  0x0400 | (MD << 7) | CHG_GPIO1,
    CMD_ADAXD_GPIO2 =  0x0400 | (MD << 7) | CHG_GPIO2,
    CMD_ADAXD_GPIO3 =  0x0400 | (MD << 7) | CHG_GPIO3,
    CMD_ADAXD_GPIO4 =  0x0400 | (MD << 7) | CHG_GPIO4,
    CMD_ADAXD_GPIO5 =  0x0400 | (MD << 7) | CHG_GPIO5,
    CMD_ADAXD_VREF2 =  0x0400 | (MD << 7) | CHG_VREF2,

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
    CMD_STCOMM  = 0x0723        // Start I2C /SPI Communication
} BTM_command_t;

/*============================================================================*/
/* PUBLIC CONSTANTS */
// Do not change

#define BTM_NUM_MODULES 12
#define BTM_REG_GROUP_SIZE 6 // All of the LTC6811 register groups consist of 6 bytes

/*============================================================================*/
/* STRUCTURES FOR GATHERED DATA */

/*
 * NOTE: the BTM_module entity would be considered a "cell" by the LTC6811
 * datasheet's naming conventions. Here it's called a module due to the fact
 * that we arrange physical battery cells in parallel to create modules.
 * (the cells in a module are in parallel - they're all at the same voltage
 * and their voltage is measured at the module, not cell level).
 */
struct BTM_module {
    // To ignore particular modules in the string (for checking functions),
    // the enable parameter has been included. A zero means
    // the module will be ignored when checking for faults, etc.
    BTM_module_enable_t enable;
    uint16_t voltage;
    uint16_t temperature;
    int status;
};

struct BTM_stack {
    uint8_t cfgr[BTM_REG_GROUP_SIZE]; // Configuration Register Group setting
    unsigned int stack_voltage;
    struct BTM_module module[BTM_NUM_MODULES];
    // TODO: balancing settings, other stack-level (IC-level) parameters
    // Don't forget to add any new parameters to BTM_init()
};

typedef struct {
    unsigned int pack_voltage;
    struct BTM_stack stack[BTM_NUM_DEVICES];
} BTM_PackData_t;

// Status type for error reporting
typedef struct {
    enum BTM_Error error;
    unsigned int device_num; // Device at which error occurred, if applicable.
    // 0 = N/A, 1 = first device in chain, 2 = second device...
    // If there is no error (error == BTM_OK), device_num should be 0.
    // The reason for the N/A option is not all operations have a means of
    // differentiating responses of different LTC6811's in the chain.
} BTM_Status_t;

#define BTM_STATUS_DEVICE_NA 0  // "device number not applicable" value
                                // for device_num attribute of BTM_Status_t

/*============================================================================*/
/* PUBLIC VARIABLES */

// BTM_SPI_handle - must set this variable to the HAL SPI handle corresponding
// to the SPI peripheral to which the LTC devices are connected
SPI_HandleTypeDef * BTM_SPI_handle;

/*============================================================================*/
/* FUNCTION PROTOTYPES */
uint16_t BTM_calculatePec15(uint8_t* data, int len);
void BTM_init(BTM_PackData_t * pack);
void BTM_wakeup(void);
void BTM_sendCmd(BTM_command_t command);
BTM_Status_t BTM_sendCmdAndPoll(BTM_command_t command);
void BTM_writeRegisterGroup(BTM_command_t command, uint8_t tx_data[][BTM_REG_GROUP_SIZE]);
BTM_Status_t BTM_readRegisterGroup(BTM_command_t command, uint8_t rx_data[][BTM_REG_GROUP_SIZE]);
BTM_Status_t BTM_readBatt(BTM_PackData_t * packData);
float BTM_regValToVoltage(unsigned int raw_reading);
void BTM_writeCS(CS_state_t new_state);

#endif /* LTC6811_BTM_H_ */
