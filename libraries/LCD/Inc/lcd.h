#ifndef __LCD_H__
#define __LCD_H__

#include "ST7565.h"
#include <stdint.h>


/** DISPLAY SETTINGS */
#define CHAR_WIDTH          6
#define FIELD_START         0


/** START DISPLAY FIELDS */

/** FIELD 0 */
#define FIELD_0       "Speed: "
#define FIELD_0_DATA_POS   (7 * CHAR_WIDTH)
#define FIELD_0_LINE  0
void LCD_display_data_field_0(uint32_t speed);

/** FIELD 1 */
#define FIELD_1       "Other Data: "
#define FIELD_1_DATA_POS   (12 * CHAR_WIDTH)
#define FIELD_1_LINE  1
void LCD_display_data_field_1(float throttle_percent);

/** FIELD 2 */

/** END DISPLAY FIELDS */

void LCD_init(SPI_HandleTypeDef* hspi);
void LCD_print(uint8_t x, uint8_t line, char *c);

// Functions for ST7565 library to use
void LCD_write_command(uint8_t cmd);
void LCD_write_data(uint8_t data);


#endif /*__ LCD_H__ */
