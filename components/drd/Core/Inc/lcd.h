#ifndef LCD_GRAPHICS_H
#define LCD_GRAPHICS_H

/**
 * References the library: https://github.com/mberntsen/STM32-Libraries
 */

#include "spi.h"
#include <main.h>
#include "stdint.h"
#include "stdbool.h"
#include "font_verdana.h"
#include "drive_state.h"

/** Display Parameters */
#define BOTTOM_RIGHT_X      127
#define BOTTOM_RIGHT_Y      63

/** Drive Page */
#define SPEED_FONT          (Verdana48_digits)
#define SPEED_NULL_FONT		(Verdana32)
#define SPEED_X             35
#define SPEED_ONEDIGIT_X	82
#define SPEED_TWODIGIT_X	52
#define SPEED_THREEDIGIT_X	40
#define SPEED_Y             7
#define SPEED_SPACING       -3 // ChatGPT generated font has too much padding
#define SPEED_UNIT_KPH_X	76
#define SPEED_UNIT_MPH_X	72
#define SPEED_UNIT_Y		1
#define MPH                 1
#define KPH                 0
#define SPEED_UNITS_FONT    (Verdana8)
#define SPEED_UNITS_SPACING 1

#define SOC_FONT            (Verdana16)
#define SOC_ONEDIGIT_X      13
#define SOC_TWODIGIT_X      3
#define SOC_THREEDIGIT_X    0
#define SOC_Y               0
#define SOC_SPACING         1
#define SOC_UNITS_FONT      (Verdana8)
#define SOC_UNITS_X			27
#define SOC_UNITS           '%'

#define ECO_MODE_X             	12
#define ECO_MODE_Y              24
#define ECO_MODE_FONT           (Verdana12)
#define ECO_SYMBOL              'E'
#define POWER_SYMBOL            '~'
#define POWER_MODE_X            9
#define POWER_MODE_Y            20
#define POWER_MODE_FONT         (Webdings14)
#define DRIVE_MODE_ECO          1       // ECO Mode is GPIO high (logic 1) for MDI to MC.
#define DRIVE_MODE_POWER        0

#define STATE_X             9
#define STATE_Y             46
#define STATE_FONT          (Verdana16)
#define FORWARD_STATE       0x01    
#define FORWARD_SYMBOL      'D'    
#define PARK_STATE          0x03        
#define PARK_SYMBOL         'P'    
#define REVERSE_STATE       0x04        
#define REVERSE_SYMBOL      'R'    
#define ERROR_SYMBOL        'X'
#define STATE_SPACING		1

#define FAULT_X             55
#define FAULT_Y             0
#define FAULT_SYMBOL_FONT   (Wingdings)
#define FAULT_SYMBOL		'N'

#define WARNING_X             79
#define WARNING_Y             -4
#define WARNING_SYMBOL_FONT   (Webdings14)
#define WARNING_SYMBOL		  'x'


/** Fault Page */
#define FAULT_SPACING		1

#define FAULT_LABEL_FONT		(Verdana12)
#define FAULT_LABEL_X			0
#define FAULT_LABEL_Y			0
#define FAULT_LABEL_CHARS		"FAULTS"

#define FAULT_LABEL_UNDERLINE_X 57
#define FAULT_LABEL_UNDERLINE_Y 14

#define FAULT_FOUR_FONT         (Verdana12)
#define FAULT_FOUR_X1			0
#define FAULT_FOUR_Y1			18
#define FAULT_FOUR_X2			0
#define FAULT_FOUR_Y2			34
#define FAULT_FOUR_X3			0
#define FAULT_FOUR_Y3			49

#define FAULT_EIGHT_FONT        (Verdana8)
#define FAULT_EIGHT_X1			0
#define FAULT_EIGHT_Y1			16
#define FAULT_EIGHT_X2			64
#define FAULT_EIGHT_Y2			16
#define FAULT_EIGHT_X3			0
#define FAULT_EIGHT_Y3			28
#define FAULT_EIGHT_X4			64
#define FAULT_EIGHT_Y4			28
#define FAULT_EIGHT_X5			0
#define FAULT_EIGHT_Y5			40
#define FAULT_EIGHT_X6			64
#define FAULT_EIGHT_Y6			40
#define FAULT_EIGHT_X7			0
#define FAULT_EIGHT_Y7			52
#define FAULT_EIGHT_X8			64
#define FAULT_EIGHT_Y8			52

