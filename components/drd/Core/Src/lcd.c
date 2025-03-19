#include "lcd.h"
#include <stdio.h>

/*--------------------------------------------------------------------------
  Internal Types & Variables
--------------------------------------------------------------------------*/

/* A simple bounding box structure used internally to track text regions */
typedef struct {
    uint8_t x1;
    uint8_t y1;
    uint8_t x2;
    uint8_t y2;
} bounding_box_t;

/* Function Declarations */
static void lcd_pixel(uint8_t x, uint8_t y, uint8_t colour);
static void lcd_clear_bounding_box(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);
static void lcd_refresh();
static void draw_rectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t color);
static bounding_box_t draw_text(char *string, unsigned char x, unsigned char y, const unsigned char *font, unsigned char spacing);
static bounding_box_t draw_char(unsigned char c, unsigned char x, unsigned char y, const unsigned char *font);

/* Internal buffer for pixel operations (assumes a 128x64 display) */
static uint8_t lcd_buffer[(128 * 64) / 8];

/* Internal SPI handle for LCD communication */
static SPI_HandleTypeDef* sg_spi_handle = NULL;

/* Static variables to store old bounding boxes for updating text fields */
static bounding_box_t old_bb_speed          = {0, 0, 0, 0};
static bounding_box_t old_bb_speed_units    = {0, 0, 0, 0};
static bounding_box_t old_bb_drive_state    = {0, 0, 0, 0};
static bounding_box_t old_bb_drive_mode    = {0, 0, 0, 0};
static bounding_box_t old_bb_soc            = {0, 0, 0, 0};

static uint8_t lcd_flipped = 0;

#ifdef ST7565_DIRTY_PAGES
static uint8_t lcd_dirty_pages;
#endif

/*--------------------------------------------------------------------------
  Internal Helper Functions
--------------------------------------------------------------------------*/

/**
 * @brief Sets or clears a single pixel in the internal display buffer.
 * 
 * @param x The x coordinate (1-based).
 * @param y The y coordinate (1-based).
 * @param color 1 to set the pixel, 0 to clear it.
 */
static void lcd_pixel(uint8_t x, uint8_t y, uint8_t colour) {

    if (x > SCREEN_WIDTH || y > SCREEN_HEIGHT) return;

    // Real screen coordinates are 0-63, not 1-64.
    x -= 1;
    y -= 1;

    unsigned short array_pos = x + ((y / 8) * 128);

    #ifdef ST7565_DIRTY_PAGES
        lcd_dirty_pages |= 1 << (array_pos / 128);
    #endif

    if (colour) {
        lcd_buffer[array_pos] |= 1 << (y % 8);
    } else {
        lcd_buffer[array_pos] &= 0xFF ^ 1 << (y % 8);
    }
}

/**
 * @brief Clears a rectangular area in the internal display buffer.
 * 
 * @param x1 Left coordinate (1-based).
 * @param y1 Top coordinate (1-based).
 * @param x2 Right coordinate (1-based).
 * @param y2 Bottom coordinate (1-based).
 */
static void lcd_clear_bounding_box(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2) {
    if (x1 >= SCREEN_WIDTH || x2 >= SCREEN_WIDTH || y1 >= SCREEN_HEIGHT || y2 >= SCREEN_HEIGHT || x1 > x2 || y1 > y2)
        return;

    for (unsigned char y = y1; y <= y2; y++) {
        for (unsigned char x = x1; x <= x2; x++) {
            unsigned short array_pos = x + ((y / 8) * 128);
            lcd_buffer[array_pos] = 0;
        }
    }
}

/**
 * @brief Refreshes the LCD display by calling the ST7565 display update.
 */
