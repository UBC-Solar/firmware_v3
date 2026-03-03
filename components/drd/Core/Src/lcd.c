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
static bounding_box_t old_bb_fault_indicator = {0, 0, 0, 0};
static bounding_box_t old_bb_warning_indicator = {0, 0, 0, 0};

static char faults[8][10] = {0};
static char warning_char[8][10] = {0};

/* External variables*/
lcd_data_t g_lcd_data = {0};
lcd_batt_faults_t g_lcd_batt_faults = {0};
lcd_motor_faults_t g_lcd_motor_faults = {0};
lcd_warnings_t g_lcd_warnings = {0};
temperature_data_t g_lcd_temperatures[8] = {0};

uint8_t g_LCD_page = 1;
uint8_t g_LCD_page_change = 0;

/*--------------------------------------------------------------------------
  HELPER FUNCTIONS
--------------------------------------------------------------------------*/

/**
 * @brief Checks and updates the faults.
 *
 * @param batt_faults A struct containing the battery faults.
 * @param motor_faults A struct containing the motor faults.
 * @return The count of faults.
 */
uint8_t check_faults(lcd_batt_faults_t batt_faults, lcd_motor_faults_t motor_faults){
	uint8_t fault_count = 0;

	if (batt_faults.battery_fault) {
		sprintf(faults[fault_count], "%s", BATT_FLT_CHARS);
		fault_count++;
	}
	if (batt_faults.supp_lo) {
		sprintf(faults[fault_count], "%s", BATT_SUPPLO_FLT_CHARS);
		fault_count++;
	}
	if (batt_faults.voltage_high) {
		sprintf(faults[fault_count], "%s", BATT_VOLTHIGH_FLT_CHARS);
		fault_count++;
	}
	if (batt_faults.voltage_low) {
		sprintf(faults[fault_count], "%s", BATT_VOLTLOW_FLT_CHARS);
		fault_count++;
	}
	if (batt_faults.slave_board_comm_fault) {
		sprintf(faults[fault_count], "%s", BATT_SLAVE_COMM_FLT_CHARS);
		fault_count++;
	}
	if (batt_faults.overvolt_fault) {
		sprintf(faults[fault_count], "%s", BATT_OVERVOLT_FLT_CHARS);
		fault_count++;
	}
	if (batt_faults.undervolt_fault) {
		sprintf(faults[fault_count], "%s", BATT_UNDERVOLT_FLT_CHARS);
		fault_count++;
	}
	if (batt_faults.overtemp_fault) {
		sprintf(faults[fault_count], "%s", BATT_OVERTEMP_FLT_CHARS);
		fault_count++;
	}
	if (batt_faults.charge_overcurrent_fault) {
		sprintf(faults[fault_count], "%s", BATT_CHARGE_OC_FLT_CHARS);
		fault_count++;
	}
	if (batt_faults.discharge_overcurrent_fault) {
		sprintf(faults[fault_count], "%s", BATT_DISCHARGE_OC_FLT_CHARS);
		fault_count++;
	}
	if (batt_faults.reset_from_watchdog) {
		sprintf(faults[fault_count], "%s", BATT_RST_FROM_WATCH_FLT_CHARS);
		fault_count++;
	}
	if (motor_faults.motor_system_error) {
		sprintf(faults[fault_count], "%s", MTR_SYSTEM_FLT_CHARS);
		fault_count++;
	}
	if (motor_faults.overcurrent_fault) {
		sprintf(faults[fault_count], "%s", MTR_OVERCURR_FLT_CHARS);
		fault_count++;
	}
	if (motor_faults.overvoltage_fault) {
		sprintf(faults[fault_count], "%s", MTR_OVERVOLT_FLT_CHARS);
		fault_count++;
	}
	if (motor_faults.fet_thermistor_error) {
		sprintf(faults[fault_count], "%s", MTR_OVERTEMP_FLT_CHARS);
		fault_count++;
	}
	if (motor_faults.motor_comm_fault) {
		sprintf(faults[fault_count], "%s", MTR_COMM_FLT_CHARS);
		fault_count++;
	}
	if (motor_faults.throttle_adc_outofrange) {
		sprintf(faults[fault_count], "%s", MTR_THROT_ADC_OOR_FLT_CHARS);
		fault_count++;
	}
	if (motor_faults.throttle_adc_mismatch) {
		sprintf(faults[fault_count], "%s",  MTR_THROT_ADC_MISMATCH_FLT_CHARS);
		fault_count++;
	}

	return fault_count;
}

