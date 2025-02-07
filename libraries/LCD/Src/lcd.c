/**
 * References the library: https://github.com/mberntsen/STM32-Libraries
 */

#include "spi.h"
#include "lcd.h"
#include "main.h"
#include <stdio.h>

/** START FIELD DISPLAYING FUNCTION DEFINITIONS */

/**
 * @brief Internal function in the init to print the fields 
 */
void print_fields()
{
    LCD_print(FIELD_START, FIELD_0_LINE, FIELD_0);     // Field 0
    LCD_print(FIELD_START, FIELD_1_LINE, FIELD_1);     // Field 1
}

/** FIELD 0 */
void LCD_display_data_field_0(uint32_t speed)
{
    // convert speed to string.
    char speed_str[12];  // Buffer to hold the converted string (enough for 10 digits + null terminator)
    sprintf(speed_str, "%lu", (unsigned long)speed);  // Convert uint32_t to string

    LCD_print(FIELD_0_DATA_POS, FIELD_0_LINE, speed_str);     // Field 1
}


/** FIELD 0 */
void LCD_display_data_field_1(float throttle_percent)
{
    char throttle_str[8];  // Buffer to hold the formatted string (e.g., "100.0%" is 6 chars + null terminator)
    sprintf(throttle_str, "%.1f%%", throttle_percent);  // Format as a percentage with 1 decimal place

    LCD_print(FIELD_1_DATA_POS, FIELD_1_LINE, throttle_str);  // Display throttle percentage
}

/** END FIELD DISPLAYING FUNCTION DEFINITIONS */

/** BASE LOGIC FOR LCD USAGE */

static SPI_HandleTypeDef* sg_spi_handle;

/**
 * @brief Sends specificed command to LCD screen via SPI
 * 
 * @param cmd Command to send. See Page 8 on https://newhavendisplay.com/content/specs/NHD-C12864A1Z-FSW-FBW-HTT.pdf
 */
void LCD_write_command(uint8_t cmd) {
    // HAL_GPIO_WritePin(CS, CS_Pin, GPIO_PIN_RESET); // CS = 0 to select
    HAL_GPIO_WritePin(A0_GPIO_Port, A0_Pin, GPIO_PIN_RESET); // A0 = 0 for command

    // TODO: Send `cmd` over SPI (not implemented yet)
    uint8_t cmd_arr[1] = {cmd};
    HAL_SPI_Transmit(sg_spi_handle, &cmd_arr[0], 1, 10);

    // HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET); // CS = 0 to select
}

/**
 * @brief Sends data to LCD screen via SPI so that the specified pixels turn black (or white)
 * 
 * @param cmd Command to send. See this update on how writing data works: 
 *            https://ubcsolar26.monday.com/boards/7524367653/pulses/7915667617/posts/3834350358
 */
void LCD_write_data(uint8_t data) {
    // HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET); // CS = 0 to select
    HAL_GPIO_WritePin(A0_GPIO_Port, A0_Pin, GPIO_PIN_SET); // A0 = A0 = 1 for data

    uint8_t data_arr[1] = {data};
    HAL_SPI_Transmit(sg_spi_handle, &data_arr[0], 1, 10);
    // SPI_Write(data); // Send command manually

    // HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET); // CS = 0 to select
}

/**
 * @brief Performs an LCD init based on 
 *        https://github.com/NewhavenDisplay/NHD-C12864A1Z_Example/blob/main/NHD-C12864A1Z/NHD-C12864A1Z.ino#L197
 * 
 *        Also prints all the field names to get ready to display their data.
 */
void LCD_init(SPI_HandleTypeDef* hspi)
{
    sg_spi_handle = hspi;
    ST7565_begin();
    print_fields();
}

/**
 * @brief Sends data to LCD screen via SPI so that the specified pixels turn black (or white)
 * 
 * @param x The position along the line to start printing. Typically a multiple of CHAR_WIDTH
 * @param line The line to do the printing on
 * @param c Pointer to character array that you want to print
 */
void LCD_print(uint8_t x, uint8_t line, char *c) {
    ST7565_drawstring(x, line, c);
    ST7565_display();
}

/**
 * @brief Clears the LCD display
 */
void LCD_clear()
{
    ST7565_clear_display();
}

