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

/** Drive Page */
#define SPEED_FONT          (Verdana48_digits)
#define SPEED_X             35
#define SPEED_ONEDIGIT_X	87
#define SPEED_TWODIGIT_X	57
#define SPEED_THREEDIGIT_X	45
#define SPEED_Y             5
#define SPEED_SPACING       -3 // ChatGPT generated font has too much padding
#define SPEED_UNIT_KPH_X	76
#define SPEED_UNIT_MPH_X	72
#define SPEED_UNIT_Y		1
#define MPH                 1
#define KPH                 0
#define SPEED_UNITS_FONT    (Verdana8)
#define SPEED_UNITS_SPACING 1

#define SOC_FONT            (Verdana16)
#define SOC_X               3
#define SOC_Y               0
#define SOC_SPACING         1
#define SOC_UNITS_FONT      (Verdana8)
#define SOC_UNITS           '%'
#define WIDEST_NUM_LEN_VERDANA16        11  // pixels

#define ECO_MODE_X              3
#define ECO_MODE_Y              26
#define ECO_MODE_FONT           (Verdana12)
#define ECO_SYMBOL              "E"
#define POWER_SYMBOL            '~'
#define POWER_MODE_X            6
#define POWER_MODE_Y            20
#define POWER_MODE_FONT         (Webdings14)
#define DRIVE_MODE_ECO          1       // ECO Mode is GPIO high (logic 1) for MDI to MC.
#define DRIVE_MODE_POWER        0

#define STATE_X             9
#define STATE_Y             45
#define STATE_FONT          (Verdana16)
#define FORWARD_STATE       0x01    
#define FORWARD_SYMBOL      'D'    
#define PARK_STATE          0x03        
#define PARK_SYMBOL         'P'    
#define REVERSE_STATE       0x04        
#define REVERSE_SYMBOL      'R'    
#define ERROR_SYMBOL        'X'
#define STATE_SPACING		1

/** Debug Page */
#define MAX_POSITIVE_POWER              5400.0f
#define MAX_NEGATIVE_POWER              3000.0f   // use the absolute value for negative power
#define BAR_LEFT                        1
#define BAR_TOP                         1
#define BAR_BOTTOM                      20
#define BAR_RIGHT BOTTOM_RIGHT_X
#define CENTER_X                        43

#define TEMP_FONT            	(Verdana16)
#define TEMP_X              	70
#define TEMP_Y              	1
#define TEMP_SPACING         	1
#define TEMP_UNITS_FONT      	(Verdana8)
#define TEMP_UNITS           	'C'
#define TEMP_UNITS_SPACING		7
#define TEMP_DEGREES_FONT		(Custom)
#define TEMP_DEGREES_SYMBOL 	0xB0 // Hex ASCII value for °
#define TEMP_DEGREES_SPACING	2

#define DIRTY_PAGE_CHANGE		0xFF
#define MAXPAGES				2

#define SCREEN_HEIGHT                   64
#define SCREEN_WIDTH                    128

#define ST7565_DIRTY_PAGES

#define LCD_UPDATE_DELAY 		200


/*	Datatypes */
 typedef struct {
    volatile uint32_t* speed;
    volatile uint8_t speed_units;
    volatile int16_t* pack_current;
    volatile uint16_t* pack_voltage;
    volatile uint8_t* drive_state;
    volatile uint8_t* soc;
    volatile uint8_t drive_mode;
    volatile uint8_t* temperature;
} lcd_data_t;

typedef struct {
    uint8_t x1;
    uint8_t y1;
    uint8_t x2;
    uint8_t y2;
} bounding_box_t;

/*	User Variables	*/
extern lcd_data_t g_lcd_data;
extern uint8_t g_LCD_page;
extern uint8_t g_LCD_page_change;

/** 
 * @brief Displays the speed on the LCD.
 * 
 * @param speed The speed value to display.
 * @param units The speed units (LCD_SPEED_UNITS_MPH or LCD_SPEED_UNITS_KPH).
 */
void LCD_display_speed(volatile uint32_t* speed, volatile uint8_t units);

/**
 * @brief Displays the drive state on the LCD.
 * 
 * @param state The drive state (e.g., FORWARD_STATE, PARK_STATE, REVERSE_STATE).
 */
void LCD_display_drive_state(volatile drive_state_t* state);

/**
 * @brief Displays the state of charge (SOC) on the LCD.
 * 
 * @param soc The state of charge (in percent).
 */
void LCD_display_SOC(volatile uint32_t* soc);

/**
 * @brief Displays a battery power bar based on pack current and voltage.
 * 
 * @param pack_current The battery pack current.
 * @param pack_voltage The battery pack voltage.
 */
void LCD_display_power_bar(volatile int16_t* pack_current, volatile uint16_t* pack_voltage);

/**
 * @brief Displays an E for ECO mode and P for POWER mode
 * 
 * @param drive_mode The drive mode
 */
void LCD_display_drive_mode(volatile uint8_t drive_mode);

/**
 * @brief Displays an Temperature on the LCD
 *
 * @param temperature The temperature of moto
 */
void LCD_display_temperature(volatile uint8_t* temperature);

/**
 * @brief Changes the screen
 */
void LCD_change_screen();

/*
 * @brief CAN rx function which parses message data needed by the LCD
 *
 * @param msg_id 	The id of the CAN message
 * @param data  	The data of the CAN message
 */
void LCD_CAN_rx_handle(uint32_t msg_id, uint8_t* data);

/**
 * @brief Initializes the LCD and SPI interface.
 *
 * @param hspi Pointer to the SPI handle.
 */
void LCD_init(SPI_HandleTypeDef* hspi);

#endif // LCD_GRAPHICS_H
