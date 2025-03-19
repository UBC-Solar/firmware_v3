#ifndef LCD_GRAPHICS_H
#define LCD_GRAPHICS_H

/**
 * References the library: https://github.com/mberntsen/STM32-Libraries
 */

#include "spi.h"
#include <main.h>
#include "stdint.h"
#include "font_verdana.h"
#include "drive_state.h"

/** Display Parameters */
#define BOTTOM_RIGHT_X      127
#define BOTTOM_RIGHT_Y      63

#define SPEED_FONT          (Verdana32)
#define SPEED_X             36
#define SPEED_Y             29
#define SPEED_SPACING       2
#define WIDEST_NUM_LEN_VERDANA32           24      // pixels
#define MPH                 0
#define KPH                 1
#define SPEED_UNITS_FONT    (Verdana8)
#define SPEED_UNITS_SPACING 1

#define STATE_IDX           0
#define FORWARD_STATE       0x01    
#define FORWARD_SYMBOL      'D'    
#define PARK_STATE          0x03        
#define PARK_SYMBOL         'P'    
#define REVERSE_STATE       0x04        
#define REVERSE_SYMBOL      'R'    
#define ERROR_SYMBOL        'X'    
#define STATE_X             1
#define STATE_Y             47
#define STATE_FONT          (Verdana16)
#define STATE_SPACING       1

#define SOC_FONT            (Verdana16)
#define SOC_X               95
#define SOC_Y               46
#define SOC_SPACING         1
#define SOC_UNITS_FONT      (Verdana8)
#define SOC_UNITS           '%'
#define WIDEST_NUM_LEN_VERDANA16        11  // pixels

#define MAX_POSITIVE_POWER              5400.0f
#define MAX_NEGATIVE_POWER              3000.0f   // use the absolute value for negative power
#define BAR_LEFT                        1
#define BAR_TOP                         1
#define BAR_BOTTOM                      20
#define BAR_RIGHT BOTTOM_RIGHT_X
#define CENTER_X                        43

#define ECO_MODE_X             3
#define ECO_MODE_Y             25
#define POWER_MODE_X           2
#define POWER_MODE_Y           20
#define POWER_MODE_FONT          (Webdings14)
#define ECO_MODE_FONT          (Verdana12)
#define ECO_SYMBOL              'E'    
#define POWER_SYMBOL            '~'    
#define DRIVE_MODE_ECO          0
#define DRIVE_MODE_POWER        1

#define SCREEN_HEIGHT                   64
#define SCREEN_WIDTH                    128

#define ST7565_DIRTY_PAGES

/* Font Parameters */
#define FONT_HEADER_TYPE		0
#define FONT_HEADER_ORIENTATION	1
#define FONT_HEADER_START		2
#define FONT_HEADER_LETTERS		3
#define FONT_HEADER_HEIGHT		4

#define FONT_TYPE_FIXED			0
#define FONT_TYPE_PROPORTIONAL	1

#define FONT_ORIENTATION_VERTICAL_CEILING	2

/* LCD COMMAND PARAMS */
#define CMD_SET_ADC_NORMAL          0xA0
#define CMD_DISPLAY_OFF             0xAE
#define CMD_SET_COM_NORMAL          0xC0
#define CMD_SET_BIAS_9              0xA2 
#define CMD_SET_POWER_CONTROL       0x28
#define CMD_SET_RESISTOR_RATIO      0x20
#define CMD_SET_VOLUME_FIRST        0x81
#define CMD_SET_CONTRAST            0x11
#define CMD_DISPLAY_ON              0xAF
#define CMD_SET_ALLPTS_NORMAL       0xA4
 
 /** Command: Set the current page (0..7). */
 #define CMD_SET_PAGE			0b10110000
 /** Command: set the least significant 4 bits of the column address. */
 #define CMD_COLUMN_LOWER		0b00000000
 /** Command: set the most significant 4 bits of the column address. */
 #define CMD_COLUMN_UPPER		0b00010000
 #define CMD_DISPLAY_START		0b01000000

 #define LCD_UPDATE_DELAY 200
 typedef struct {
    // Values will be set from drive_state.c
    uint8_t speed;
    uint8_t speed_units;
    uint8_t pack_current;
    uint8_t pack_voltage;
    uint8_t drive_state;
    uint8_t soc;
    uint8_t drive_mode;
} lcd_data_t;

extern lcd_data_t g_lcd_data;

/** 
 * @brief Displays the speed on the LCD.
 * 
 * @param speed The speed value to display.
 * @param units The speed units (LCD_SPEED_UNITS_MPH or LCD_SPEED_UNITS_KPH).
 */
void LCD_display_speed(uint32_t speed, int units);

/**
 * @brief Displays the drive state on the LCD.
 * 
 * @param state The drive state (e.g., FORWARD_STATE, PARK_STATE, REVERSE_STATE).
 */
void LCD_display_drive_state(drive_state_t state);

/**
 * @brief Displays the state of charge (SOC) on the LCD.
 * 
 * @param soc The state of charge (in percent).
 */
void LCD_display_SOC(uint32_t soc);

/**
 * @brief Displays a battery power bar based on pack current and voltage.
 * 
 * @param pack_current The battery pack current.
 * @param pack_voltage The battery pack voltage.
 */
void LCD_display_power_bar(float pack_current, float pack_voltage);

/**
 * @brief Displays an E for ECO mode and P for POWER mode
 * 
 * @param drive_mode The drive mode
 */
void LCD_display_drive_mode(uint8_t drive_mode);

/**
 * @brief Initializes the LCD and SPI interface.
 * 
 * @param hspi Pointer to the SPI handle.
 */
void LCD_init(SPI_HandleTypeDef* hspi);

/**
 * @brief Prints a string to the LCD at the specified coordinates.
 * 
 * @param x The x-coordinate.
 * @param line The line number.
 * @param c Pointer to the string to print.
 */
void LCD_print(uint8_t x, uint8_t line, char *c);

/**
 * @brief Sends a command to the LCD via SPI.
 * 
 * @param cmd The command byte to send.
 */
void LCD_write_command(uint8_t cmd);

/**
 * @brief Sends data to the LCD via SPI.
 * 
 * @param data The data byte to send.
 */
void LCD_write_data(uint8_t data);

#endif // LCD_GRAPHICS_H