static void lcd_refresh() {
    for (int y = 0; y < 8; y++) {

    #ifdef ST7565_DIRTY_PAGES
            // Only copy this page if it is marked as "dirty"
            if (!(lcd_dirty_pages & (1 << y))) continue;
    #endif

        LCD_write_command(CMD_SET_PAGE | y);

        // Reset column to the left side.  The internal memory of the
        // screen is 132*64, we need to account for this if the display
        // is flipped.
        //
        // Some screens seem to map the internal memory to the screen
        // pixels differently, the ST7565_REVERSE define allows this to
        // be controlled if necessary.
#ifdef ST7565_REVERSE
        if (!lcd_flipped) {
#else
        if (lcd_flipped) {
#endif
            LCD_write_command(CMD_COLUMN_LOWER | 4);
        } else {
            LCD_write_command(CMD_COLUMN_LOWER);
        }
        LCD_write_command(CMD_COLUMN_UPPER);

        for (int x = 0; x < 128; x++) {
            LCD_write_data(lcd_buffer[y * 128 + x]);
        }
    }

    #ifdef ST7565_DIRTY_PAGES
        // All pages have now been updated, reset the indicator.
        lcd_dirty_pages = 0;
    #endif
}

/**
 * @brief Draws a rectangle outline using the internal pixel function.
 * 
 * @param x1 Left coordinate (1-based).
 * @param y1 Top coordinate (1-based).
 * @param x2 Right coordinate (1-based).
 * @param y2 Bottom coordinate (1-based).
 * @param color 1 to draw pixel.
 */
static void draw_rectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t color)
{
    for(uint8_t x = x1; x <= x2; x++) {
        lcd_pixel(x, y1, color);
        lcd_pixel(x, y2, color);
    }
    for(uint8_t y = y1; y <= y2; y++) {
        lcd_pixel(x1, y, color);
        lcd_pixel(x2, y, color);
    }
}

/**
 * @brief Draws a text string using an external graphics library.
 * 
 * @param str The null-terminated string to draw.
 * @param x Starting x coordinate.
 * @param y Starting y coordinate.
 * @param font Pointer to the font to use.
 * @param spacing Spacing between characters.
 * @return bounding_box_t The bounding box of the drawn text.
 */
static bounding_box_t draw_text(char *string, unsigned char x, unsigned char y, const unsigned char *font, unsigned char spacing) {
	bounding_box_t ret;
	bounding_box_t tmp;

	ret.x1 = x;
	ret.y1 = y;

	spacing += 1;

	// BUG: As we move right between chars we don't actually wipe the space
	while (*string != 0) {
		tmp = draw_char(*string++, x, y, font);

		// Leave a single space between characters
		x = tmp.x2 + spacing;
	}

	ret.x2 = tmp.x2;
	ret.y2 = tmp.y2;

	return ret;
}

/**
 * @brief Draws a single character using an external graphics library.
 * 
 * @param c The character to draw.
 * @param x Starting x coordinate.
 * @param y Starting y coordinate.
 * @param font Pointer to the font to use.
 * @return bounding_box_t The bounding box of the drawn character.
 */
static bounding_box_t draw_char(unsigned char c, unsigned char x, unsigned char y, const unsigned char *font) {
	unsigned short pos;
	uint8_t width;
	bounding_box_t ret;

	ret.x1 = x;
	ret.y1 = y;
	ret.x2 = x;
	ret.y2 = y;

	// Read first byte, should be 0x01 for proportional
	if (font[FONT_HEADER_TYPE] != FONT_TYPE_PROPORTIONAL) return ret;

	// Check second byte, should be 0x02 for "vertical ceiling"
	if (font[FONT_HEADER_ORIENTATION] != FONT_ORIENTATION_VERTICAL_CEILING) return ret;

	// Check that font start + number of bitmaps contains c
	if (!(c >= font[FONT_HEADER_START] && c <= font[FONT_HEADER_START] + font[FONT_HEADER_LETTERS])) return ret;

	// Adjust for start position of font vs. the char passed
	c -= font[FONT_HEADER_START];

	// Work out where in the array the character is
	pos = font[c * FONT_HEADER_START + 5];
	pos <<= 8;
	pos |= font[c * FONT_HEADER_START + 6];

	// Read first byte from this position, this gives letter width
	width = font[pos];

	// Draw left to right
	uint8_t i;
	for (i = 0; i < width; i++) {

		// Draw top to bottom
		for (uint8_t j = 0; j < font[FONT_HEADER_HEIGHT]; j++) {
			// Increment one data byte every 8 bits, or
			// at the start of a new column  HiTech optimizes
			// the modulo, so no need to try and avoid it.
			if (j % 8 == 0) pos++;

			if (font[pos] & 1 << (j % 8)) {
				lcd_pixel(x + i, y + j, 1);
			} else {
				lcd_pixel(x + i, y + j, 0);
			}
		}
	}

	ret.x2 = ret.x1 + width - 1;
	ret.y2 = ret.y1 + font[FONT_HEADER_HEIGHT];

	return ret;
}