#define BATT_FLT_CHARS		   			"BAT_FLT"
#define BATT_SUPPLO_FLT_CHARS		  	"SUPP_LOW"
#define BATT_VOLTHIGH_FLT_CHARS		  	"VOLT_HIGH"
#define BATT_VOLTLOW_FLT_CHARS		 	"VOLT_LOW"
#define BATT_SLAVE_COMM_FLT_CHARS	 	"SLV_COMM"
#define BATT_OVERVOLT_FLT_CHARS			"BAT_OVLT"
#define BATT_UNDERVOLT_FLT_CHARS		"BAT_UVLT"
#define BATT_OVERTEMP_FLT_CHARS			"BAT_OTMP"
#define BATT_CHARGE_OC_FLT_CHARS	  	"BAT_COC"
#define BATT_DISCHARGE_OC_FLT_CHARS		"BAT_DCOC"
#define BATT_RST_FROM_WATCH_FLT_CHARS	"BAT_RFW"

#define MTR_SYSTEM_FLT_CHARS		  		"MTR_SYS"
#define MTR_OVERCURR_FLT_CHARS				"MTR_OCUR"
#define MTR_OVERVOLT_FLT_CHARS				"MTR_OVLT"
#define MTR_OVERTEMP_FLT_CHARS				"MTR_OTMP"
#define MTR_COMM_FLT_CHARS					"MTR_COMM"
#define MTR_THROT_ADC_OOR_FLT_CHARS			"THRT_OOR"
#define MTR_THROT_ADC_MISMATCH_FLT_CHARS	"THRT_MSM"

/** Warning Page */
#define WARNING_SPACING			1


#define WARNING_LABEL_FONT		(Verdana12)
#define WARNING_LABEL_X			0
#define WARNING_LABEL_Y			0
#define WARNING_LABEL_CHARS		"WARNINGS"

#define WARNING_LABEL_UNDERLINE_X 85
#define WARNING_LABEL_UNDERLINE_Y 14

#define WARNING_FOUR_FONT         (Verdana12)
#define WARNING_FOUR_X1			0
#define WARNING_FOUR_Y1			18
#define WARNING_FOUR_X2			0
#define WARNING_FOUR_Y2			34
#define WARNING_FOUR_X3			0
#define WARNING_FOUR_Y3			49

#define WARNING_EIGHT_FONT          (Verdana8)
#define WARNING_EIGHT_X1			0
#define WARNING_EIGHT_Y1			16
#define WARNING_EIGHT_X2			64
#define WARNING_EIGHT_Y2			16
#define WARNING_EIGHT_X3			0
#define WARNING_EIGHT_Y3			28
#define WARNING_EIGHT_X4			64
#define WARNING_EIGHT_Y4			28
#define WARNING_EIGHT_X5			0
#define WARNING_EIGHT_Y5			40
#define WARNING_EIGHT_X6			64
#define WARNING_EIGHT_Y6			40
#define WARNING_EIGHT_X7			0
#define WARNING_EIGHT_Y7			52
#define WARNING_EIGHT_X8			64
#define WARNING_EIGHT_Y8			52

#define LOWVOLT_WARN_CHARS		"LOW_VOLT"
#define HIGHVOLT_WARN_CHARS		"HIGH_VOLT"
#define LOWTEMP_WARN_CHARS		"LOW_TEMP"
#define HIGHTEMP_WARN_CHARS		"HIGH_TEMP"
#define NOMSG_WARN_CHARS		"NO_MSG"
#define PACK_OC_WARN_CHARS		"PACK_OC"
#define PACK_OD_WARN_CHARS		"PACK_ODC"

/** Temperature Page */
#define TEMP_FONT            	(Verdana12)
#define TEMP_LABEL_FONT			(Verdana8)
#define TEMP_SPACING         	1
#define TEMP_MPPT_OFFSET		26
#define TEMP_BATT_OFFSET		34
#define TEMP_MTR_OFFSET			23
#define TEMP_UNITS_FONT      	(Verdana8)
#define TEMP_UNITS           	'C'
#define TEMP_UNITS_OFFSET		7
#define TEMP_DEGREES_FONT		(Custom)
#define TEMP_DEGREES_SYMBOL 	0xB0 // Hex ASCII value for °
#define TEMP_DEGREES_OFFSET_X	2
#define TEMP_DEGREES_OFFSET_Y	2

