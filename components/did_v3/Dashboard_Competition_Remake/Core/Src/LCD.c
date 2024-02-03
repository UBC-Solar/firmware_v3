#include "LCD.h"

#define TRUE 1
#define FALSE 0

/**
 * A Delay with a value of ~1ms
 * @Param counts: Number of milliseconds
 * Returns: nothing
 */
//TODO: Rewrite this function to use Timing Interrupts
void Delay(uint32_t counts)
{
	uint32_t i;
	uint16_t j;
	for (i = 0; i < counts; i++)
	{
		for(j = 0; j<16000; j++){}
	}

}

/**
 * A Delay with a value of ~1us
 * @Param counts: Number of microseconds
 * Returns: nothing
 */
//TODO: Rewrite this function to use Timing Interrupts
void MicrosecondDelay(uint32_t counts)
{
	uint32_t i;
	uint8_t j;

	for (i = 0; i < counts; i++)
	{
		for (j = 0; j<16; j++){}
	}
}

 /**
  * Moves cursor to the position on the screen determined by XY Coordinates
  * @Param x: x-coordinate
  * @Param y: y-coordinate
  */
void MoveCursor(uint8_t x, uint8_t y)
{
    /**
     * The screen's dimension is 40 x 30, and the location of the
     * cursor is described by the number in the CSRW register. This
     * number is the "offeset" from the top left corner in row-major
     * form
     *
     * Row major:
     * 0 1 2 3 4
     * 5 6 7 8 9
     *
     */
    uint16_t offset = 40*y + x;

    TransmitCommand(0x46);
    // The CSRW register is 2 bytes long, so the first parameter
    // is the first byte and the second parameter is the second
    // byte
    TransmitCommandParameter(offset&0xFF);
    TransmitCommandParameter(offset >> 8);
}

 /**
 * Purpose: Prints 1200 " " characters to clear the screen
 */
void ClearScreen()
{
    //Set cursor to start position
    MoveCursor(0, 0);

    //replace all characters with spaces
    TransmitCommand(0x42);
    for (uint16_t i = 0; i < 1200; i++)
    {
        TransmitCommandParameter(0x20);
    }
    MoveCursor(0, 0);
}

 /**
  * Saves each subcharacter to the SG RAM 1, in locations 0x80 to 0x8F
  * @Param bitmap: An array of the bitmap of that particular subcharacter
  * @Param offset: Some offset number
  * Returns: nothing
  */
void SaveCharacterToRAM(uint8_t* bitmap, uint8_t offset)
{
    TransmitCommand(0x46);
    TransmitCommandParameter(offset);
    TransmitCommandParameter(0x48);
    TransmitCommand(0x42);

    for (int i = 0; i < BYTEPERBITMAP; i++)
    {
        TransmitCommandParameter(bitmap[i]);
    }
}

/**
 * Writes custom bitmaps to the LCD screen RAM
 */
void CharacterBitMaps(void)
{
    uint8_t bitmap[16][BYTEPERBITMAP] = {{0x7E, 0x7E, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x7E, 0x7E},
                                    {0x7E, 0x7E, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x7E, 0x7E},
                                    {0x7E, 0x7E, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60},
                                    {0x7E, 0x7E, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06},
                                    {0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x7E, 0x7E},
                                    {0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x7E, 0x7E},
                                    {0x7E, 0x7E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
                                    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7E, 0x7E},
                                    {0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60},
                                    {0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06},
                                    {0x00, 0x06, 0x06, 0x0E, 0x0E, 0x1E, 0x36, 0x36, 0x36, 0x66, 0x66, 0x66, 0xE6, 0xE6, 0x86, 0x00},
                                    {0x00, 0x62, 0x62, 0x62, 0x66, 0x66, 0x66, 0x6C, 0x6C, 0x6C, 0x70, 0x70, 0x70, 0x60, 0x60, 0x00},
                                    {0x00, 0x60, 0x60, 0x70, 0x70, 0x78, 0x6C, 0x6C, 0x6C, 0x66, 0x66, 0x66, 0x67, 0x67, 0x61, 0x00},
                                    {0x00, 0x46, 0x46, 0x46, 0x66, 0x66, 0x66, 0x36, 0x36, 0x36, 0x0E, 0x0E, 0x0E, 0x06, 0x06, 0x00},
                                    {0xC0, 0xE0, 0x60, 0x30, 0x30, 0x30, 0x18, 0x18, 0x0C, 0x0C, 0x04, 0x06, 0x06, 0x06, 0x07, 0x03},
                                    {0x03, 0x07, 0x06, 0x06, 0x0C, 0x0C, 0x18, 0x18, 0x18, 0x30, 0x30, 0x60, 0x60, 0x60, 0xE0, 0xC0}};
    TransmitCommand(0x5C);
    TransmitCommandParameter(0x00);
    TransmitCommandParameter(0x40);
    TransmitCommand(0x4C);

    for (uint8_t i = 0; i < 16; i++)
    {
        SaveCharacterToRAM(bitmap[i], i*BYTEPERBITMAP);
    }
}

