# USAGE
This README explains how to use the LCD library. This is intended for use on the DRD.

- See the **Libraries** section on the [wiki](https://wiki.ubcsolar.com/tutorials/stm32cubeide) for how to import a library to a project.

## Useful Links
* [DRD Altium Link](https://ubc-solar.365.altium.com/designs/A1D09E3F-0EB6-42A4-A897-C945D50A3C55?variant=[No+Variations]&activeDocumentId=E_PAS_DRD.SchDoc(1)&activeView=SCH&location=[1,95.68,26.62,35.19]#design)
* [LCD Basic Arduino Functionality GitHub Link](https://github.com/NewhavenDisplay/NHD-C12864A1Z_Example/blob/main/NHD-C12864A1Z/NHD-C12864A1Z.ino#L197)
* [LCD Printing Library GitHub Link](https://github.com/mberntsen/STM32-Libraries/blob/master/ST7565/src/ST7565.c)
* [LCD Screen ](https://newhavendisplay.com/content/specs/NHD-C12864A1Z-FSW-FBW-HTT.pdf)
* [LCD ST7565P Controller Datasheet](https://support.newhavendisplay.com/hc/en-us/article_attachments/4414878945687)

## Additions
* Adds the weight of extra logic to program the LCD screen
* Adds a **1024 byte buffer** to quickly handle LCD operations!

## Initialization
Call the `LCD_init()` function once with a **pointer** to your SPI handle. This will likely occur in your `main.c` code. To initialize the SPI peripheral correctly set these fields as follows:
![alt text](image.png)
![alt text](image-1.png)

Possible differences
* Since the DRD does not have a 32kHz crystal oscillator you will likely have a different baudrate. The default baudrate **should** work, however, when debugging consider this problem.

## How to Add a Field
There are 2 parts to adding a field onto the LCD screen.
1. Adding the fields heading/name.
2. Adding the fields data.

### Set the Field Name
To do this, go into `lcd.h` first and find the `/** START DISPLAY FIELDS */` section. In here, add a field by defining the following. 

**Notes** 
* Replace `x` with whatever the next field number is. 
* Line numbers are 0-indexed. Meaning the line at the top of the screen is line 0 and correspondingly field 0.
* The `DATA_POS` is calculated as the character width times the number of characters of the heading/name (not including the null terminating character). Example `"Speed: "` is exactly 7 characters including the space. We include the space if we want tthe data to come after a space. 

**Explanation of what to Define**
* `FIELD_x`: Specifies the heading/name of the field. Example: `#define FIELD_0       "Speed: "`
* `FIELD_x_DATA_POS`: Specifies where in the line the data will start printing. Example: `#define FIELD_0_DATA_POS   (7 * CHAR_WIDTH)`
* `FIELD_x_LINE`: Specifies the line number . Example: `#define FIELD_0_LINE  0`

### Set the Field Data
Now that these defines are made we need to declare the function prototype for the field we want to display. To do this, name the function `LCD_display_data_field_x` where x is the field number. For the argument of the function, you need to decide what data type do you want to display. For example things that need a decimal point could be of the float type. Things that are integers will be int. You can choose whatever you want as long as you can convert it to a string (see in the next step). Example: `void LCD_display_data_field_0(uint32_t speed);`

Navigate to `lcd.c` now and locate `void print_fields()`. In this function, you need to perform an `LCD_print` for the field you want to display.

Go down a bit and define the `void LCD_display_data_field_x` function you made the declaration for in `lcd.h`. This function does 2 things.
1. Takes the input number and converts it to a string.
2. Takes the string and performs an `LCD_print` to display it.

For a simple defintion, use `sprintf` to generate your string. 

Example
```c
void LCD_display_data_field_0(uint32_t speed)
{
    // convert speed to string.
    char speed_str[12];  // Buffer to hold the converted string (enough for 10 digits + null terminator)
    sprintf(speed_str, "%lu", (unsigned long)speed);  // Convert uint32_t to string

    LCD_print(FIELD_0_DATA_POS, FIELD_0_LINE, speed_str);     // Field 1
}
```

Now you are done! You can use the `LCD_display_data_field_x` function you made wherever in your code to update the LCD screen data!

## Library Functionality
There are extra functions we can add to the library, however, they are removed to make the file simple to read. Please see this [link](https://github.com/mberntsen/STM32-Libraries/blob/master/ST7565/src/ST7565.c) for the original library code.
