#include "CANbus_functions.h"
#include "stm32f3xx_hal.h"
#include <math.h>

//private variables
BTM_PackData_t PH_PACKDATA;
uint8_t aData_series623[8] = { 0 };
uint8_t aData_series624[8] = { 0 };
uint8_t aData_series626[8] = { 0 };
uint8_t aData_series627[8] = { 0 };

  //DESIGN UNCERTAINTY: off-by-one dimensioning?
uint8_t PH_LookUpTable[BTM_NUM_DEVICES + 1][BTM_NUM_MODULES + 1] =
    {
        {0,0,0,0},
        {0,1,2,3},
        {0,4,5,6},
        {0,7,8,9}
    }

//function prototypes
void CANinfoPullAndFormatMessage623();

//For message addressed 623.
void VoltageComparator(uint16_t * pMinVoltage, uint16_t * pMaxVoltage);
void packVoltageEncoder(unsigned int pack_voltage);
uint8_t VoltageUnitShifter(uint16_t reading_that_was_decimal_shifted_to_avoid_floating_point_numericals);

void CANinfoPullAndFormatMessage626();
void CANinfoPullAndFormatMessage627();
void CANinfoPullAndFormatCompact();

//function definitions

//purpose: pull info and format
//input: array to fill with message
//output: see input; array is filled with message
void CANinfoPullAndFormatSeries623(uint8_t aData_series623[8], BTM_PackData_t * pPH_PACKDATA)
{
    unsigned int
        packVoltage = 0;
    uint16_t
        MinVtg = 0,
        MaxVtg = 0;
    uint8_t
        minStack = 0,
        minModule = 0,
        maxStack = 0,
        maxModule = 0;
    uint8_t
        minVtgBYTE = 0,
        maxVtgBYTE = 0,
        minBattModuleSticker = 0,
        maxBattModuleSticker = 0;

  //Collecting and translating the collected data into CAN frame format

  //gather min and max voltages
    VoltageComparator(&MinVtg, &MaxVtg, &minStack, &minModule, &maxStack, &maxModule, &pPH_PACKDATA);
    MinVtgBYTE = VoltageUnitShifter(MinVtg);
    MaxVtgBYTE = VoltageUnitShifter(MaxVtg);
    minBattModuleSticker = PH_LookUpTable[minStack][minModule];
    maxBattModuleSticker = PH_LookUpTable[maxStack][maxModule];

  /**
  pack Voltage
  format:
    unsigned, 0 to 65 kV -> ASSUMING decimal values from 0 to 65 000
    because it's alloted 2 bytes in the data frame
  */
  packVoltage = packVoltageEncoder(PH_PACKDATA -> pack_voltage);

    //setting byte order in aData_series623 array
    aData_series623[0] = (uint8_t)(packVoltage >> 8);//PROBABLY DONE WRONG //intent: most-sig half of pack_voltage is bit-shifted right by 8 bits, such that ONLY the MSH is casted.
    aData_series623[1] = (uint8_t)(packVoltage);
    aData_series623[2] = minVtgBYTE;
    aData_series623[3] = minBattModuleSticker;
    aData_series623[4] = maxVtgBYTE;
    aData_series623[5] = maxBattModuleSticker;
    //aData_series623[6] = 0; //redundant
    //aData_series623[7] = 0; //redundant

  //end of function
}

void CANinfoPullAndFormatMessage627(){
    uint8_t
        averageTemperature = 0,
        minTmp = 0,
        maxTmp = 0,
        minTmpModuleSticker = 0,
        maxTmpModuleSticker = 0;

    averageTemperature = temperatureDataRetrieval();


    aData_series627[0] = averageTemperature;
    // aData_series627[1] = 0; //redundant
    aData_series627[2] = minTmp;
    aData_series627[3] = minTmpModuleSticker;
    aData_series627[4] = maxTmp;
    aData_series627[5] = maxTmpModuleSticker;
    // aData_series627[6] = 0; //redundant
    // aData_series627[7] = 0; //redundant
}


/*
Function Name: VoltageComparator
Function Purpose: Scan the array of voltages of the modules per stacks, and return the min and max temperature-voltages

Input:
    pointers:
        pMinVoltage - pointer to variable to hold minimum voltage found.
        pMaxVoltage - pointer to variable to hold maximum voltage found.
        .....
        pMinStack - pointer to variable to hold INDEX of the battery stack containing the battery module with minimum voltage found.
        pMinModule - pointer to variable to hold INDEX of the module with the minimum voltage found.
        .....
        pMaxStack - pointer to variable to hold INDEX of the battery stack containing the battery module with maximum voltage found.
        pMaxModule - pointer to variable to hold INDEX of the module with the maximum voltage found.

Output:
    Data is stored in variables pointed to. See input.
*/
void VoltageComparator(
    uint16_t * pMinVoltage, uint16_t * pMaxVoltage, uint8_t * pMinStack, uint8_t * pMinModule, uint8_t * pMaxStack, uint8_t * pMaxModule){
    uint16_t
        localMinVolt = 65535, //note that the raw register readings are decimal-shifted to avoid storing floating points. 2^16 - 1
        localMaxVolt = 0;
    uint8_t
        minStack = 0,
        minModule = 0,
        maxStack = 0,
        maxModule = 0;

  //combines the minVolt and maxVolt loops to reduce redundant struct pulls.
    for(int i = 0; i < BTM_NUM_DEVICES; ++i)
    {
        for(int j = 0; j < BTM_NUM_MODULES; ++j)
        {

            localVoltage = PH_PACKDATA -> stack[i].module[j].voltage;

            if(localVoltage < localMinVolt)
            {
                localMinVolt = localVoltage;
                minStack = i;
                minModule = j;
            }
            if(localVoltage > localMaxVolt)
            {
                localMaxVolt = localVoltage;
                maxStack = i;
                maxModule = j;
            }
        }
    }

  //"return" min and max voltage
    *pMinVoltage = localMinVolt;
    *pMaxVoltage = localMaxVolt;
    *pMinStack = minStack;
    *pMinModule = minModule;
    *pMaxStack = maxStack;
    *pMaxModule = maxModule;

}

unsigned int packVoltageEncoder(pack_voltage){
    //DESIGN UNCERTAINTY: what function updates pack_voltage in the struct?
    return pack_voltage;
}

uint8_t VoltageUnitShifter(uint16_t tenth_mV_reading){
    return 1337;
}

/*
Function Name: temperatureDataRetrieval
Function Purpose: Scan the array of temperature-voltages of the modules per stacks, find the min and max temperature-voltages, and calculate the average temperature.
Note: The voltage measurements are from thermistors, whose resistances vary with temperature.
      This and related functions do NOT return temperature in units of degree Celcius unless it explicitly says it does.
*/
void temperatureDataRetrieval()



    //placeholder code, to see the format of struct calls.
    //Looks really clean
    //retreving voltage of module 12 of stack 3 (note the off-by-one array index)
    //uint16_t PH_ModuleVOLTAGE = PACKDATApointer -> stack[2].module[11].voltage;
