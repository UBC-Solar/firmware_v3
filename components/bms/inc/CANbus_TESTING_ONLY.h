#ifndef CANBUS_TESTING_ONLY_H_
#define CANBUS_TESTING_ONLY_H_

#include <stdint.h>
#include <math.h>
#include <stdio.h>

/*
    Copied from stm32f3xx_hal_can.h, provided by STMicroelectronics.
*/
typedef struct
{
  uint32_t FilterIdHigh;          /*!< Specifies the filter identification number (MSBs for a 32-bit
                                       configuration, first one for a 16-bit configuration).
                                       This parameter must be a number between Min_Data = 0x0000 and Max_Data = 0xFFFF. */

  uint32_t FilterIdLow;           /*!< Specifies the filter identification number (LSBs for a 32-bit
                                       configuration, second one for a 16-bit configuration).
                                       This parameter must be a number between Min_Data = 0x0000 and Max_Data = 0xFFFF. */

  uint32_t FilterMaskIdHigh;      /*!< Specifies the filter mask number or identification number,
                                       according to the mode (MSBs for a 32-bit configuration,
                                       first one for a 16-bit configuration).
                                       This parameter must be a number between Min_Data = 0x0000 and Max_Data = 0xFFFF. */

  uint32_t FilterMaskIdLow;       /*!< Specifies the filter mask number or identification number,
                                       according to the mode (LSBs for a 32-bit configuration,
                                       second one for a 16-bit configuration).
                                       This parameter must be a number between Min_Data = 0x0000 and Max_Data = 0xFFFF. */

  uint32_t FilterFIFOAssignment;  /*!< Specifies the FIFO (0 or 1U) which will be assigned to the filter.
                                       This parameter can be a value of @ref CAN_filter_FIFO */

  uint32_t FilterBank;            /*!< Specifies the filter bank which will be initialized.
                                       This parameter mus be a number between Min_Data = 0 and Max_Data = 13. */

  uint32_t FilterMode;            /*!< Specifies the filter mode to be initialized.
                                       This parameter can be a value of @ref CAN_filter_mode */

  uint32_t FilterScale;           /*!< Specifies the filter scale.
                                       This parameter can be a value of @ref CAN_filter_scale */

  uint32_t FilterActivation;      /*!< Enable or disable the filter.
                                       This parameter can be a value of @ref CAN_filter_activation */

  uint32_t SlaveStartFilterBank;  /*!< Select the start filter bank for the slave CAN instance.
                                       STM32F3xx devices don't support slave CAN instance (dual CAN). Therefore
                                       this parameter is meaningless but it has been kept for compatibility accross
                                       STM32 families. */

} CAN_FilterTypeDef;

/**
  * @brief  CAN Tx message header structure definition
  */

//from stm32f3xx.h
typedef enum
{
    DISABLE = 0U,
    ENABLE = !DISABLE
} FunctionalState;
//end of stm32f3xx.h copy

typedef struct
{
  uint32_t StdId;    /*!< Specifies the standard identifier.
                          This parameter must be a number between Min_Data = 0 and Max_Data = 0x7FF. */

  uint32_t ExtId;    /*!< Specifies the extended identifier.
                          This parameter must be a number between Min_Data = 0 and Max_Data = 0x1FFFFFFF. */

  uint32_t IDE;      /*!< Specifies the type of identifier for the message that will be transmitted.
                          This parameter can be a value of @ref CAN_identifier_type */

  uint32_t RTR;      /*!< Specifies the type of frame for the message that will be transmitted.
                          This parameter can be a value of @ref CAN_remote_transmission_request */

  uint32_t DLC;      /*!< Specifies the length of the frame that will be transmitted.
                          This parameter must be a number between Min_Data = 0 and Max_Data = 8. */

  FunctionalState TransmitGlobalTime; /*!< Specifies whether the timestamp counter value captured on start
                          of frame transmission, is sent in DATA6 and DATA7 replacing pData[6] and pData[7].
                          @note: Time Triggered Communication Mode must be enabled.
                          @note: DLC must be programmed as 8 bytes, in order these 2 bytes are sent.
                          This parameter can be set to ENABLE or DISABLE. */

} CAN_TxHeaderTypeDef;

/**
  * @brief  CAN Rx message header structure definition
  */
typedef struct
{
  uint32_t StdId;    /*!< Specifies the standard identifier.
                          This parameter must be a number between Min_Data = 0 and Max_Data = 0x7FF. */

  uint32_t ExtId;    /*!< Specifies the extended identifier.
                          This parameter must be a number between Min_Data = 0 and Max_Data = 0x1FFFFFFF. */

  uint32_t IDE;      /*!< Specifies the type of identifier for the message that will be transmitted.
                          This parameter can be a value of @ref CAN_identifier_type */

  uint32_t RTR;      /*!< Specifies the type of frame for the message that will be transmitted.
                          This parameter can be a value of @ref CAN_remote_transmission_request */

  uint32_t DLC;      /*!< Specifies the length of the frame that will be transmitted.
                          This parameter must be a number between Min_Data = 0 and Max_Data = 8. */

  uint32_t Timestamp; /*!< Specifies the timestamp counter value captured on start of frame reception.
                          @note: Time Triggered Communication Mode must be enabled.
                          This parameter must be a number between Min_Data = 0 and Max_Data = 0xFFFF. */

  uint32_t FilterMatchIndex; /*!< Specifies the index of matching acceptance filter element.
                          This parameter must be a number between Min_Data = 0 and Max_Data = 0xFF. */

} CAN_RxHeaderTypeDef;



/** @defgroup CAN_filter_mode CAN Filter Mode
  * @{
  */
