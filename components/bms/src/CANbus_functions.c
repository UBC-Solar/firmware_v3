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

void CANinfoPullAndFormatMessage624();
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
        minBattModuleIndex = 0,
        maxBattModuleIndex = 0;

  //Collecting and translating the collected data into CAN frame format

  //gather min and max voltages
    VoltageComparator(&MinVtg, &MaxVtg, &minStack, &minModule, &maxStack, &maxModule, &pPH_PACKDATA);
    MinVtgBYTE = VoltageUnitShifter(MinVtg);
    MaxVtgBYTE = VoltageUnitShifter(MaxVtg);
    minBattModuleIndex = PH_LookUpTable[minStack][minModule];
    maxBattModuleIndex = PH_LookUpTable[maxStack][maxModule];

  /**
  pack Voltage
  format:
    unsigned, 0 to 65 kV -> ASSUMING decimal values from 0 to 65 000
    because it's alloted 2 bytes in the data frame
  */
  packVoltage = packVoltageEncoder(PH_PACKDATA -> pack_voltage);

    //setting byte order in aData_series623 array
    aData_series623[0] = (uint8_t)(packVoltage >> 8); //intent: most-sig half of pack_voltage is bit-shifted right by 8 bits, such that ONLY the MSH is casted.
    aData_series623[1] = (uint8_t)(packVoltage);
    aData_series623[2] = minVtgBYTE;
    aData_series623[3] = minBattModuleIndex;
    aData_series623[4] = maxVtgBYTE;
    aData_series623[5] = maxBattModuleIndex;
    //aData_series623[6] = 0; //redundant
    //aData_series623[7] = 0; //redundant

  //end of function
}

void CANinfoPullAndFormatMessage624()
{
    uint8_t
        current;

    current = ECUgpioInfoPull();

    aData_series624[0] = current;

    //end of code
}

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



    //placeholder code, to see the format of struct calls.
    //Looks really clean
    //retreving voltage of module 12 of stack 3 (note the off-by-one array index)
    //uint16_t PH_ModuleVOLTAGE = PH_PACKDATA.stack[2].module[11].voltage;
