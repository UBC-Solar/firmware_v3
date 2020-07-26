#include "ltc6813_btm_temp.h"
#include "stm32f3xx_hal.h"
#include <math.h>

#define GPIO_9_TOGGLE 0b00001000 // For bitwise operations with CFGRB byte 0

// Private function prototypes:
BTM_Status_t readThermistorVoltage(
    uint16_t GPIO1_voltage[BTM_NUM_DEVICES],
    uint16_t GPIO2_voltage[BTM_NUM_DEVICES],
    uint16_t GPIO3_voltage[BTM_NUM_DEVICES],
    uint16_t GPIO4_voltage[BTM_NUM_DEVICES],
    uint16_t GPIO5_voltage[BTM_NUM_DEVICES],
    uint16_t GPIO6_voltage[BTM_NUM_DEVICES],
    uint16_t GPIO7_voltage[BTM_NUM_DEVICES],
    uint16_t GPIO8_voltage[BTM_NUM_DEVICES],
    uint16_t REF2_voltage[BTM_NUM_DEVICES]
);

void volts2temp(uint16_t ADC[], uint16_t REF2[], float temp_celsius[]);


/*
Function name: BTM_TEMP_measureState
Purpose: Algorithm for reading thermistors across multiple muxes
input:
Pointer to predefined pack data structure.
Internal functions pull readings from hardware registers
Output:
Voltages of thermistors stored in the given pack data structure
ie. pack.stack[].module[].temperature, as well as
actual temperature in degrees celsius
ie. pack.stack[].GPIO[].channel[].temperature_C.
Returns:
BTM_Status_t Status from communication functions.
Note: nothing new written to pack if return value != BTM_OK
Internal functions:
readThermistorVoltage()
volts2temp
*/
BTM_Status_t BTM_TEMP_measureState(BTM_PackData_t* pack)
{
    uint16_t GPIO1_voltage[MUX_CHANNELS][BTM_NUM_DEVICES] = {0}; // add in header MUX_CHANNELS = 2
    uint16_t GPIO2_voltage[MUX_CHANNELS][BTM_NUM_DEVICES] = {0};
    uint16_t GPIO3_voltage[MUX_CHANNELS][BTM_NUM_DEVICES] = {0};
    uint16_t GPIO4_voltage[MUX_CHANNELS][BTM_NUM_DEVICES] = {0};
    uint16_t GPIO5_voltage[MUX_CHANNELS][BTM_NUM_DEVICES] = {0};
    uint16_t GPIO6_voltage[MUX_CHANNELS][BTM_NUM_DEVICES] = {0};
    uint16_t GPIO7_voltage[MUX_CHANNELS][BTM_NUM_DEVICES] = {0};
    uint16_t GPIO8_voltage[MUX_CHANNELS][BTM_NUM_DEVICES] = {0};
    uint16_t REF2_voltage[MUX_CHANNELS][BTM_NUM_DEVICES] = {0};
    float temp_celsius[NUM_GPIOS][MUX_CHANNELS][BTM_NUM_DEVICES]; // add in header NUM_GPIOS = 8
    BTM_Status_t status = {BTM_OK, 0};
    uint8_t cfgrb_to_write[BTM_NUM_DEVICES][BTM_REG_GROUP_SIZE];


    // Refer to the LTC6813 datasheet pages 60 and 65 for format and content of config_val
    for(int ic_num = 0; ic_num < BTM_NUM_DEVICES; ic_num++) {
        for(int reg_num = 0; reg_num < BTM_REG_GROUP_SIZE; reg_num++) {
            cfgrb_to_write[ic_num][reg_num] =  pack->stack[ic_num].cfgrb[reg_num];
            if (reg_num == 0)
            {
                cfgrb_to_write[ic_num][reg_num] &= ~GPIO_9_TOGGLE; // GPIO 9 = 0
                // update the stored configuration register
                pack->stack[ic_num].cfgrb[reg_num] = cfgrb_to_write[ic_num][reg_num];
            }
        }
    }

    BTM_writeRegisterGroup(CMD_WRCFGB, cfgrb_to_write);
    // CMD_WRCFGB has to be added to the header file: CMD_WRCFGB = 0b000_0010_0100// CMD_WRCFGB has to be added to the header file: CMD_WRCFGB = 0b000_0010_0100

    // perform readings for channel 0
    status = readThermistorVoltage
    (
        GPIO1_voltage[0],
        GPIO2_voltage[0],
        GPIO3_voltage[0],
        GPIO4_voltage[0],
        GPIO5_voltage[0],
        GPIO6_voltage[0],
        GPIO7_voltage[0],
        GPIO8_voltage[0],
        REF2_voltage[0]
    );

    if (status.error != BTM_OK)
        return status;

    // This can't be rewritten as a for loop with separate arrays for each GPIO voltage
    volts2temp(GPIO1_voltage[0], REF2_voltage[0], temp_celsius[0][0]);
    volts2temp(GPIO2_voltage[0], REF2_voltage[0], temp_celsius[1][0]);
    volts2temp(GPIO3_voltage[0], REF2_voltage[0], temp_celsius[2][0]);
    volts2temp(GPIO4_voltage[0], REF2_voltage[0], temp_celsius[3][0]);
    volts2temp(GPIO5_voltage[0], REF2_voltage[0], temp_celsius[4][0]);
    volts2temp(GPIO6_voltage[0], REF2_voltage[0], temp_celsius[5][0]);
    volts2temp(GPIO7_voltage[0], REF2_voltage[0], temp_celsius[6][0]);
    volts2temp(GPIO8_voltage[0], REF2_voltage[0], temp_celsius[7][0]);

    // Switch to the other side of the muxes
    for(int ic_num = 0; ic_num < BTM_NUM_DEVICES; ic_num++) {
        cfgrb_to_write[ic_num][0] |= GPIO_9_TOGGLE; // GPIO 9 = 1
        // update the stored configuration register
        pack->stack[ic_num].cfgrb[0] = cfgrb_to_write[ic_num][0];
    }

    BTM_writeRegisterGroup(CMD_WRCFGB, cfgrb_to_write);

    status = readThermistorVoltage
    (
        GPIO1_voltage[1],
        GPIO2_voltage[1],
        GPIO3_voltage[1],
        GPIO4_voltage[1],
        GPIO5_voltage[1],
        GPIO6_voltage[1],
        GPIO7_voltage[1],
        GPIO8_voltage[1],
        REF2_voltage[1]
    );

    if (status.error != BTM_OK)
        return status;

    volts2temp(GPIO1_voltage[1], REF2_voltage[1], temp_celsius[0][1]);
    volts2temp(GPIO2_voltage[1], REF2_voltage[1], temp_celsius[1][1]);
    volts2temp(GPIO3_voltage[1], REF2_voltage[1], temp_celsius[2][1]);
    volts2temp(GPIO4_voltage[1], REF2_voltage[1], temp_celsius[3][1]);
    volts2temp(GPIO5_voltage[1], REF2_voltage[1], temp_celsius[4][1]);
    volts2temp(GPIO6_voltage[1], REF2_voltage[1], temp_celsius[5][1]);
    volts2temp(GPIO7_voltage[1], REF2_voltage[1], temp_celsius[6][1]);
    volts2temp(GPIO8_voltage[1], REF2_voltage[1], temp_celsius[7][1]);

    // Copy gathered temperature data to pack data structure
    for(int ic_num = 0; ic_num < BTM_NUM_DEVICES; ic_num++)
    {
        for(int gpio_num = 0; gpio_num < NUM_GPIOS; gpio_num++) {
            for(int mux_channel = 0; mux_channel < MUX_CHANNELS; mux_channel++) {
                pack->stack[ic_num].module[2 * gpio_num + mux_channel].temperature =
                    temp_celsius[gpio_num][mux_channel][ic_num];
            }
        }
    }

    return status;
}