#define MPPT_A_LABEL			0x1
#define MPPT_A_CHARS			"PTA:"
#define MPPT_A_X				0
#define MPPT_A_Y				0

#define MPPT_B_LABEL			0x2
#define MPPT_B_CHARS			"PTB:"
#define MPPT_B_X				0
#define MPPT_B_Y				16

#define MPPT_C_LABEL			0x3
#define MPPT_C_CHARS			"PTC:"
#define MPPT_C_X				0
#define MPPT_C_Y				32

#define MPPT_D_LABEL			0x4
#define MPPT_D_CHARS			"PTD:"
#define MPPT_D_X				0
#define MPPT_D_Y				48

#define BATT_MAX_LABEL			0x5
#define BATT_MAX_CHARS			"BMAX:"
#define BATT_MAX_X				62
#define BATT_MAX_Y				0

#define BATT_MIN_LABEL			0x6
#define BATT_MIN_CHARS			"BMIN:"
#define BATT_MIN_X				62
#define BATT_MIN_Y				16

#define MTR_CONT_LABEL			0x7
#define MTR_CONT_CHARS			"MC:"
#define MTR_CONT_X				62
#define MTR_CONT_Y				32

#define MTR_THERM_LABEL			0x8
#define MTR_THERM_CHARS			"MT:"
#define MTR_THERM_X				62
#define MTR_THERM_Y				48

/** Debug Page */
#define MAX_POSITIVE_POWER              5400.0f
#define MAX_NEGATIVE_POWER              3000.0f   // use the absolute value for negative power
#define BAR_LEFT                        0
#define BAR_TOP                         0
#define BAR_BOTTOM                      15
#define BAR_RIGHT 						BOTTOM_RIGHT_X
#define CENTER_X                        43

#define DEBUG_SPEED_FONT          	(Verdana32)
#define DEBUG_SPEED_X             	35
#define DEBUG_SPEED_ONEDIGIT_X		84
#define DEBUG_SPEED_TWODIGIT_X		57
#define DEBUG_SPEED_THREEDIGIT_X	42
#define DEBUG_SPEED_Y             	22
#define DEBUG_SPEED_SPACING       	1
#define DEBUG_SPEED_UNIT_KPH_X		76
#define DEBUG_SPEED_UNIT_MPH_X		72
#define DEBUG_SPEED_UNIT_Y			22
#define DEBUG_SPEED_UNITS_FONT    	(Verdana8)
#define DEBUG_SPEED_UNITS_SPACING 	1

#define DEBUG_SOC_FONT            (Verdana16)
#define DEBUG_SOC_ONEDIGIT_X      13
#define DEBUG_SOC_TWODIGIT_X      3
#define DEBUG_SOC_THREEDIGIT_X    0
#define DEBUG_SOC_Y               23
#define DEBUG_SOC_SPACING         1
#define DEBUG_SOC_UNITS_X		  27
#define DEBUG_SOC_UNITS_FONT      (Verdana8)

#define DEBUG_ECO_MODE_X              12
#define DEBUG_ECO_MODE_Y              24
#define DEBUG_ECO_MODE_FONT           (Verdana12)
#define DEBUG_POWER_MODE_X            9
#define DEBUG_POWER_MODE_Y            20
#define DEBUG_POWER_MODE_FONT         (Webdings14)

#define DEBUG_STATE_X             9
#define DEBUG_STATE_Y             45
#define DEBUG_STATE_FONT          (Verdana16)
#define DEBUG_STATE_SPACING		1


/** LCD Screen Constants */
#define DIRTY_PAGE_CHANGE		0xFF
#define MAXPAGES				5

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
} lcd_data_t;

typedef struct {
	volatile uint8_t* temperature;
	uint8_t temp_label;
} temperature_data_t;

//typedef struct {
//	volatile uint8_t* mppt_a_temperature;
//	volatile uint8_t* mppt_b_temperature;
//	volatile uint8_t* mppt_c_temperature;
//	volatile uint8_t* mppt_d_temperature;
//	volatile uint8_t* batt_min_temperature;
//	volatile uint8_t* batt_max_temperature;
//	volatile uint8_t* motor_cont_temperature;
//	volatile uint8_t* motor_therm_temperature;
//}; temperature_data_t;