/**
 * Outputs a string of characters to the screen
 * @Param Str[]: A string, "array of characters", whatever
 * @Param starting_x: x-coordinate to write the string to
 * @Param starting_y: y-coordinate to write the string to
 * Returns: nothing
 */
void OutputString(char Str[], uint8_t starting_x, uint8_t starting_y)
{

    static uint8_t LOOKUPTABLE[LOOKUP_ROW][LOOKUP_COLUMN] = {            //Lookup Table for all Characters

                        {0x82,0x83,0x82,0x83}, {0x80,0x81,0x84,0x85}, {0x82,0x86,0x84,0x87}, {0x82,0x83,0x84,0x85}, //A B C D
                        {0x80,0x86,0x84,0x87}, {0x82,0x86,0x82,0x86}, {0x82,0x86,0x84,0x81}, {0x84,0x85,0x88,0x89}, //E F G H
                        {0x83,0x86,0x85,0x87}, {0x20,0x83,0x87,0x85}, {0x89,0x8F,0x89,0x8E}, {0x88,0x20,0x84,0x87}, //I J K L
                        {0x8C,0x8A,0x88,0x89}, {0x8C,0x89,0x88,0x8D}, {0x82,0x83,0x84,0x85}, {0x80,0x81,0x88,0x20}, //M N O P
                        {0x80,0x81,0x20,0x89}, {0x80,0x81,0x88,0x8E}, {0x80,0x86,0x87,0x81}, {0x83,0x86,0x89,0x20}, //Q R S T
                        {0x88,0x89,0x84,0x85}, {0x88,0x89,0x8E,0x8F}, {0x88,0x89,0x8B,0x8D}, {0x8E,0x8F,0x8F,0x8E}, //U V W X
                        {0x84,0x85,0x20,0x85}, {0x86,0x81,0x80,0x87},                                               //Y Z
                        {0x82,0x83,0x84,0x85}, {0x89,0x20,0x89,0x20},                                               //0 1
                        {0x86,0x81,0x80,0x87}, {0x86,0x81,0x87,0x85}, {0x84,0x85,0x20,0x89}, {0x80,0x86,0x87,0x81}, //2 3 4 5
                        {0x82,0x86,0x80,0x81}, {0x86,0x83,0x20,0x89}, {0x80,0x81,0x84,0x85}, {0x80,0x81,0x20,0x89}, //6 7 8 9
                        {0x20,0x20,0x20,0x20}, {0x20,0x20,0x2E,0x20}, {0x2E,0x20,0x2E,0x20}, {0x87,0x87,0x86,0x86}, //SPACE, ., :, -
                        };

    uint8_t temp;
    uint8_t x = starting_x;

    TransmitCommand(0x4C);                                  //Set the cursor direction to "Right"

    for (uint8_t c = 0; c < Str[c] != '\0'; c++)
    {
        MoveCursor(x, starting_y);
        temp = (uint8_t) Str[c];
        if (temp == 32)
        {
            temp = 36;                  //if character is " "
        }
        else if (temp == 58)
        {
            temp = 38;                  //if character is ":"
        }
        else if (temp == 46)
        {
            temp = 37;                  //if character is "."
        }
		else if (temp == 45)
		{
			temp = 39;	                //if character is "-"
		}
        else if (temp <= 57)
        {
            temp -= OFFSET_NUMERIC;     //if character is between "0" and "9"
        }
        else if (temp <= 90)
        {
            temp -= OFFSET_CAPITAL;     //if character is between "A" and "Z"
        }
        else if (temp <= 122)
        {
            temp -= OFFSET_LOWCASE;     //if character is between "a" and "z"
        }

        //TEMP IS NOW THE INDEX TO THE LOOKUP TABLE

        /***********************************
        ** CHARACTER ASSEMBLY AND OUTPUT
        *
        * 1 character is
        *  _ _
        * |A B|
        * |C D|
        *  - -
        * (4 subcharacters, in that order)     *
        ***********************************/

        TransmitCommand(0x42);                                  //print subcharacers A and B
        TransmitCommandParameter(LOOKUPTABLE[temp][0]);
        TransmitCommandParameter(LOOKUPTABLE[temp][1]);

        MoveCursor(x, starting_y + 1);

        TransmitCommand(0x42);                                  //print the subcharacters C and D
        TransmitCommandParameter(LOOKUPTABLE[temp][2]);
        TransmitCommandParameter(LOOKUPTABLE[temp][3]);

        x += 3;
    }
}