/*--------------------------------------------------------------------------
  Public Function Implementations
--------------------------------------------------------------------------*/

/**
 * @brief Displays the speed on the LCD.
 * 
 * @param speed The speed value to display.
 * @param units The speed units (LCD_SPEED_UNITS_MPH or LCD_SPEED_UNITS_KPH).
 */
void LCD_display_speed(uint32_t speed, int units)
{
    char speed_str[12];
    /* Clear the previous speed and unit areas */
    lcd_clear_bounding_box(SPEED_X - SPEED_SPACING, SPEED_Y, old_bb_speed.x2 + SPEED_SPACING, BOTTOM_RIGHT_Y);
    lcd_clear_bounding_box(old_bb_speed.x2 - SPEED_SPACING, SPEED_Y, old_bb_speed_units.x2 + SPEED_UNITS_SPACING, SPEED_Y + 11);
    
    if (speed < 10) { // Single digit speed
        sprintf(speed_str, "%01lu", (unsigned long)speed);  
        old_bb_speed = draw_text(speed_str, SPEED_X + 14, SPEED_Y, SPEED_FONT, SPEED_SPACING);
    } else {
        sprintf(speed_str, "%02lu", (unsigned long)speed);  
        old_bb_speed = draw_text(speed_str, SPEED_X, SPEED_Y, SPEED_FONT, SPEED_SPACING);
    }
    
    /* Draw the speed units */
    switch (units) {
        case KPH:
            old_bb_speed_units = draw_text("kph", SPEED_X + 2 * WIDEST_NUM_LEN_VERDANA32, SPEED_Y, SPEED_UNITS_FONT, SPEED_UNITS_SPACING);
            break;
            case MPH:
            old_bb_speed_units = draw_text("mph", SPEED_X + 2 * WIDEST_NUM_LEN_VERDANA32, SPEED_Y, SPEED_UNITS_FONT, SPEED_UNITS_SPACING);
            break;
        default:
            old_bb_speed_units = draw_text("xxx", SPEED_X + 2 * WIDEST_NUM_LEN_VERDANA32, SPEED_Y, SPEED_UNITS_FONT, SPEED_UNITS_SPACING);
            break;
    }
    lcd_refresh();
}

/**
 * @brief Displays the drive state on the LCD.
 * 
 * @param state The drive state (e.g., FORWARD_STATE, PARK_STATE, REVERSE_STATE).
 */
void LCD_display_drive_state(int state)
{
    char state_str[2] = {ERROR_SYMBOL, '\0'};  // Default to error symbol.
    lcd_clear_bounding_box(STATE_X, STATE_Y, old_bb_drive_state.x2, BOTTOM_RIGHT_Y);
    
    switch (state) {
        case FORWARD_STATE:
            state_str[STATE_IDX] = FORWARD_SYMBOL;
            break;
        case PARK_STATE:
            state_str[STATE_IDX] = PARK_SYMBOL; 
            break;
        case REVERSE_STATE:
            state_str[STATE_IDX] = REVERSE_SYMBOL;
            break;
        default:
            break;
    }
    old_bb_drive_state = draw_text(state_str, STATE_X, STATE_Y, STATE_FONT, STATE_SPACING);
    lcd_refresh();
}

/**
 * @brief Displays the state of charge (SOC) on the LCD.
 * 
 * @param soc The state of charge (in percent).
 */
void LCD_display_SOC(uint32_t soc)
{
    char soc_str[12];
    bounding_box_t bb;
    lcd_clear_bounding_box(SOC_X - SOC_SPACING, SOC_Y, BOTTOM_RIGHT_X, BOTTOM_RIGHT_Y);
    
    if (soc < 10) {
        sprintf(soc_str, "%01lu", (unsigned long)soc);
        bb = draw_text(soc_str, SOC_X + 10, SOC_Y, SOC_FONT, SOC_SPACING);
    } else {
        sprintf(soc_str, "%02lu", (unsigned long)soc);
        bb = draw_text(soc_str, SOC_X, SOC_Y, SOC_FONT, SOC_SPACING);
    }

    UNUSED(bb);     // remove warnigin

    old_bb_soc = draw_char(SOC_UNITS, SOC_X + 2 * WIDEST_NUM_LEN_VERDANA16 + 2, SOC_Y, SOC_UNITS_FONT);
    lcd_refresh();
}