typedef struct {
   volatile bool battery_fault;
   volatile bool supp_lo;
   volatile bool voltage_high;
   volatile bool voltage_low;
   volatile bool slave_board_comm_fault;
   volatile bool overvolt_fault;
   volatile bool undervolt_fault;
   volatile bool overtemp_fault;
   volatile bool charge_overcurrent_fault;
   volatile bool discharge_overcurrent_fault;
   volatile bool reset_from_watchdog;
} lcd_batt_faults_t;

typedef struct {
   volatile bool motor_system_error;
   volatile bool overcurrent_fault;
   volatile bool overvoltage_fault;
   volatile bool fet_thermistor_error;
   volatile bool motor_comm_fault;
   volatile bool throttle_adc_outofrange;
   volatile bool throttle_adc_mismatch;
} lcd_motor_faults_t;

typedef struct {
	volatile bool low_volt_warning;
	volatile bool high_volt_warning;
	volatile bool low_temp_warning;
	volatile bool high_temp_warning;
	volatile bool no_ecu_message;
	volatile bool pack_overdischarge;
	volatile bool pack_overcharge;
} lcd_warnings_t;

typedef struct {
    uint8_t x1;
    uint8_t y1;
    uint8_t x2;
    uint8_t y2;
} bounding_box_t;

/*	User Variables	*/
extern lcd_data_t g_lcd_data;
extern lcd_batt_faults_t g_lcd_batt_faults;
extern lcd_motor_faults_t g_lcd_motor_faults;
extern lcd_warnings_t g_lcd_warnings;
extern temperature_data_t g_lcd_temperatures[8];
extern uint8_t g_LCD_page;
extern uint8_t g_LCD_page_change;

/** 
 * @brief Displays the speed on the LCD.
 * 
 * @param speed The speed value to display.
 * @param units The speed units (LCD_SPEED_UNITS_MPH or LCD_SPEED_UNITS_KPH).
 */
void LCD_display_speed_drive_page(volatile uint32_t* speed, volatile uint8_t units);

/**
 * @brief Displays the drive state on the LCD.
 * 
 * @param state The drive state (e.g., FORWARD_STATE, PARK_STATE, REVERSE_STATE).
 */
void LCD_display_drive_state_drive_page(volatile drive_state_t* state);

/**
 * @brief Displays the state of charge (SOC) on the LCD.
 * 
 * @param soc The state of charge (in percent).
 */
void LCD_display_SOC_drive_page(volatile uint32_t* soc);

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
 * @brief Displays a Temperature on the LCD (0-255)
 *
 * @param temperature A struct containing the temperature and id of the temperature.
 */
void LCD_display_temperature(temperature_data_t temperature_data);

/**
 * @brief Displays a fault indicator on the LCD Drive Page
 *
 * @param fault_indicator A general indicator to signal a fault to prompt the driver to change pages
 */
void LCD_display_fault_indicator(lcd_batt_faults_t batt_faults, lcd_motor_faults_t motor_faults);

/**
 * @brief Displays a warning indicator on the LCD Drive Page
 *
 * @param warning_indicator A general indicator to signal a warning to prompt the driver to change pages
 */
void LCD_display_warning_indicator(lcd_warnings_t warnings);

/**
 * @brief Dynamically displays battery and motor faults on the LCD
 *
 * @param batt_faults The battery faults to be displayed on the LCD
 * @param motor_faults The motor faults to be displayed on the LCD
 */
void LCD_display_faults(lcd_batt_faults_t batt_faults, lcd_motor_faults_t motor_faults);

/**
 * @brief Displays a motor faults on the LCD
 *
 * @param fault_indicator An indicator to see who
 */
void LCD_display_warnings(lcd_warnings_t warnings);

/**
 * @brief Displays the speed on the LCD.
 *
 * @param speed The speed value to display.
 * @param units The speed units (LCD_SPEED_UNITS_MPH or LCD_SPEED_UNITS_KPH).
 */
void LCD_display_speed_debug_page(volatile uint32_t* speed, volatile uint8_t units);

/**
 * @brief Displays the drive state on the LCD.
 *
 * @param state The drive state (e.g., FORWARD_STATE, PARK_STATE, REVERSE_STATE).
 */
void LCD_display_drive_state_debug_page(volatile drive_state_t* state);

/**
 * @brief Displays the state of charge (SOC) on the LCD.
 *
 * @param soc The state of charge (in percent).
 */
void LCD_display_SOC_debug_page(volatile uint32_t* soc);



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