/**
 * Converts a number to a string which can be output on the screen
 * The output can take any value from '-999.9' to ' 999.9'
 * @Param num: The integer portion of the number to be displayed
 * @Param dec: The decimal portion of the number to be displayed
 * @param decimal_en: 1 if decimal should be included; 0 otherwise.
 * @Param x: x-coordinate to write the character
 * @Param y: y-coordinate to write the character
 * Returns: nothing
 */
void OutputPaddedInteger(int32_t num, uint8_t dec, uint8_t decimal_en, uint8_t x, uint8_t y)
{
	uint8_t i;
    char str[5] = {' ',' ',' ',' ','\0'};
	char decplace[2] = {' ', '\0'};

    if (num < 0)
    {
    	if (num > -99) str[1] = '-'; // make negative closer to number if only 2 digits
    	else str[0] = '-';
        num = -1 * num;
    }
	else if (num == 0)
	{
		str[3] = '0';
	}

	if (num > 999)
	{
		num = 999;
	}

    for (i = 3; num != 0; i--)
    {
        str[i] = "0123456789"[num%10];
        num = num/10;
    }

    OutputString(str, x, y);

    // If decimal_en = 0, skip decimal output
    if (!decimal_en) return;

    //Output 1 decimal place
    OutputString(".", x + 12, y);

    dec = dec % 10;

    decplace[0] = "0123456789"[dec];
    OutputString(decplace, x + 15, y);
}

/**
 * Periodically called to change the value of the Charge or Speed display bar
 * @Param num: The number to be represented
 * @Param max: The maximum value that the bar can display
 * @Param y: The y-coordinate of the bar
 * Returns: nothing
 */
void SetBar(uint8_t num, uint8_t max, uint8_t y)
{

    uint8_t blocks = 4*(num*10/max) + (num%10 >= 5)*2;

    MoveCursor(0, y);
    TransmitCommand(0x4C);
    TransmitCommand(0x42);

    for (uint8_t i = 0; i < blocks; i++)
    {
        TransmitCommandParameter(0x1A);
    }

    for (uint8_t i = blocks; i < 40; i++)
    {
        TransmitCommandParameter(0x20);
    }
}

/**
 * Displays all information in a single screen
 */
