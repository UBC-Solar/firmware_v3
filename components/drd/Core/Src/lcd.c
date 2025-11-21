#include "lcd_driver.h"
#include "lcd.h"
#include "fault_lights.h"
#include <stdio.h>
#include "cyclic_data_handler.h"
#include "diagnostic.h"
#include "soc.h"
#include "drd_freertos.h"

/*--------------------------------------------------------------------------
  Internal Types & Variables
--------------------------------------------------------------------------*/

/* Static variables to store old bounding boxes for updating text fields */
static bounding_box_t old_bb_speed          = {0, 0, 0, 0};
static bounding_box_t old_bb_drive_state    = {0, 0, 0, 0};
static bounding_box_t old_bb_drive_mode     = {0, 0, 0, 0};
static bounding_box_t old_bb_soc            = {0, 0, 0, 0};
static bounding_box_t old_bb_temp			= {0, 0, 0, 0};

/* External variables to store page current state of the page */
lcd_data_t g_lcd_data = {0};
uint8_t g_LCD_page = 1;
uint8_t g_LCD_page_change = 0;

/*--------------------------------------------------------------------------
  Public Function Implementations
--------------------------------------------------------------------------*/

/**
 * @brief Displays the speed on the LCD.
 * 
 * @param speed The speed value to display.
 * @param units The speed units (LCD_SPEED_UNITS_MPH or LCD_SPEED_UNITS_KPH).
 */
void LCD_display_speed(volatile uint32_t* speed, volatile uint8_t units)
{
    char speed_str[12];
    /* Clear the previous speed and unit areas */
    lcd_clear_bounding_box(57, 5, 127, 53);
    
    if (speed == NULL) {  // Stale speed data
        sprintf(speed_str, "--"); 
        old_bb_speed = draw_text(speed_str, SPEED_ONEDIGIT_X, SPEED_Y, SPEED_FONT, SPEED_SPACING);
        g_diagnostics.cyclic_flags.speed_timeout = true; 
    } 
    else if (*speed < 10) { // Single digit speed
        sprintf(speed_str, "%01lu", (unsigned long)*speed);  
        old_bb_speed = draw_text(speed_str, SPEED_ONEDIGIT_X, SPEED_Y, SPEED_FONT, SPEED_SPACING);
        g_diagnostics.cyclic_flags.speed_timeout = false; 
    } else if (*speed < 100){ // Double digit second
        sprintf(speed_str, "%02lu", (unsigned long)*speed);  
        old_bb_speed = draw_text(speed_str, SPEED_TWODIGIT_X, SPEED_Y, SPEED_FONT, SPEED_SPACING);
        g_diagnostics.cyclic_flags.speed_timeout = false; 
    } else{
    	sprintf(speed_str, "%03lu", (unsigned long)*speed);
		old_bb_speed = draw_text(speed_str, SPEED_THREEDIGIT_X, SPEED_Y, SPEED_FONT, SPEED_SPACING);
		g_diagnostics.cyclic_flags.speed_timeout = false;
    }
    
    /* Draw the speed units */
    
    switch (units) {
        case KPH:
            draw_text("kph", SPEED_X + SPEED_UNIT_KPH_X, SPEED_UNIT_Y, SPEED_UNITS_FONT, SPEED_UNITS_SPACING);
            break;
        case MPH:
            draw_text("mph", SPEED_X + SPEED_UNIT_MPH_X, SPEED_UNIT_Y, SPEED_UNITS_FONT, SPEED_UNITS_SPACING);
            break;
        default:
            draw_text("xxx", SPEED_X + SPEED_UNIT_MPH_X, SPEED_UNIT_Y, SPEED_UNITS_FONT, SPEED_UNITS_SPACING);
            break;
    }
    lcd_refresh();
}

/**
 * @brief Displays the drive state on the LCD.
 * 
 * @param state The drive state (e.g., FORWARD_STATE, PARK_STATE, REVERSE_STATE).
 */
void LCD_display_drive_state(volatile drive_state_t* state)
{
    char state_str[2] = {ERROR_SYMBOL, '\0'};  // Default to error symbol.
    lcd_clear_bounding_box(23, 47, 35, 68);

    if (state == NULL) {  // Stale data for drive state
        sprintf(state_str, "-");
        g_diagnostics.cyclic_flags.drive_state_timeout = true;
    } 
    else {
        switch (*state) {
            case FORWARD:
                state_str[0] = FORWARD_SYMBOL;
                break;
            case PARK:
                state_str[0] = PARK_SYMBOL;
                break;
            case REVERSE:
                state_str[0] = REVERSE_SYMBOL;
                break;
            default:
                state_str[0] = ERROR_SYMBOL;
                break;
        }
        g_diagnostics.cyclic_flags.drive_state_timeout = false; 
    }
    old_bb_drive_state = draw_text(state_str, STATE_X, STATE_Y, STATE_FONT, STATE_SPACING);
    lcd_refresh();
}