/**
 * @brief Checks and updates the warnings.
 *
 * @param warnings A struct containing the warnings
 *
 * @return The count of warnings.
 */
uint8_t check_warnings(lcd_warnings_t warnings){
	uint8_t warning_count = 0;

	if (warnings.high_temp_warning) {
		sprintf(warning_char[warning_count], "%s", HIGHTEMP_WARN_CHARS);
		warning_count++;
	}
	if (warnings.high_volt_warning) {
		sprintf(warning_char[warning_count], "%s", HIGHVOLT_WARN_CHARS);
		warning_count++;
	}
	if (warnings.low_temp_warning) {
		sprintf(warning_char[warning_count], "%s", LOWTEMP_WARN_CHARS);
		warning_count++;
	}
	if (warnings.low_volt_warning) {
		sprintf(warning_char[warning_count], "%s", LOWVOLT_WARN_CHARS);
		warning_count++;
	}
	if (warnings.no_ecu_message) {
		sprintf(warning_char[warning_count], "%s", NOMSG_WARN_CHARS);
		warning_count++;
	}
	if (warnings.pack_overcharge) {
		sprintf(warning_char[warning_count], "%s", PACK_OC_WARN_CHARS);
		warning_count++;
	}
	if (warnings.pack_overdischarge) {
		sprintf(warning_char[warning_count], "%s", PACK_OD_WARN_CHARS);
		warning_count++;
	}

	return warning_count;
}

/*--------------------------------------------------------------------------
  PAGE 1 (DRIVE PAGE) FUNCTIONS
--------------------------------------------------------------------------*/

/**
 * @brief Displays the speed on the LCD drive page.
 * 
 * @param speed The speed value to display.
 * @param units The speed units (LCD_SPEED_UNITS_MPH or LCD_SPEED_UNITS_KPH).
 */