/*
Function name: readThermistorVoltage
Purpose: get voltage reading of thermistors directly from GPIO pin per LTC6813, and output into a register
input:
uint16_t GPIOx_voltage[BTM_NUM_DEVICES] - two bytes long as it is the concatenation of two register bytes (x = 1-8).
uint16_t REF2_voltage[BTM_NUM_DEVICES] - two bytes long as it is the concatenation of two register bytes
For each LTC6813 device, internal functions pull voltage readings of GPIO pin from hardware registers.
output:
- Returns status value indicating success of register read.
- Readings stored in array. See input.
*/
BTM_Status_t readThermistorVoltage(
    uint16_t GPIO1_voltage[BTM_NUM_DEVICES],
    uint16_t GPIO2_voltage[BTM_NUM_DEVICES],
    uint16_t GPIO3_voltage[BTM_NUM_DEVICES],
    uint16_t GPIO4_voltage[BTM_NUM_DEVICES],
    uint16_t GPIO5_voltage[BTM_NUM_DEVICES],
    uint16_t GPIO6_voltage[BTM_NUM_DEVICES],
    uint16_t GPIO7_voltage[BTM_NUM_DEVICES],
    uint16_t GPIO8_voltage[BTM_NUM_DEVICES],
    uint16_t REF2_voltage[BTM_NUM_DEVICES]
)
{
    uint8_t registerAUXA_voltages[BTM_NUM_DEVICES][BTM_REG_GROUP_SIZE];
    uint8_t registerAUXB_voltages[BTM_NUM_DEVICES][BTM_REG_GROUP_SIZE];
    uint8_t registerAUXC_voltages[BTM_NUM_DEVICES][BTM_REG_GROUP_SIZE];
    //uint8_t registerAUXD_voltages[BTM_NUM_DEVICES][BTM_REG_GROUP_SIZE];
    uint16_t voltage_reading = 0;
    BTM_Status_t status = {BTM_OK, 0};
    //start conversion
    //ADAX, ADC AuXillary start-conversion command
    status = BTM_sendCmdAndPoll(CMD_ADAX_ALL);
    if (status.error != BTM_OK) return status; // There's a communication problem

    //retrieve register readings
    status = BTM_readRegisterGroup(CMD_RDAUXA, registerAUXA_voltages);
    if (status.error != BTM_OK) return status; // There's a communication problem
    status = BTM_readRegisterGroup(CMD_RDAUXB, registerAUXB_voltages);
    if (status.error != BTM_OK) return status; // There's a communication problem
    status = BTM_readRegisterGroup(CMD_RDAUXC, registerAUXC_voltages); // CMD_RDAUXC need to be added to the header file (CMD_RDAUXC  = 0x000D)
    if (status.error != BTM_OK) return status; // There's a communication problem
    /*status = BTM_readRegisterGroup(CMD_RDAUXD, registerAUXD_voltages); // CMD_RDAUXD need to be added to the header file (CMD_RDAUXD  = 0x000F)
    if (status.error != BTM_OK) return status; // There's a communication problem*/

    //output reading by assigning to pointed array the first two bytes of registerAUXA_voltages
    for (int board = 0; board < BTM_NUM_DEVICES; board++)
    {
        // Combine 2 bytes of voltage reading
        voltage_reading = ( ((uint16_t) registerAUXA_voltages[board][1]) << 8)
            | registerAUXA_voltages[board][0];
        // Store in given array
        GPIO1_voltage[board] = voltage_reading;

        voltage_reading = ( ((uint16_t) registerAUXA_voltages[board][3]) << 8)
            | registerAUXA_voltages[board][2];
        GPIO2_voltage[board] = voltage_reading;

        voltage_reading = ( ((uint16_t) registerAUXA_voltages[board][5]) << 8)
            | registerAUXA_voltages[board][4];
        GPIO3_voltage[board] = voltage_reading;

        voltage_reading = ( ((uint16_t) registerAUXB_voltages[board][1]) << 8)
            | registerAUXB_voltages[board][0];
        GPIO4_voltage[board] = voltage_reading;

        voltage_reading = ( ((uint16_t) registerAUXB_voltages[board][3]) << 8)
            | registerAUXB_voltages[board][2];
        GPIO5_voltage[board] = voltage_reading;

        voltage_reading = ( ((uint16_t) registerAUXB_voltages[board][5]) << 8)
            | registerAUXB_voltages[board][4];
        REF2_voltage[board] = voltage_reading;

        voltage_reading = ( ((uint16_t) registerAUXC_voltages[board][1]) << 8)
            | registerAUXC_voltages[board][0];
        GPIO6_voltage[board] = voltage_reading;

        voltage_reading = ( ((uint16_t) registerAUXC_voltages[board][3]) << 8)
            | registerAUXC_voltages[board][2];
        GPIO7_voltage[board] = voltage_reading;

        voltage_reading = ( ((uint16_t) registerAUXC_voltages[board][5]) << 8)
            | registerAUXC_voltages[board][4];
        GPIO8_voltage[board] = voltage_reading;

        /*voltage_reading = ( ((uint16_t) registerAUXD_voltages[board][1]) << 8)
            | registerAUXD_voltages[board][0];
        GPIO9_voltage[board] = voltage_reading;*/
    }

    return status;
}