/**
 * @brief Displays the state of charge (SOC) on the LCD.
 * 
 * @param soc The state of charge (in percent).
 */
void LCD_display_SOC(volatile uint32_t* soc)
{
    char soc_str[12];
    lcd_clear_bounding_box(23, 1, 45, 22);
    
    // Check for stale data and display "--" if so.
    if (soc == NULL) {
        sprintf(soc_str, "--");
        old_bb_soc = draw_text(soc_str, SOC_X, SOC_Y, SOC_FONT, SOC_SPACING);
        g_diagnostics.cyclic_flags.soc_timeout = true; 
    } 
    else if (*soc < 10) {
        sprintf(soc_str, "%01lu", (unsigned long)* soc);
        old_bb_soc = draw_text(soc_str, SOC_X + 10, SOC_Y, SOC_FONT, SOC_SPACING);
        g_diagnostics.cyclic_flags.soc_timeout = false;
    }
    else if (*soc < 100){
        sprintf(soc_str, "%02lu", (unsigned long)* soc);
        old_bb_soc = draw_text(soc_str, SOC_X, SOC_Y, SOC_FONT, SOC_SPACING);
        g_diagnostics.cyclic_flags.soc_timeout = false;
    }
    else {
    	sprintf(soc_str, "%03lu", (unsigned long)* soc);
    	old_bb_soc = draw_text(soc_str, SOC_X, SOC_Y, SOC_FONT, SOC_SPACING);
		g_diagnostics.cyclic_flags.soc_timeout = false;
    }

    draw_char(SOC_UNITS, SOC_X + 2 * WIDEST_NUM_LEN_VERDANA16 + 2, SOC_Y, SOC_UNITS_FONT);
    lcd_refresh();
}

/**
 * @brief Displays a battery power bar based on pack current and voltage.
 * 
 * @param pack_current The battery pack current.
 * @param pack_voltage The battery pack voltage.
 */
void LCD_display_power_bar(volatile int16_t*  pack_current, volatile uint16_t* pack_voltage)
{
    /* Clear the drawing area (including extra space for the center line) */
    lcd_clear_bounding_box(BAR_LEFT, BAR_TOP, BAR_RIGHT, BAR_BOTTOM + 3);

    /* Draw the outline of the power bar */
    draw_rectangle(BAR_LEFT, BAR_TOP, BAR_RIGHT, BAR_BOTTOM, 1);

    g_diagnostics.cyclic_flags.current_timeout = (pack_current == NULL) ? true : false;
    g_diagnostics.cyclic_flags.voltage_timeout = (pack_voltage == NULL) ? true : false;

    /* If either of voltage or current equals NULL, we display a cross over the bar*/
    if (pack_current == NULL || pack_voltage == NULL) {
        int bar_width = BAR_RIGHT - BAR_LEFT;
        int bar_height = BAR_BOTTOM - BAR_TOP;
        for (int i=0; i <= bar_width; i++) {
            int x = BAR_LEFT + i;
            int y = BAR_TOP + (i * bar_height) / bar_width;
            lcd_pixel(x, y, 1);
        }
        for (int i = 0; i <= bar_width; i++) {
            int x = BAR_RIGHT - i;
            int y = BAR_TOP + (i * bar_height) / bar_width;
            lcd_pixel(x, y, 1);
        }
        lcd_refresh();
        return;
    }
    else{
        float power = (float)*pack_current * (float)*pack_voltage;
        int fill_pixels = 0;

        if (power > 0) {
            float ratio = power / MAX_POSITIVE_POWER;
            if (ratio > 1.0f)
                ratio = 1.0f;
            int total_pixels_right = BAR_RIGHT - CENTER_X;
            fill_pixels = (int)(ratio * total_pixels_right);
            for (int y = BAR_TOP + 1; y < BAR_BOTTOM; y++) {
                for (int x = CENTER_X + 1; x <= CENTER_X + fill_pixels; x++) {
                    lcd_pixel(x, y, 1);
                }
            }
        }
        else if (power < 0) {
            float ratio = (-power) / MAX_NEGATIVE_POWER;
            if (ratio > 1.0f)
                ratio = 1.0f;
            int total_pixels_left = CENTER_X - BAR_LEFT;
            fill_pixels = (int)(ratio * total_pixels_left);
            for (int y = BAR_TOP + 1; y < BAR_BOTTOM; y++) {
                for (int x = CENTER_X - 1; x >= CENTER_X - fill_pixels; x--) {
                    lcd_pixel(x, y, 1);
                }
            }
        }

        /* Redraw the center line extending 3 pixels below the bar */
        for (int y = BAR_TOP; y <= BAR_BOTTOM + 3; y++) {
            lcd_pixel(CENTER_X, y, 1);
        }
        lcd_refresh();
    }
}

/**
 * @brief Displays an E for ECO mode and P for POWER mode
 * 
 * @param drive_mode The drive mode
 */