/**
 * @brief Displays a battery power bar based on pack current and voltage.
 * 
 * @param pack_current The battery pack current.
 * @param pack_voltage The battery pack voltage.
 */
void LCD_display_power_bar(float pack_current, float pack_voltage)
{
    float power = pack_current * pack_voltage;
    int fill_pixels = 0;

    /* Clear the drawing area (including extra space for the center line) */
    lcd_clear_bounding_box(BAR_LEFT, BAR_TOP, BAR_RIGHT, BAR_BOTTOM + 3);

    /* Draw the outline of the power bar */
    draw_rectangle(BAR_LEFT, BAR_TOP, BAR_RIGHT, BAR_BOTTOM, 1);

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
    } else if (power < 0) {
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


/**
 * @brief Displays an E for ECO mode and P for POWER mode
 * 
 * @param drive_mode The drive mode
 */
void LCD_display_drive_mode(uint8_t drive_mode)
{
    char drive_mode_c = ERROR_SYMBOL;   // Default to error symbol.
    lcd_clear_bounding_box(0, old_bb_drive_mode.y1 + 7, old_bb_drive_mode.x2, old_bb_drive_mode.y2);
    
    switch (drive_mode) {
        case DRIVE_MODE_ECO:
            drive_mode_c = ECO_SYMBOL;
            old_bb_drive_mode = draw_char(drive_mode_c, ECO_MODE_X, ECO_MODE_Y, ECO_MODE_FONT);
            break;
        case DRIVE_MODE_POWER:
            drive_mode_c = POWER_SYMBOL; 
            old_bb_drive_mode = draw_char(drive_mode_c, POWER_MODE_X, POWER_MODE_Y, POWER_MODE_FONT);
            break;
        default:
            break;
    }

    // With LCD Refresh the topbar gets cut into. This is because lighting bolt has unecesary white space :(.
}

/**
 * @brief Sends a command to the LCD via SPI.
 * 
 * @param cmd The command byte to send.
 */
void LCD_write_command(uint8_t cmd)
{
    /* Set A0 low for command */
    HAL_GPIO_WritePin(DISPLAY_A0_GPIO_Port, DISPLAY_A0_Pin, GPIO_PIN_RESET);
    
    uint8_t cmd_arr[1] = {cmd};
    HAL_SPI_Transmit(sg_spi_handle, cmd_arr, 1, 10);
}

/**
 * @brief Sends data to the LCD via SPI so that the specified pixels turn black (or white).
 * 
 * @param data The data byte to send.
 */
void LCD_write_data(uint8_t data)
{
    /* Set A0 high for data */
    HAL_GPIO_WritePin(DISPLAY_A0_GPIO_Port, DISPLAY_A0_Pin, GPIO_PIN_SET);
    
    uint8_t data_arr[1] = {data};
    HAL_SPI_Transmit(sg_spi_handle, data_arr, 1, 10);
}

/**
 * @brief Initializes the LCD based on the ST7565 library and prints the field names.
 * 
 * @param hspi Pointer to the SPI handle.
 */
void LCD_init(SPI_HandleTypeDef* hspi)
{
    HAL_GPIO_WritePin(DISPLAY_RESET_GPIO_Port, DISPLAY_RESET_Pin, GPIO_PIN_RESET); 
    HAL_Delay(30);
    HAL_GPIO_WritePin(DISPLAY_RESET_GPIO_Port, DISPLAY_RESET_Pin, GPIO_PIN_SET); 
    HAL_Delay(30);

    sg_spi_handle = hspi;

    LCD_write_command(CMD_SET_ADC_NORMAL);          
    LCD_write_command(CMD_DISPLAY_OFF);
    LCD_write_command(CMD_SET_COM_NORMAL + 8);        // This makes the drawing flipped
    LCD_write_command(CMD_SET_BIAS_9);
    LCD_write_command(CMD_SET_POWER_CONTROL | 0x7);
    LCD_write_command(CMD_SET_RESISTOR_RATIO | 0x6);    // set lcd operating voltage (regulator resistor, ref voltage resistor)
    LCD_write_command(CMD_SET_VOLUME_FIRST);
    LCD_write_command(CMD_SET_CONTRAST - 5);
    LCD_write_command(CMD_DISPLAY_START);
    LCD_write_command(CMD_DISPLAY_ON);
    LCD_write_command(CMD_SET_ALLPTS_NORMAL);
}