void LCD_display_speed_drive_page(volatile uint32_t* speed, volatile uint8_t units)
{
    char speed_str[12];
    
	/* Clear the previous speed and unit areas */
	lcd_clear_bounding_box(SPEED_THREEDIGIT_X, old_bb_speed.y1 + 10, BOTTOM_RIGHT_X, old_bb_speed.y2);

	if (speed == NULL) {  // Stale speed data
		sprintf(speed_str, "XX");
		old_bb_speed = draw_text(speed_str, SPEED_TWODIGIT_X + 10, SPEED_Y + 10, SPEED_NULL_FONT, SPEED_SPACING + 10);
		g_diagnostics.cyclic_flags.speed_timeout = true;
	} else if (*speed < 10) { // Single digit speed
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
 * @brief Displays the state of charge (SOC) on the LCD drive page.
 * 
 * @param soc The state of charge (in percent).
 */
void LCD_display_SOC_drive_page(volatile uint32_t* soc)
{
    char soc_str[12];
    
	lcd_clear_bounding_box(old_bb_soc.x1, old_bb_soc.y1, old_bb_soc.x2, old_bb_soc.y2 - SOC_SPACING);

	// Check for stale data and display "--" if so.
	if (soc == NULL) {
		sprintf(soc_str, "--");
		old_bb_soc = draw_text(soc_str, SOC_TWODIGIT_X, SOC_Y, SOC_FONT, SOC_SPACING);
		g_diagnostics.cyclic_flags.soc_timeout = true;
	}
	else if (*soc < 10) {
		sprintf(soc_str, "%01lu", (unsigned long)* soc);
		old_bb_soc = draw_text(soc_str, SOC_ONEDIGIT_X, SOC_Y, SOC_FONT, SOC_SPACING);
		g_diagnostics.cyclic_flags.soc_timeout = false;
	}
	else if (*soc < 100){
		sprintf(soc_str, "%02lu", (unsigned long)* soc);
		old_bb_soc = draw_text(soc_str, SOC_TWODIGIT_X, SOC_Y, SOC_FONT, SOC_SPACING);
		g_diagnostics.cyclic_flags.soc_timeout = false;
	}
	else {
		sprintf(soc_str, "%03lu", (unsigned long)* soc);
		old_bb_soc = draw_text(soc_str, SOC_THREEDIGIT_X, SOC_Y, SOC_FONT, SOC_SPACING);
		g_diagnostics.cyclic_flags.soc_timeout = false;
	}

	draw_char(SOC_UNITS, SOC_UNITS_X, SOC_Y, SOC_UNITS_FONT);

    lcd_refresh();
}

/**
 * @brief Displays an E for ECO mode and P for POWER mode
 *
 * @param drive_mode The drive mode
 */
void LCD_display_drive_mode(volatile uint8_t drive_mode)
{
	lcd_clear_bounding_box(old_bb_drive_mode.x1, old_bb_drive_mode.y1 + 4,
							   old_bb_drive_mode.x2, old_bb_drive_mode.y2 - 2);

	// Drive mode is valid, display the corresponding symbol.
	switch (drive_mode) {
		case DRIVE_MODE_ECO:
			old_bb_drive_mode = draw_char(ECO_SYMBOL, ECO_MODE_X, ECO_MODE_Y, ECO_MODE_FONT);
			break;
		case DRIVE_MODE_POWER:
			old_bb_drive_mode = draw_char(POWER_SYMBOL, POWER_MODE_X, POWER_MODE_Y, POWER_MODE_FONT);
			break;
		default:
			old_bb_drive_mode = draw_char(ERROR_SYMBOL, ECO_MODE_X, ECO_MODE_Y, ECO_MODE_FONT);
			break;
	}

    lcd_refresh();
}

/**
 * @brief Displays the drive state on the LCD drive page.
 *
 * @param state The drive state (e.g., FORWARD_STATE, PARK_STATE, REVERSE_STATE).
 */
void LCD_display_drive_state_drive_page(volatile drive_state_t* state)
{
    char state_str[2] = {ERROR_SYMBOL, '\0'};  // Default to error symbol.

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

    lcd_clear_bounding_box(STATE_X, STATE_Y, old_bb_drive_state.x2, BOTTOM_RIGHT_Y);
	old_bb_drive_state = draw_text(state_str, STATE_X, STATE_Y, STATE_FONT, STATE_SPACING);

    lcd_refresh();
}


/**
 * @brief Displays a fault indicator on the LCD Drive Page
 *
 * @param fault_indicator A general indicator to signal a fault to prompt the driver to change pages
 */
void LCD_display_fault_indicator(lcd_batt_faults_t batt_faults, lcd_motor_faults_t motor_faults)
{
	lcd_clear_bounding_box(old_bb_fault_indicator.x1, old_bb_fault_indicator.y1,
			old_bb_fault_indicator.x2, old_bb_fault_indicator.y2);


	uint8_t fault_count = check_faults(batt_faults, motor_faults);

	// Check if there is an existing fault
	if(fault_count > 0) {
		old_bb_fault_indicator = draw_char(FAULT_SYMBOL, FAULT_X, FAULT_Y, FAULT_SYMBOL_FONT);
	}
	lcd_refresh();
}

/**
 * @brief Displays a warning indicator on the LCD Drive Page
 *
 * @param warning_indicator A general indicator to signal a warning to prompt the driver to change pages
 */
void LCD_display_warning_indicator(lcd_warnings_t warnings)
{
	lcd_clear_bounding_box(old_bb_warning_indicator.x1, old_bb_warning_indicator.y1,
			old_bb_warning_indicator.x2, old_bb_warning_indicator.y2);

	uint8_t warning_count = check_warnings(warnings);

	// Check if there is an existing warning
	if(warning_count > 0){
		old_bb_warning_indicator = draw_char(WARNING_SYMBOL, WARNING_X, WARNING_Y, WARNING_SYMBOL_FONT);
	}
	lcd_refresh();
}

/*--------------------------------------------------------------------------
  PAGE 2 (FAULT PAGE) FUNCTIONS
--------------------------------------------------------------------------*/

/**
 * @brief Dynamically displays battery and motor faults on the LCD
 *
 * @param batt_faults The battery faults to be displayed on the LCD
 * @param motor_faults The motor faults to be displayed on the LCD
 */
void LCD_display_faults(lcd_batt_faults_t batt_faults, lcd_motor_faults_t motor_faults){

	lcd_clear_bounding_box(0, FAULT_FOUR_Y1, BOTTOM_RIGHT_X, BOTTOM_RIGHT_Y);

	uint8_t fault_count = check_faults(batt_faults, motor_faults);

	draw_text(FAULT_LABEL_CHARS, FAULT_LABEL_X, FAULT_LABEL_Y, FAULT_LABEL_FONT, FAULT_SPACING);
	for(uint8_t i = 0; i < FAULT_LABEL_UNDERLINE_X; i++){
		lcd_pixel(i, FAULT_LABEL_UNDERLINE_Y, 1);
	}
	if(fault_count <= 3) {
		draw_text(faults[0], FAULT_FOUR_X1, FAULT_FOUR_Y1, FAULT_FOUR_FONT, FAULT_SPACING);
		draw_text(faults[1], FAULT_FOUR_X2, FAULT_FOUR_Y2, FAULT_FOUR_FONT, FAULT_SPACING);
		draw_text(faults[2], FAULT_FOUR_X3, FAULT_FOUR_Y3, FAULT_FOUR_FONT, FAULT_SPACING);
	}
	else if(fault_count <= 8) {
		draw_text(faults[0], FAULT_EIGHT_X1, FAULT_EIGHT_Y1, FAULT_EIGHT_FONT, FAULT_SPACING);
		draw_text(faults[1], FAULT_EIGHT_X2, FAULT_EIGHT_Y2, FAULT_EIGHT_FONT, FAULT_SPACING);
		draw_text(faults[2], FAULT_EIGHT_X3, FAULT_EIGHT_Y3, FAULT_EIGHT_FONT, FAULT_SPACING);
		draw_text(faults[3], FAULT_EIGHT_X4, FAULT_EIGHT_Y4, FAULT_EIGHT_FONT, FAULT_SPACING);
		draw_text(faults[4], FAULT_EIGHT_X5, FAULT_EIGHT_Y5, FAULT_EIGHT_FONT, FAULT_SPACING);
		draw_text(faults[5], FAULT_EIGHT_X6, FAULT_EIGHT_Y6, FAULT_EIGHT_FONT, FAULT_SPACING);
		draw_text(faults[6], FAULT_EIGHT_X7, FAULT_EIGHT_Y7, FAULT_EIGHT_FONT, FAULT_SPACING);
		draw_text(faults[7], FAULT_EIGHT_X8, FAULT_EIGHT_Y8, FAULT_EIGHT_FONT, FAULT_SPACING);
	}

	lcd_refresh();

}

/*--------------------------------------------------------------------------
  PAGE 3 (WARNING PAGE) FUNCTIONS
--------------------------------------------------------------------------*/

/**
 * @brief Displays a motor faults on the LCD
 *
 * @param fault_indicator An indicator to see who
 */
void LCD_display_warnings(lcd_warnings_t warnings)
{
	lcd_clear_bounding_box(0, WARNING_FOUR_Y1, BOTTOM_RIGHT_X, BOTTOM_RIGHT_Y);

	uint8_t warning_count = check_warnings(warnings);

	draw_text(WARNING_LABEL_CHARS, WARNING_LABEL_X, WARNING_LABEL_Y, WARNING_LABEL_FONT, WARNING_SPACING);
	for(uint8_t i = 0; i < WARNING_LABEL_UNDERLINE_X; i++){
		lcd_pixel(i, WARNING_LABEL_UNDERLINE_Y, 1);
	}
	if(warning_count <= 3) {
		draw_text(warning_char[0], WARNING_FOUR_X1, WARNING_FOUR_Y1, WARNING_FOUR_FONT, WARNING_SPACING);
		draw_text(warning_char[1], WARNING_FOUR_X2, WARNING_FOUR_Y2, WARNING_FOUR_FONT, WARNING_SPACING);
		draw_text(warning_char[2], WARNING_FOUR_X3, WARNING_FOUR_Y3, WARNING_FOUR_FONT, WARNING_SPACING);
	}
	else if(warning_count <= 8) {
		draw_text(warning_char[0], WARNING_EIGHT_X1, WARNING_EIGHT_Y1, WARNING_EIGHT_FONT, WARNING_SPACING);
		draw_text(warning_char[1], WARNING_EIGHT_X2, WARNING_EIGHT_Y2, WARNING_EIGHT_FONT, WARNING_SPACING);
		draw_text(warning_char[2], WARNING_EIGHT_X3, WARNING_EIGHT_Y3, WARNING_EIGHT_FONT, WARNING_SPACING);
		draw_text(warning_char[3], WARNING_EIGHT_X4, WARNING_EIGHT_Y4, WARNING_EIGHT_FONT, WARNING_SPACING);
		draw_text(warning_char[4], WARNING_EIGHT_X5, WARNING_EIGHT_Y5, WARNING_EIGHT_FONT, WARNING_SPACING);
		draw_text(warning_char[5], WARNING_EIGHT_X6, WARNING_EIGHT_Y6, WARNING_EIGHT_FONT, WARNING_SPACING);
		draw_text(warning_char[6], WARNING_EIGHT_X7, WARNING_EIGHT_Y7, WARNING_EIGHT_FONT, WARNING_SPACING);
		draw_text(warning_char[7], WARNING_EIGHT_X8, WARNING_EIGHT_Y8, WARNING_EIGHT_FONT, WARNING_SPACING);
	}

	lcd_refresh();
}

/*--------------------------------------------------------------------------
  PAGE 4 (TEMPERATURE PAGE) FUNCTIONS
--------------------------------------------------------------------------*/

/**
 * @brief Displays a Temperature on the LCD (0-255)
 *
 * @param temperature A struct containing the temperature and id of the temperature.
 */
void LCD_display_temperature(temperature_data_t temperature_data){

	// TODO: When assigning with CAN ensure that the name is set too

	// Stores a Bounding Box used for changing temp symbol position
	bounding_box_t bb = {0};

	// Variables to hold values based on what temperature is being displayed
	char temp_label[8];
	char temp_str[4];
	uint8_t temp_x;
	uint8_t temp_y;
	uint8_t temp_shift;

	switch(temperature_data.temp_label){
		case MPPT_A_LABEL:
			sprintf(temp_label, "%s", MPPT_A_CHARS);
			temp_x = MPPT_A_X;
			temp_y = MPPT_A_Y;
			temp_shift = TEMP_MPPT_OFFSET;
			lcd_clear_bounding_box(TEMP_MPPT_OFFSET, MPPT_A_Y, BATT_MAX_X-2, MPPT_B_Y);
			break;
		case MPPT_B_LABEL:
			sprintf(temp_label, "%s", MPPT_B_CHARS);
			temp_x = MPPT_B_X;
			temp_y = MPPT_B_Y;
			temp_shift = TEMP_MPPT_OFFSET;
			lcd_clear_bounding_box(TEMP_MPPT_OFFSET, MPPT_B_Y, BATT_MAX_X-2, MPPT_C_Y);
			break;
		case MPPT_C_LABEL:
			sprintf(temp_label, "%s", MPPT_C_CHARS);
			temp_x = MPPT_C_X;
			temp_y = MPPT_C_Y;
			temp_shift = TEMP_MPPT_OFFSET;
			lcd_clear_bounding_box(TEMP_MPPT_OFFSET, MPPT_C_Y, BATT_MAX_X-2, MPPT_D_Y);
			break;
		case MPPT_D_LABEL:
			sprintf(temp_label, "%s", MPPT_D_CHARS);
			temp_x = MPPT_D_X;
			temp_y = MPPT_D_Y;
			temp_shift = TEMP_MPPT_OFFSET;
			lcd_clear_bounding_box(TEMP_MPPT_OFFSET, MPPT_D_Y, BATT_MAX_X-2, BOTTOM_RIGHT_Y);
			break;
		case BATT_MAX_LABEL:
			sprintf(temp_label, "%s", BATT_MAX_CHARS);
			temp_x = BATT_MAX_X;
			temp_y = BATT_MAX_Y;
			temp_shift = TEMP_BATT_OFFSET;
			lcd_clear_bounding_box(BATT_MAX_X + TEMP_BATT_OFFSET, 0, BOTTOM_RIGHT_X, BATT_MIN_Y);
			break;
		case BATT_MIN_LABEL:
			sprintf(temp_label, "%s", BATT_MIN_CHARS);
			temp_x = BATT_MIN_X;
			temp_y = BATT_MIN_Y;
			temp_shift = TEMP_BATT_OFFSET;
			lcd_clear_bounding_box(BATT_MAX_X + TEMP_BATT_OFFSET, BATT_MIN_Y, BOTTOM_RIGHT_X, MTR_CONT_Y);
			break;
		case MTR_CONT_LABEL:
			sprintf(temp_label, "%s", MTR_CONT_CHARS);
			temp_x = MTR_CONT_X;
			temp_y = MTR_CONT_Y;
			temp_shift = TEMP_MTR_OFFSET;
			lcd_clear_bounding_box(MTR_CONT_X + TEMP_MTR_OFFSET, MTR_CONT_Y, BOTTOM_RIGHT_X, MTR_THERM_Y);
			break;
		case MTR_THERM_LABEL:
			sprintf(temp_label, "%s", MTR_THERM_CHARS);
			temp_x = MTR_THERM_X;
			temp_y = MTR_THERM_Y;
			temp_shift = TEMP_MTR_OFFSET;
			lcd_clear_bounding_box(MTR_CONT_X + TEMP_MTR_OFFSET, MTR_THERM_Y, BOTTOM_RIGHT_X, BOTTOM_RIGHT_Y);
			break;
		default:
			break;
	}

	draw_text(temp_label, temp_x, temp_y, TEMP_LABEL_FONT, TEMP_SPACING);

	// Check digits in temperature data received
	if (temperature_data.temperature == NULL) {  // temperature not read
		sprintf(temp_str, "--");
		bb = draw_text(temp_str, temp_x + temp_shift, temp_y, TEMP_FONT, TEMP_SPACING);
	}
	else if (*temperature_data.temperature < 10) { // Single digit temperature
		sprintf(temp_str, "%01lu", (unsigned long)*temperature_data.temperature);
		bb = draw_text(temp_str, temp_x + temp_shift, temp_y, TEMP_FONT, TEMP_SPACING);
	}
	else if(*temperature_data.temperature < 100) { // Double digit temperature
		sprintf(temp_str, "%02lu", (unsigned long)*temperature_data.temperature);
		bb = draw_text(temp_str, temp_x + temp_shift, temp_y, TEMP_FONT, TEMP_SPACING);
	}
	else { // Triple digit
		sprintf(temp_str, "%03lu", (unsigned long)*temperature_data.temperature);
		bb = draw_text(temp_str, temp_x + temp_shift, temp_y, TEMP_FONT, TEMP_SPACING);
	}

	// Draws the Degrees Celsius symbol according to the position of the bounding box
	draw_char(TEMP_DEGREES_SYMBOL, bb.x2 + TEMP_DEGREES_OFFSET_X, temp_y - TEMP_DEGREES_OFFSET_Y, TEMP_DEGREES_FONT);
	draw_char(TEMP_UNITS, bb.x2 + TEMP_UNITS_OFFSET, temp_y, TEMP_UNITS_FONT);

	lcd_refresh();
}


/*--------------------------------------------------------------------------
  PAGE 5 (DEBUG PAGE) FUNCTIONS
--------------------------------------------------------------------------*/

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
 * @brief Displays the speed on the LCD debug page.
 *
 * @param speed The speed value to display.
 * @param units The speed units (LCD_SPEED_UNITS_MPH or LCD_SPEED_UNITS_KPH).
 */
void LCD_display_speed_debug_page(volatile uint32_t* speed, volatile uint8_t units)
{
    char speed_str[12];

	/* Clear the previous speed and unit areas */
	lcd_clear_bounding_box(DEBUG_SPEED_THREEDIGIT_X, old_bb_speed.y1, BOTTOM_RIGHT_X, old_bb_speed.y2 - 3);

	if (speed == NULL) {  // Stale speed data
		sprintf(speed_str, "XX");
		old_bb_speed = draw_text(speed_str, DEBUG_SPEED_TWODIGIT_X + 10, DEBUG_SPEED_Y + 10, DEBUG_SPEED_FONT, DEBUG_SPEED_SPACING + 10);
		g_diagnostics.cyclic_flags.speed_timeout = true;
	} else if (*speed < 10) { // Single digit speed
		sprintf(speed_str, "%01lu", (unsigned long)*speed);
		old_bb_speed = draw_text(speed_str, DEBUG_SPEED_ONEDIGIT_X, DEBUG_SPEED_Y, DEBUG_SPEED_FONT, DEBUG_SPEED_SPACING);
		g_diagnostics.cyclic_flags.speed_timeout = false;
	} else if (*speed < 100){ // Double digit second
		sprintf(speed_str, "%02lu", (unsigned long)*speed);
		old_bb_speed = draw_text(speed_str, DEBUG_SPEED_TWODIGIT_X, DEBUG_SPEED_Y, DEBUG_SPEED_FONT, DEBUG_SPEED_SPACING);
		g_diagnostics.cyclic_flags.speed_timeout = false;
	} else{
		sprintf(speed_str, "%03lu", (unsigned long)*speed);
		old_bb_speed = draw_text(speed_str, DEBUG_SPEED_THREEDIGIT_X, DEBUG_SPEED_Y, DEBUG_SPEED_FONT, DEBUG_SPEED_SPACING);
		g_diagnostics.cyclic_flags.speed_timeout = false;
	}

	/* Draw the speed units */
	switch (units) {
		case KPH:
			draw_text("kph", DEBUG_SPEED_X + DEBUG_SPEED_UNIT_KPH_X, DEBUG_SPEED_UNIT_Y, DEBUG_SPEED_UNITS_FONT, DEBUG_SPEED_UNITS_SPACING);
			break;
		case MPH:
			draw_text("mph", DEBUG_SPEED_X + DEBUG_SPEED_UNIT_MPH_X, DEBUG_SPEED_UNIT_Y, DEBUG_SPEED_UNITS_FONT, DEBUG_SPEED_UNITS_SPACING);
			break;
		default:
			draw_text("xxx", DEBUG_SPEED_X + DEBUG_SPEED_UNIT_MPH_X, DEBUG_SPEED_UNIT_Y, DEBUG_SPEED_UNITS_FONT, DEBUG_SPEED_UNITS_SPACING);
			break;
	}

    lcd_refresh();

}

/**
 * @brief Displays the state of charge (SOC) on the LCD debug page.
 *
 * @param soc The state of charge (in percent).
 */
void LCD_display_SOC_debug_page(volatile uint32_t* soc)
{
    char soc_str[12];

	lcd_clear_bounding_box(old_bb_soc.x1, old_bb_soc.y1, old_bb_soc.x2, old_bb_soc.y2 - 5);

	// Check for stale data and display "--" if so.
	if (soc == NULL) {
		sprintf(soc_str, "--");
		old_bb_soc = draw_text(soc_str, DEBUG_SOC_TWODIGIT_X, SOC_Y, DEBUG_SOC_FONT, DEBUG_SOC_SPACING);
		g_diagnostics.cyclic_flags.soc_timeout = true;
	}
	else if (*soc < 10) {
		sprintf(soc_str, "%01lu", (unsigned long)* soc);
		old_bb_soc = draw_text(soc_str, DEBUG_SOC_ONEDIGIT_X, DEBUG_SOC_Y, DEBUG_SOC_FONT, DEBUG_SOC_SPACING);
		g_diagnostics.cyclic_flags.soc_timeout = false;
	}
	else if (*soc < 100){
		sprintf(soc_str, "%02lu", (unsigned long)* soc);
		old_bb_soc = draw_text(soc_str, DEBUG_SOC_TWODIGIT_X, DEBUG_SOC_Y, DEBUG_SOC_FONT, DEBUG_SOC_SPACING);
		g_diagnostics.cyclic_flags.soc_timeout = false;
	}
	else {
		sprintf(soc_str, "%03lu", (unsigned long)* soc);
		old_bb_soc = draw_text(soc_str, DEBUG_SOC_THREEDIGIT_X, DEBUG_SOC_Y, DEBUG_SOC_FONT, DEBUG_SOC_SPACING);
		g_diagnostics.cyclic_flags.soc_timeout = false;
	}

	draw_char(SOC_UNITS, DEBUG_SOC_UNITS_X, DEBUG_SOC_Y, DEBUG_SOC_UNITS_FONT);

    lcd_refresh();
}

/**
 * @brief Displays the drive state on the LCD debug page.
 *
 * @param state The drive state (e.g., FORWARD_STATE, PARK_STATE, REVERSE_STATE).
 */
void LCD_display_drive_state_debug_page(volatile drive_state_t* state)
{
    char state_str[2] = {ERROR_SYMBOL, '\0'};  // Default to error symbol.

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

	lcd_clear_bounding_box(DEBUG_STATE_X, DEBUG_STATE_Y, 20, BOTTOM_RIGHT_Y);
	old_bb_drive_state = draw_text(state_str, DEBUG_STATE_X, DEBUG_STATE_Y, DEBUG_STATE_FONT, DEBUG_STATE_SPACING);

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

    	if(next_page){
    		if(g_LCD_page < MAXPAGES){
    			g_LCD_page_change = 1;
    			g_LCD_page++;
			} else {
				g_LCD_page_change = 1;
				g_LCD_page = 1;
			}
    	}
//    	else if(previous_page){
//    		if(g_LCD_page > 1){
//    			g_LCD_page_change = 1;
//    			g_LCD_page--;
//			}
//    	}
    }

    if(msg_id == CAN_ID_MDI_TEMP)
    {
    	uint8_t temperature = data[0];
    	set_cyclic_temperature(temperature);
    }
}