void LCD_display_drive_mode(volatile uint8_t drive_mode)
{
    char drive_mode_c = ERROR_SYMBOL;   // Default to error symbol.
    lcd_clear_bounding_box(23, 24, 37, 44);
    
    // Drive mode is valid, display the corresponding symbol.
    switch (drive_mode) {
        case DRIVE_MODE_ECO:
            old_bb_drive_mode = draw_text(ECO_SYMBOL, ECO_MODE_X, ECO_MODE_Y, ECO_MODE_FONT, 1);
            break;
        case DRIVE_MODE_POWER:
            old_bb_drive_mode = draw_char(POWER_SYMBOL, POWER_MODE_X, POWER_MODE_Y, POWER_MODE_FONT);
            break;
        default:
            drive_mode_c = ERROR_SYMBOL;  // Display error symbol for invalid mode.
            old_bb_drive_mode = draw_char(drive_mode_c, ECO_MODE_X, ECO_MODE_Y, ECO_MODE_FONT);
            break;
    }
    lcd_refresh();
}

/**
 * @brief Displays an Temperature on the LCD (0-255)
 *
 * @param temperature The temperature of motor
 */
void LCD_display_temperature(volatile uint8_t* temperature){
	char temp_str[4];
	lcd_clear_bounding_box(TEMP_X, TEMP_Y, old_bb_temp.x2 + 13, old_bb_temp.y2);

	// Check digits in temperature data received
	if (temperature == NULL) {  // temperature not read
		sprintf(temp_str, "--");
		old_bb_temp = draw_text(temp_str, TEMP_X, TEMP_Y, TEMP_FONT, TEMP_SPACING);
	}
	else if (*temperature < 10) { // Single digit temperature
		sprintf(temp_str, "%01lu", (unsigned long)*temperature);
		old_bb_temp = draw_text(temp_str, TEMP_X, TEMP_Y, TEMP_FONT, TEMP_SPACING);
	}
	else if(*temperature < 100) { // Double digit temperature
		sprintf(temp_str, "%02lu", (unsigned long)*temperature);
		old_bb_temp = draw_text(temp_str, TEMP_X, TEMP_Y, TEMP_FONT, TEMP_SPACING);
	}
	else { // Triple digit
		sprintf(temp_str, "%03lu", (unsigned long)*temperature);
		old_bb_temp = draw_text(temp_str, TEMP_X, TEMP_Y, TEMP_FONT, TEMP_SPACING);
	}

	// Draws the Degrees Celsius symbol according to the position of the bounding box
	draw_char(TEMP_DEGREES_SYMBOL, old_bb_temp.x2 + TEMP_DEGREES_SPACING, TEMP_Y - TEMP_DEGREES_SPACING, TEMP_DEGREES_FONT);
	draw_char(TEMP_UNITS, old_bb_temp.x2 + TEMP_UNITS_SPACING, TEMP_Y, TEMP_UNITS_FONT);

	lcd_refresh();
}





/**
 * @brief Changes the screen
 */
void LCD_change_screen(){
	lcd_dirty_pages = DIRTY_PAGE_CHANGE;
	lcd_clear_bounding_box(0,0, BOTTOM_RIGHT_X ,BOTTOM_RIGHT_Y);
	lcd_refresh();
}

/*
 * @brief CAN rx function which parses message data needed by the LCD
 *
 * @param msg_id 	The id of the CAN message
 * @param data  	The data of the CAN message
 */
void LCD_CAN_rx_handle(uint32_t msg_id, uint8_t* data)
{
	if(msg_id == CAN_ID_PACK_CURRENT)
	{
        int16_t tmp_pack_current = (data[1] << 8) | (data[0]);
        tmp_pack_current /= 65.535;
        set_cyclic_pack_current(tmp_pack_current);

        g_pack_current_soc = tmp_pack_current;
	}
    
    if(msg_id == CAN_ID_PACK_VOLTAGE)
	{
        uint16_t tmp_pack_voltage = (data[1] << 8) | (data[0]);
        tmp_pack_voltage /= PACK_VOLTAGE_DIVISOR;
		set_cyclic_pack_voltage(tmp_pack_voltage);
        
        g_total_pack_voltage_soc = tmp_pack_voltage;
        
        osEventFlagsSet(calculate_soc_flagHandle, SOC_CALCULATE_ON);
	}

    if(msg_id == STR_CAN_MSG_ID)
    {
    	uint8_t next_page = (data[0] & 1);
    	uint8_t previous_page = (data[0] >> 1) & 1;

    	if(next_page){
    		if(g_LCD_page < MAXPAGES){
    			g_LCD_page_change = 1;
    			g_LCD_page++;
			}
    	}
    	else if(previous_page){
    		if(g_LCD_page > 1){
    			g_LCD_page_change = 1;
    			g_LCD_page--;
			}
    	}
    }

    if(msg_id == CAN_ID_MDI_TEMP)
    {
    	uint8_t temperature = data[0];
    	set_cyclic_temperature(temperature);
    }
}