void DisplayScreen(void)
{
	//Clears the screen
    ClearScreen();

	//COLUMN 1

	//Battery Pack Current(BMS)
	OutputString("-000.0", BATTERY_CURRENT_XPOS, BATTERY_CURRENT_YPOS);
	OutputString("A", BATTERY_CURRENT_UNIT_XPOS, BATTERY_CURRENT_UNIT_YPOS);

	//Battery Pack Voltage(BMS)
	OutputString("-000.0", BATTERY_VOLTAGE_XPOS, BATTERY_VOLTAGE_YPOS);
	OutputString("V", BATTERY_VOLTAGE_UNIT_XPOS, BATTERY_VOLTAGE_UNIT_YPOS);

	//Motor Current(Motor Controller)
	OutputString("-000.0", MOTOR_CURRENT_XPOS, MOTOR_CURRENT_YPOS);
	OutputString("A", MOTOR_CURRENT_UNIT_XPOS, MOTOR_CURRENT_UNIT_YPOS);

	//MDU Temperature(Motor Controller)
	OutputString("-000.0", MOTOR_TEMP_XPOS, MOTOR_TEMP_YPOS);
	OutputString("C", MOTOR_TEMP_UNIT_XPOS, MOTOR_TEMP_UNIT_YPOS);

	//Vehicle Speed(Motor Controller)
	OutputString("-000.0", MOTOR_SPEED_XPOS, MOTOR_SPEED_YPOS);
	OutputString("KH", MOTOR_SPEED_UNIT_XPOS, MOTOR_SPEED_UNIT_YPOS);

	//COLUMN 2

	//Minimum cell voltage in the Battery(BMS)
	OutputString("-000.0", BATTERY_MINVOLT_XPOS, BATTERY_MINVOLT_YPOS);
	OutputString("V", BATTERY_MINVOLT_UNIT_XPOS, BATTERY_MINVOLT_UNIT_YPOS);

	//Maximum cell voltage in the Battery(BMS)
	OutputString("-000.0", BATTERY_MAXVOLT_XPOS, BATTERY_MAXVOLT_YPOS);
	OutputString("V", BATTERY_MAXVOLT_UNIT_XPOS, BATTERY_MAXVOLT_UNIT_YPOS);

	//Supplemental Battery Voltage(BMS)
	OutputString("-000.0", BATTERY_SUPPVOLT_XPOS, BATTERY_SUPPVOLT_YPOS);
	OutputString("V", BATTERY_SUPPVOLT_UNIT_XPOS, BATTERY_SUPPVOLT_UNIT_YPOS);

	//Maximum Temperature in the Battery(BMS)
	OutputString("-000.0", BATTERY_MAXTEMP_XPOS, BATTERY_MAXTEMP_YPOS);
	OutputString("C", BATTERY_MAXTEMP_UNIT_XPOS, BATTERY_MAXTEMP_UNIT_YPOS);

	//State of charge percentage
	OutputString("-000.0", BATTERY_CHARGE_XPOS, BATTERY_CHARGE_YPOS);

	//DEPRECATED
	//Maximum Temperature in the Array(Array)
	//OutputString("-000.0", ARRAY_MAXTEMP_XPOS, ARRAY_MAXTEMP_YPOS);
	//OutputString("C", ARRAY_MAXTEMP_UNIT_XPOS, ARRAY_MAXTEMP_UNIT_YPOS);

	//DEPRECATED
	//State of Charge Bar in percentage (BMS)
	//SetBar(100, 100, CHARGE_BAR_YPOS);

}

/**
 * Updates Screen Titles depending on current page number
 * @Param pageNum: The current page number
 */
