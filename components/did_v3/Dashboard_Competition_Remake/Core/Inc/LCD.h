#include "stm32f1xx_hal.h"



/************************************************************************
Header file containing all constants and function declarations to the LCD
*************************************************************************/
/*****************************
 ** DECLARES BUS CONNECTIONS**
 *****************************/
#define BYTEPERBITMAP 16
#define LOOKUP_COLUMN 4
#define LOOKUP_ROW 41
#define OFFSET_NUMERIC 22
#define OFFSET_CAPITAL 65
#define OFFSET_LOWCASE 97

/****************************
 ** DECLARE DATA POSITIONS **
 * **************************/

/* See Display_Horizontal_Character_Numbering.jpg in Brightside/BS Software/BS DID/ for how numbering works */

/* Page 0 */
// SOC
#define SOC_XPOS 0
#define SOC_YPOS 0
#define SOC_DATA_XPOS 15
#define SOC_DATA_YPOS 0
#define SOC_UNIT_XPOS 38
#define SOC_UNIT_YPOS 0
// Cruise Control Set
#define CRUISE_XPOS 0
#define CRUISE_YPOS 3
#define CRUISE_DATA_XPOS 15
#define CRUISE_DATA_YPOS 3
#define CRUISE_UNIT_XPOS 35
#define CRUISE_UNIT_YPOS 3
// Vehicle Velocity (Motor Speed)
#define SPEED_XPOS 0
#define SPEED_YPOS 6
#define SPEED_DATA_XPOS 15
#define SPEED_DATA_YPOS 6
#define SPEED_UNIT_XPOS 35
#define SPEED_UNIT_YPOS 6
// MCB Drive State
#define STATE_XPOS 0
#define STATE_YPOS 9
#define STATE_DATA_XPOS 15
#define STATE_DATA_YPOS 9
#define STATE_UNIT_XPOS 38
#define STATE_UNIT_YPOS 9
// Simulation Target TARGET
#define TARGET_XPOS 0
#define TARGET_YPOS 12
#define TARGET_DATA_XPOS 15
#define TARGET_DATA_YPOS 12
#define TARGET_UNIT_XPOS 35
#define TARGET_UNIT_YPOS 12

/* Page 1 */
// Warnings
// Low Voltage Warning
#define LV_WARN_XPOS 0
#define LV_WARN_YPOS 0
#define LV_WARN_DATA_XPOS 32
#define LV_WARN_DATA_YPOS 0
// High Voltage Warning
#define HV_WARN_XPOS 0
#define HV_WARN_YPOS 3
#define HV_WARN_DATA_XPOS 32
#define HV_WARN_DATA_YPOS 3
// Low Temperature Warning
#define LT_WARN_XPOS 0
#define LT_WARN_YPOS 6
#define LT_WARN_DATA_XPOS 32
#define LT_WARN_DATA_YPOS 6
// High Temperature Warning
#define HT_WARN_XPOS 0
#define HT_WARN_YPOS 9
#define HT_WARN_DATA_XPOS 32
#define HT_WARN_DATA_YPOS 9

/* Page 2 */
// Motor Current
#define MOTOR_CURRENT_XPOS 0
#define MOTOR_CURRENT_YPOS 0
#define MOTOR_CURRENT_DATA_XPOS 15
#define MOTOR_CURRENT_DATA_YPOS 0
#define MOTOR_CURRENT_UNIT_XPOS 37
#define MOTOR_CURRENT_UNIT_YPOS 0
// Array Current
#define ARRAY_CURRENT_XPOS 0
#define ARRAY_CURRENT_YPOS 3
#define ARRAY_CURRENT_DATA_XPOS 15
#define ARRAY_CURRENT_DATA_YPOS 3
#define ARRAY_CURRENT_UNIT_XPOS 37
#define ARRAY_CURRENT_UNIT_YPOS 3
// Low Voltage Current
#define LV_CURRENT_XPOS 0
#define LV_CURRENT_YPOS 6
#define LV_CURRENT_DATA_XPOS 15
#define LV_CURRENT_DATA_YPOS 6
#define LV_CURRENT_UNIT_XPOS 37
#define LV_CURRENT_UNIT_YPOS 6
// BUS Current (Total)
#define BUS_CURRENT_XPOS 0
#define BUS_CURRENT_YPOS 9
#define BUS_CURRENT_DATA_XPOS 15
#define BUS_CURRENT_DATA_YPOS 9
#define BUS_CURRENT_UNIT_XPOS 37
#define BUS_CURRENT_UNIT_YPOS 9

