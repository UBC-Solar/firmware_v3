#include "lcd.h"

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


/* LCD Drawing Functions */

/**
 * @brief Sets or clears a single pixel in the internal display buffer.
 *
 * @param x The x coordinate (1-based).
 * @param y The y coordinate (1-based).
 * @param color 1 to set the pixel, 0 to clear it.
 */
void lcd_pixel(uint8_t x, uint8_t y, uint8_t colour);

/**
 * @brief Clears a rectangular area in the internal display buffer.
 *
 * @param x1 Left coordinate (1-based).
 * @param y1 Top coordinate (1-based).
 * @param x2 Right coordinate (1-based).
 * @param y2 Bottom coordinate (1-based).
 */
void lcd_clear_bounding_box(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);

/**
 * @brief Refreshes the LCD display by calling the ST7565 display update.
 */
void lcd_refresh();

/**
 * @brief Draws a rectangle outline using the internal pixel function.
 *
 * @param x1 Left coordinate (1-based).
 * @param y1 Top coordinate (1-based).
 * @param x2 Right coordinate (1-based).
 * @param y2 Bottom coordinate (1-based).
 * @param color 1 to draw pixel.
 */
void draw_rectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t color);

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
bounding_box_t draw_text(char *string, unsigned char x, unsigned char y, const unsigned char *font, unsigned char spacing);

/**
 * @brief Draws a single character using an external graphics library.
 *
 * @param c The character to draw.
 * @param x Starting x coordinate.
 * @param y Starting y coordinate.
 * @param font Pointer to the font to use.
 * @return bounding_box_t The bounding box of the drawn character.
 */
bounding_box_t draw_char(unsigned char c, unsigned char x, unsigned char y, const unsigned char *font);


/* LCD Initializing Functions */

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