void UpdateScreenTitles(uint8_t pageNum) {


	switch(pageNum)
	{
		case PAGE_0:
			/* Titles */
			OutputString("SOC", SOC_XPOS, SOC_YPOS);
			OutputString("CRUZE", CRUISE_XPOS, CRUISE_YPOS);
			OutputString("SPEED", SPEED_XPOS, SPEED_YPOS);
			OutputString("STATE", STATE_XPOS, STATE_YPOS);
            OutputString("TAR", TARGET_XPOS, TARGET_YPOS);
			/* Units */
			OutputString("%", SOC_UNIT_XPOS, SOC_UNIT_YPOS);
			OutputString("KM", CRUISE_UNIT_XPOS, CRUISE_UNIT_YPOS);
			OutputString("KM", SPEED_UNIT_XPOS, SPEED_UNIT_YPOS);
            OutputString("KM", TARGET_UNIT_XPOS, TARGET_UNIT_YPOS);
			break;
		case PAGE_1:
			/* Titles */
			OutputString("LV WARN", LV_WARN_XPOS, LV_WARN_YPOS);
			OutputString("HV WARN", HV_WARN_XPOS, HV_WARN_YPOS);
			OutputString("LT WARN", LT_WARN_XPOS, LT_WARN_YPOS);
			OutputString("HT WARN", HT_WARN_XPOS, HT_WARN_YPOS);
			break;
		case PAGE_2:
			/* Titles */
			OutputString("MTR C", MOTOR_CURRENT_XPOS, MOTOR_CURRENT_YPOS);
			OutputString("ARR C", ARRAY_CURRENT_XPOS, ARRAY_CURRENT_YPOS);
			OutputString("LV C", LV_CURRENT_XPOS, LV_CURRENT_YPOS);
			OutputString("PAK C", PACK_CURRENT_XPOS, PACK_CURRENT_YPOS);
			/* Units */
			OutputString("%", MOTOR_CURRENT_UNIT_XPOS, MOTOR_CURRENT_UNIT_YPOS);
			OutputString("A", ARRAY_CURRENT_UNIT_XPOS, ARRAY_CURRENT_UNIT_YPOS);
			OutputString("A", LV_CURRENT_UNIT_XPOS, LV_CURRENT_UNIT_YPOS);
			OutputString("%", PACK_CURRENT_UNIT_XPOS, PACK_CURRENT_UNIT_YPOS);
			break;
		case PAGE_3:
			/* Titles */
			OutputString("PK T", PACK_TEMP_XPOS, PACK_TEMP_YPOS);
			OutputString("PK V", PACK_VOLT_XPOS, PACK_VOLT_YPOS);

			/* Units */
			OutputString("C", PACK_TEMP_UNIT_XPOS, PACK_TEMP_UNIT_YPOS);
			OutputString("V", PACK_VOLT_UNIT_XPOS, PACK_VOLT_UNIT_XPOS);

			break;
		default:
			// Clear Screen
			ClearScreen();
			break;
	}
}

/**
 * Erases and revalues a single value field on a screen
 * @Param x: The x value of the parameter on the screen
 * @Param y: The y value of the parameter on the screen
 * @Param integerValue: The integer value of the parameter(Between -999 to 999)
 * @Param decValue: The decimal component of the parameter
 * @param decimal_en: 1 if decimal should be included; 0 otherwise.
 */
void UpdateScreenParameter(uint8_t x, uint8_t y, int32_t integerValue, uint8_t decValue, uint8_t decimal_en)
{
	//Clear a 6 x 2 rectangle to erase the previous number
	OutputString("     ", x, y);

	//Insert the new number
	OutputPaddedInteger(integerValue, decValue, decimal_en, x, y);
}

/**
 * Write a byte of data through C3 to C10(Data Buses)
 * @Param byte: 1 byte code
 * Returns: nothing
 */
void WriteByteToDataBus(uint8_t byte)
{
	GPIOC->BSRR = byte << 3;
	GPIOC->BRR = ((uint8_t) (~byte)) << 3;
}

/**
 * Writes a byte of command code through ports C3 to C10(Data Buses)
 * @Param command_code: 1 byte command code
 * returns: nothing
 */
void TransmitCommand(uint8_t command_code)
{
	GPIOC->BSRR = 0x1UL << 0;					//SET C0 to HIGH
	GPIOC->BRR = 0x1UL << 1;					//SET C1 to LOW
	WriteByteToDataBus(command_code);
	MicrosecondDelay(1);
	GPIOC->BSRR = 0x1UL << 1;					//SET C1 to HIGH
	MicrosecondDelay(1);
}