/**
 * @brief Converts a raw thermistor voltage reading from an LTC6813 into a temperature in degrees celsius
 * @param[in] Vout the thermistor voltage reading to convert
 * @param[in] REF2[] the measured reference voltage
 * @return the temperature of the thermistor in degrees celcius
 */
void volts2temp(uint16_t ADC[], uint16_t REF2[], float temp_celsius[])
{
    const float beta = 3435.0; // from NTC datasheet
    const float room_temp = 298.15; // kelvin
    const float R_balance = 10000.0; // from LTC6813 datasheet p.84. note: this doesn't account for tolerance. to be exact, measure the 10k resistor with a multimeter
    const float R_room_temp = 10000.0; // resistance at room temperature (25C)
    float Vs = 0.0;
    float R_therm = 0.0;
    float temp_kelvin = 0.0;
    float Vout = 0.0;

    // to get the voltage in volts from the LTC6813's value,
    // multiply it by 0.0001 as below.
    for(int board = 0; board < BTM_NUM_DEVICES; board++)
    {
        Vs = BTM_regValToVoltage(REF2[board]);
        Vout = BTM_regValToVoltage(ADC[board]);
        R_therm = R_balance * ((Vs / Vout) - 1);
        temp_kelvin = (beta * room_temp)
            / (beta + (room_temp * log(R_therm / R_room_temp)));
        temp_celsius[board] = temp_kelvin - 273.15;
    }
}