#define CAN_FILTERMODE_IDMASK       (0x00000000U)  /*!< Identifier mask mode */
#define CAN_FILTERMODE_IDLIST       (0x00000001U)  /*!< Identifier list mode */
/**
  * @}
  */

/** @defgroup CAN_filter_scale CAN Filter Scale
  * @{
  */
#define CAN_FILTERSCALE_16BIT       (0x00000000U)  /*!< Two 16-bit filters */
#define CAN_FILTERSCALE_32BIT       (0x00000001U)  /*!< One 32-bit filter  */
/**
  * @}
  */

/** @defgroup CAN_filter_activation CAN Filter Activation
  * @{
  */
#define CAN_FILTER_DISABLE          (0x00000000U)  /*!< Disable filter */
#define CAN_FILTER_ENABLE           (0x00000001U)  /*!< Enable filter  */
/**
  * @}
  */

/** @defgroup CAN_filter_FIFO CAN Filter FIFO
  * @{
  */
#define CAN_FILTER_FIFO0            (0x00000000U)  /*!< Filter FIFO 0 assignment for filter x */
#define CAN_FILTER_FIFO1            (0x00000001U)  /*!< Filter FIFO 1 assignment for filter x */
/**
  * @}
  */

/** @defgroup CAN_identifier_type CAN Identifier Type
  * @{
  */
#define CAN_ID_STD                  (0x00000000U)  /*!< Standard Id */
#define CAN_ID_EXT                  (0x00000004U)  /*!< Extended Id */
/**
  * @}
  */

/** @defgroup CAN_remote_transmission_request CAN Remote Transmission Request
  * @{
  */
#define CAN_RTR_DATA                (0x00000000U)  /*!< Data frame   */
#define CAN_RTR_REMOTE              (0x00000002U)  /*!< Remote frame */
/**
  * @}
  */

/** @defgroup CAN_receive_FIFO_number CAN Receive FIFO Number
  * @{
  */
#define CAN_RX_FIFO0                (0x00000000U)  /*!< CAN receive FIFO 0 */
#define CAN_RX_FIFO1                (0x00000001U)  /*!< CAN receive FIFO 1 */
/**
  * @}
  */

/** @defgroup CAN_Tx_Mailboxes CAN Tx Mailboxes
  * @{
  */
#define CAN_TX_MAILBOX0             (0x00000001U)  /*!< Tx Mailbox 0  */
#define CAN_TX_MAILBOX1             (0x00000002U)  /*!< Tx Mailbox 1  */
#define CAN_TX_MAILBOX2             (0x00000004U)  /*!< Tx Mailbox 2  */
/**
  * @}
  */

/*
    End of copy from stm32f3xx_hal_can.h, provided by STMicroelectronics
*/


///*
//    Copied from thermistor.h, created by UBC Solar.
///*
//
///**
// * @brief Converts a raw thermistor voltage reading from an LTC6811 into a temperature
// *
// * @param[in] Vout the thermistor voltage reading to convert
// * @return the temperature of the thermistor in degrees celcius
// */
//double BTM_TEMP_volts2temp(double Vout)
//{
//	const double Vs = 5.0; // assuming the supply is 5V - measure Vref2 to check
//	const double beta = 3435.0;
//	const double room_temp = 298.15;
//	const double R_balance = 10000.0; //from LTC6811 datasheet p.85. note: this doesn't account for tolerance. to be exact, measure the 10k resistor with a multimeter
//	const double R_room_temp = 10000.0; //resistance at room temperature (25C)
//	double R_therm = 0;
//	double temp_kelvin = 0;
//	double temp_celsius = 0;
//
//	// to get the voltage in volts from the LTC6811's value,
//	// multiply it by 0.0001 as below.
//	R_therm = R_balance * ((Vs / (Vout * 0.0001)) - 1);
//	temp_kelvin = (beta * room_temp)
//		/ (beta + (room_temp * log(R_therm / R_room_temp)));
//	return temp_celsius = temp_kelvin - 273.15;
//}
//
///*
//    End of copy.
//*/

/*
    Copied from ltc6811_btm.h, created by UBC Solar
*/
#define BTM_NUM_DEVICES 1U // Number of LTC6811-1 ICs daisy chained
#define BTM_NUM_MODULES 12
#define BTM_REG_GROUP_SIZE 6 // All of the LTC6811 register groups consist of 6 bytes

typedef enum {
    MODULE_DISABLED = 0,
    MODULE_ENABLED = 1
} BTM_module_enable_t;

typedef enum {
    DISCHARGE_OFF = 0,
    DISCHARGE_ON = 1
} BTM_module_bal_status_t;

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
    BTM_module_bal_status_t bal_status;
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
    // If there is no error (error == BTM_OK), device_num should be 0
} BTM_Status_t;

#define BTM_STATUS_DEVICE_NA 0  // device number not applicable value
                                // for device_num attribute of BTM_Status_t


/*
    End of Copy
*/

///*
//    Copied from ltc6811_btm.c, created by UBC Solar.
//*/
//
//#define BTM_VOLTAGE_CONVERSION_FACTOR 0.0001
//
///**
// * @brief Converts a voltage reading from a register in the LTC6811 to a float
// * Each cell voltage is provided as a 16-bit value where
// * voltage = 0.0001V * raw value
// *
// * @param raw_reading The 16-bit reading from an LTC6811
// * @return Returns a properly scaled floating-point version of raw_reading
// */
//float BTM_regValToVoltage(uint16_t raw_reading)
//{
//	return raw_reading * BTM_VOLTAGE_CONVERSION_FACTOR;
//}
//
///*
//    End of copy.
//*/

#endif