/* Page 3 */
// Pack Temperature
#define PACK_TEMP_XPOS 0
#define PACK_TEMP_YPOS 0
#define PACK_TEMP_DATA_XPOS 15
#define PACK_TEMP_DATA_YPOS 0
#define PACK_TEMP_UNIT_XPOS 37
#define PACK_TEMP_UNIT_YPOS 0
// Pack Voltage
#define PACK_VOLT_XPOS 0
#define PACK_VOLT_YPOS 3
#define PACK_VOLT_DATA_XPOS 15
#define PACK_VOLT_DATA_YPOS 3
#define PACK_VOLT_UNIT_XPOS 37
#define PACK_VOLT_UNIT_YPOS 3
// Voltage of Lowest Charged Cell
#define CELL_LV_XPOS 0
#define CELL_LV_YPOS 6
#define CELL_LV_DATA_XPOS 15
#define CELL_LV_DATA_YPOS 6
#define CELL_LV_UNIT_XPOS 37
#define CELL_LV_UNIT_YPOS 6
// Voltage of Lowest Charged Cell
#define CELL_HV_XPOS 0
#define CELL_HV_YPOS 9
#define CELL_HV_DATA_XPOS 15
#define CELL_HV_DATA_YPOS 9
#define CELL_HV_UNIT_XPOS 37
#define CELL_HV_UNIT_YPOS 9

/* Other Definitions */
#define PAGE_0 0
#define PAGE_1 1
#define PAGE_2 2
#define PAGE_3 3

#define PAGE_TIMEOUT 10000 // Time in ms until the DID changes back to the main page when on another page


//COLUMN 1

#define BATTERY_CURRENT_XPOS 0
#define BATTERY_CURRENT_YPOS 0
#define BATTERY_CURRENT_UNIT_XPOS 18
#define BATTERY_CURRENT_UNIT_YPOS 0

#define BATTERY_VOLTAGE_XPOS 0
#define BATTERY_VOLTAGE_YPOS 3
#define BATTERY_VOLTAGE_UNIT_XPOS 18
#define BATTERY_VOLTAGE_UNIT_YPOS 3

#define MOTOR_CURRENT_XPOS 0
//#define MOTOR_CURRENT_YPOS 6
//#define MOTOR_CURRENT_UNIT_XPOS 18
//#define MOTOR_CURRENT_UNIT_YPOS 6

#define MOTOR_TEMP_XPOS 0
#define MOTOR_TEMP_YPOS 9
#define MOTOR_TEMP_UNIT_XPOS 18
#define MOTOR_TEMP_UNIT_YPOS 9

#define MOTOR_SPEED_XPOS 0
#define MOTOR_SPEED_YPOS 12
#define MOTOR_SPEED_UNIT_XPOS 18
#define MOTOR_SPEED_UNIT_YPOS 12

//COLUMN 2

#define BATTERY_MINVOLT_XPOS 20
#define BATTERY_MINVOLT_YPOS 0
#define BATTERY_MINVOLT_UNIT_XPOS 37
#define BATTERY_MINVOLT_UNIT_YPOS 0

#define BATTERY_MAXVOLT_XPOS 20
#define BATTERY_MAXVOLT_YPOS 3
#define BATTERY_MAXVOLT_UNIT_XPOS 37
#define BATTERY_MAXVOLT_UNIT_YPOS 3

#define BATTERY_SUPPVOLT_XPOS 20
#define BATTERY_SUPPVOLT_YPOS 6
#define BATTERY_SUPPVOLT_UNIT_XPOS 37
#define BATTERY_SUPPVOLT_UNIT_YPOS 6

#define BATTERY_MAXTEMP_XPOS 20
#define BATTERY_MAXTEMP_YPOS 9
#define BATTERY_MAXTEMP_UNIT_XPOS 37
#define BATTERY_MAXTEMP_UNIT_YPOS 9

#define BATTERY_CHARGE_XPOS 22
#define BATTERY_CHARGE_YPOS 12

//DEPRECATED
#define ARRAY_MAXTEMP_XPOS 20
#define ARRAY_MAXTEMP_YPOS 0
#define ARRAY_MAXTEMP_UNIT_XPOS 37
#define ARRAY_MAXTEMP_UNIT_YPOS 3