/**
 * Writes a byte of parameter code through ports C3 to C10(Data Buses)
 * @Param parameter_code: 1 byte parameter code
 * returns: nothing
 */
void TransmitCommandParameter(uint8_t parameter_code)
{
	GPIOC->BRR = 0x1UL << 0;					//SET C0 to LOW
	GPIOC->BRR = 0x1UL << 1;					//SET C1 to LOW
	WriteByteToDataBus(parameter_code);
	MicrosecondDelay(1);
	GPIOC->BSRR = 0x1UL << 1;					//SET C1 to HIGH
	MicrosecondDelay(1);
}

/**
 * Initialise GPIO Pins C0 to C12 for output
 */
void InitialiseLCDPins(void)
{

	//Setup System Clock C
    RCC->APB2ENR &= 0;
	RCC->APB2ENR |= 0x1UL << 4;

	//Setup Pins C5 - C12 as OUTPUT
	GPIOC->CRL &= 0;
	GPIOC->CRH &= 0;
	GPIOC->CRL |= 0x33333333UL; //Initialise C0 to C7
	GPIOC->CRH |= 0x33333UL; //Initialise C8 to C12

	//Set Pin initial values
	GPIOC->BSRR = 0x1UL << 0;	 // C0 HIGH
	GPIOC->BSRR = 0x1UL << 1;	 // C1 HIGH
	GPIOC->BSRR = 0x1UL << 2;	 // C2 HIGH
	GPIOC->BRR = 0x1UL << 11;	 // C11 LOW
	GPIOC->BSRR = 0x1UL << 12;   //C12 HIGH

}

/**
 * Initialise screen settings and character bit maps
 */
void ScreenSetup(void)
{
    // system setup (section 6-2-1 on raio datasheet)
    TransmitCommand(0x40);
        Delay(5);
        // 0 0 IV 1 W/S M2 M1 M0
        TransmitCommandParameter(0x34);
        // WF 0 0 0 0 FX FX FX
        TransmitCommandParameter(0x87);
        // 0 0 0 0 FY FY FY FY
        TransmitCommandParameter(0xF);
        // C/R (horizontal bytes per line)
        TransmitCommandParameter(39);
        // TC/R (horizontal bytes per line, incl blanking)
        TransmitCommandParameter(47);
        // L/F (lines per frame)
        TransmitCommandParameter(239);
        // APL (horizontal address range, LSB)
        TransmitCommandParameter(40);
        // APH (horizontal address range, MSB)
        TransmitCommandParameter(0);

    // scroll parameters
    TransmitCommand(0x44);
        TransmitCommandParameter(0); // SAD 1L
        TransmitCommandParameter(0); // SAD 1H
        TransmitCommandParameter(240); // SL1
        TransmitCommandParameter(176); // SAD 2L
        TransmitCommandParameter(4); // SAD 2H
        TransmitCommandParameter(240); // SL2
        TransmitCommandParameter(0); // SAD 3L
        TransmitCommandParameter(0); // SAD 3H
        TransmitCommandParameter(0); // SAD 4
        TransmitCommandParameter(0); // SAD 4

	TransmitCommand(0x5B);
		TransmitCommandParameter(0);

    // set horizontal scroll position
    TransmitCommand(0x5A);
        TransmitCommandParameter(0);

    // set display overlay format
    TransmitCommand(0x5B);
        TransmitCommandParameter(0);

    // turn off the display and configure cursor
    TransmitCommand(0x58);
        TransmitCommandParameter(0x57);

    // set cursor size & type
    TransmitCommand(0x5D);
        TransmitCommandParameter(4);
        TransmitCommandParameter(0x84);

    // set cursor direction
    TransmitCommand(0x4C);

    // turn on the display and configure cursor
    TransmitCommand(0x59);
        TransmitCommandParameter(0x7);

    CharacterBitMaps();
    ClearScreen();
//	DisplayScreen();

}