//DEPRECATED
#define CHARGE_BAR_YPOS 12

#define HIGH 1
#define LOW 0

/**
 * Write a byte of data through C3 to C10(Data Buses)
 * @Param byte: 1 byte code
 * Returns: nothing
 */
void WriteByteToDataBus(uint8_t output_byte);
/**
 * Writes a byte of command code through ports C3 to C10(Data Buses)
 * @Param command_code: 1 byte command code
 * returns: nothing
 */
void TransmitCommand(uint8_t command_code);
/**
 * Writes a byte of parameter code through ports C3 to C10(Data Buses)
 * @Param parameter_code: 1 byte parameter code
 * returns: nothing
 */
void TransmitCommandParameter(uint8_t parameter_code);
 /**
  * Moves cursor to the position on the screen determined by XY Coordinates
  * @Param x: x-coordinate
  * @Param y: y-coordinate
  */
void MoveCursor(uint8_t x, uint8_t y);
 /**
  * Saves each subcharacter to the SG RAM 1
  * @Param bitmap: An array of the bitmap of that particular subcharacter
  * @Param offset: Some offset number
  * Returns: nothing
  */
void SaveCharacterToRAM(uint8_t* bitmap, uint8_t offset);
/**
 * Writes custom bitmaps to the LCD screen RAM
 */
void CharacterBitMaps(void);
/*
 *Purpose: Prints 1200 " " characters to clear the screen
 */
void ClearScreen(void);
/*
 * Displays all information in a single screen
 */
void DisplayScreen(void);
/**
 * Periodically called to update numerical values on the screen
 * @Param screen_state: The current state of the screen (Screen1 or Screen2)
 * @Param values[]:	An array of values to display on the appropriate screen
 * Returns: nothing
 */
void UpdateScreen(uint8_t screen_state, uint8_t values[]);
/**
 * Initialise screen settings and character bit maps
 */
void ScreenSetup(void);
/**
 * Initialise GPIO Pins C0 to C12 for output
 */
void InitialiseLCDPins(void);

/**
 * Updates Screen Titles depending on current page number
 * @Param pageNum: The current page number
 */
void UpdateScreenTitles(uint8_t pageNum);

/**
 * Erases and revalues a single value field on a screen
 * @Param x: The x value of the parameter on the screen
 * @Param y: The y value of the parameter on the screen
 * @Param integerValue: The integer value of the parameter(Between -999 to 999)
 * @Param decValue: The decimal component of the parameter
 * @param decimal_en: 1 if decimal should be included; 0 otherwise.
 */
void UpdateScreenParameter(uint8_t x, uint8_t y, int32_t integerValue, uint8_t decValue, uint8_t decimal_en);
/**
 * Periodically called to change the value of the Charge or Speed display bar
 * @Param num: The number to be represented
 * @Param max: The maximum value that the bar can display
 * @Param y: The y-coordinate of the bar
 * Returns: nothing
 */
void SetBar(uint8_t num, uint8_t max, uint8_t y);
/**
 * Outputs a string of characters to the screen
 * @Param Str[]: A string, "array of characters", whatever
 * @Param starting_x: x-coordinate to write the string to
 * @Param starting_y: y-coordinate to write the string to
 * Returns: nothing
 */
void OutputString(char Str[], uint8_t starting_x, uint8_t starting_y);
/**
 * Converts a number to a string which can be output on the screen
 * The output can take any value from '-999.9' to ' 999.9'
 * @Param num: The integer portion of the number to be displayed
 * @Param dec: The decimal portion of the number to be displayed
 * @param decimal_en: 1 if decimal should be included; 0 otherwise.
 * @Param x: x-coordinate to write the character
 * @Param y: y-coordinate to write the character
 * Returns: nothing
 */
void OutputPaddedInteger(int32_t num, uint8_t dec, uint8_t decimal_en, uint8_t x, uint8_t y);
/**
 * A Delay with a value of ~1us
 * @Param counts: Number of microseconds
 * Returns: nothing
 */
void MicrosecondDelay(uint32_t counts);
/**
 * A Delay with a value of ~1ms
 * @Param counts: Number of milliseconds
 * Returns: nothing
 */
void Delay(uint32_t counts);
