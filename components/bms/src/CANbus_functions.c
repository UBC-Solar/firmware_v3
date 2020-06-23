/*
CANbus_functions.c

This file contains the suite of functions used for CAN functionality

*/


#include "CANbus_functions.h"
#include "stm32f3xx_hal.h"
#include "stm32f3xx_hal_can.h"
#include <math.h>
//#include <stdbool.h>

//private variables
BTM_PackData_t PH_PACKDATA;


//CAN_TxHeaderTypeDef PH_CANheader;
Brightside_CAN_MessageTypeDef CANmessages_elithionSeries[PH_SERIES_SIZE];


// uint8_t PH_LookUpTable[BTM_NUM_DEVICES + 1][BTM_NUM_MODULES + 1] =
//     {
//         {0,0,0,0},
//         {0,1,2,3},
//         {0,4,5,6},
//         {0,7,8,9}
//     };

uint8_t LUT_moduleStickers[BTM_NUM_DEVICES][BTM_NUM_MODULES] =
    {
        { 1, 2, 3, 4, 5,91, 6, 7, 8, 9,10,92},
        {11,12,13,14,15,93,16,17,18,19,20,94},
        {21,22,23,24,25,26,27,28,29,30,31,32}
    }
    //note that the 9X numbers (91, 92, 93, 94) indicate the garbage channels.
    //Should any of the 9X numbers appear in a CAN message, something is not right.

//function prototypes
void CAN_InitHeaderStruct(Brightside_CAN_MessageTypeDef * CANmessages_elithionSeries[])
void CAN_CompileMessage623(uint8_t aData_series623[8], BTM_PackData_t * pPH_PACKDATA);
void CAN_CompileMessage626(uint8_t aData_series626[8], BTM_PackData_t * pPH_PACKDATA);
void CAN_CompileMessage627(uint8_t aData_series627[8], BTM_PackData_t * pPH_PACKDATA);
//For message addressed 623.
void VoltageInfoRetrieval(
    BTM_PackData_t * pPH_PACKDATA,
    uint16_t * pMinVoltage,
    uint16_t * pMaxVoltage,
    uint8_t * pMinStack,
    uint8_t * pMinModule,
    uint8_t * pMaxStack,
    uint8_t * pMaxModule);
unsigned int outOfBoundsAndCast_packVoltage(float packVoltageFLOAT, uint8_t * outOfBounds)
uint8_t outOfBoundsAndConvert_moduleVoltage(float moduleVoltageFLOAT, uint8_t * outOfBounds)
void temperatureDataRetrieval(
    BTM_PackData_t * pPH_PACKDATA,
    uint8_t * averageTemperature,
    uint16_t * minTmp,
    uint16_t * maxTmp,
    uint8_t * minTmpStack,
    uint8_t * maxTmpStack,
    uint8_t * minTmpModule,
    uint8_t * maxTmpModule);
uint8_t TwosComplement_TemperatureConverter(double temperatureDOUBLE, uint8_t * outOfBounds);

//function definitions


/*******************************
*
*      TOP LEVEL FUNCTIONS
*
********************************/
uint8_t PH_TOP_level_functions;

void CAN_InitHeaderStruct(Brightside_CAN_MessageTypeDef * CANmessages_elithionSeries[])
{
    for(int i=0 ; i < PH_SERIES_SIZE ; ++i)
    {
        CANmessages_elithionSeries[i] -> messageHeader.StdId
            = PH_START_OF_ADDRESS_SERIES + i;
        CANmessages_elithionSeries[i] -> messageHeader.ExtId = PH_UNUSED;
        CANmessages_elithionSeries[i] -> messageHeader.IDE = CAN_ID_STD;   //Predefined constant in stm32 include file.
        CANmessages_elithionSeries[i] -> messageHeader.RTR = CAN_RTR_DATA; //Predefined constant in stm32 include file.
        CANmessages_elithionSeries[i] -> messageHeader.DLC = CAN_BRIGHTSIDE_DATA_LENGTH;
        CANmessages_elithionSeries[i] -> messageHeader.TransmitGlobalTime = DISABLE;//We could use this eventually, if we use a custom message format
    }

    //separate assignments for setting unique, non-consecutive addresses.
    CANmessages_elithionSeries[0] -> messageHeader.StdId = ADDRESS_623;
    CANmessages_elithionSeries[1] -> messageHeader.StdId = ADDRESS_627;
}

void PH_CAN_FillDataFrames_TESTING(Brightside_CAN_MessageTypeDef * CANmessages_elithionSeries[], BTM_PackData_t * pPH_PACKDATA)
{
    CAN_CompileMessage623(CANmessages_elithionSeries[0] -> &dataFrame[], pPH_PACKDATA);
    CAN_CompileMessage627(CANmessages_elithionSeries[1] -> &dataFrame[], pPH_PACKDATA);

    //print contents of dataFrame for testing purposes
    for(int i = 0; i < PH_SERIES_SIZE; ++i)
    {
        for(int j = 0; i < CAN_BRIGHTSIDE_DATA_LENGTH ; ++i)
            {
            printf("Address: %u    Index: %i    Data: %u /r/n",
                CANmessages_elithionSeries[i] -> messageHeader.StdId,
                j,
                CANmessages_elithionSeries[i] -> dataFrame[i]);
        }
    }
}

void PH_CANstate(Brightside_CAN_MessageTypeDef * messageSeries[]){
    uint8_t errorFlag = 0;
    int i = 0;

    //check if there are still pending messages in the transmission mailboxes.
    //NOTE: assumption is that the TxMailboxes param (2nd parameter)
    //is using one-hot encoding, allowing for CAN_TX_MAILBOX0, CAN_TX_MAILBOX1,
    //and CAN_TX_MAILBOX2 to superimpose onto each other.
    //See actual definition of CAN_TX_MAILBOX0, CAN_TX_MAILBOX1, and CAN_TX_MAILBOX2
    //for the one hot encoding.
    // if(HAL_CAN_IsTxMessagePending(PH_hcan, 0x111u) == CAN_PENDING){
    //     errorFlag = 1; //1 for true
    // }
    if(HAL_CAN_GetTxMailboxesFreeLevel(CAN_HandleTypeDef * hcan) != 3){
        errorFlag = 1;
    }

    //Continue with placing new messages
    if(HAL_CAN_IsTxMessagePending(PH_hcan, CAN_TX_MAILBOX0) == CAN_NOT_PENDING){
        HAL_CAN_AddTxMessage(PH_hcan, messageSeries[i]->header);
    }



}

/*******************************
*
*      HELPER FUNCTIONS
*
********************************/
uint8_t PH_HELPER_functions;
/**
Function name: CAN_CompileMessage623
Function purpose:
    Retrieve data, translate it, then format it into a message matching Elithion's format.
    See the webstie for formatting details: https://www.elithion.com/lithiumate/php/controller_can_specs.php

Algorithm:
    1) Retrieve voltage data specified.
        Note: this step does its best to maintain the original data type.
    2) Translate gathered data into expected units and cast into uint8_t.
        Note: this step is where data is made to match Elithion format's units, and where the numbers are casted to uint8_t.
    3) Place data into message array, while following Elithion format.

*/
void CAN_CompileMessage623(uint8_t aData_series623[8], BTM_PackData_t * pPH_PACKDATA)
{
    unsigned int
        packVoltage = 0;
    uint16_t
        minVtg = 0,
        maxVtg = 0;
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
    uint8_t
        outOfBounds = 0;
    float
        packVoltageFLOAT = 0;
        minVtgFLOAT = 0,
        maxVtgFLOAT = 0;

  //Collecting and translating the collected data into CAN frame format

  //gather min and max voltages
    VoltageInfoRetrieval(
        &pPH_PACKDATA,
        &minVtg,
        &maxVtg,
        &minStack,
        &minModule,
        &maxStack,
        &maxModule
    );


    /**
    pack Voltage
    format:
      unsigned, 0 to 65 kV -> ASSUMING decimal values from 0 to 65 000
      because it's alloted 2 bytes in the data frame
    */


    //Convert units of 100uV to V.
    //Then checks if value is outside of expected bounds, then truncates float to unsigned int.
    packVoltageFLOAT = BTM_regValToVoltage(pPH_PACKDATA -> pack_voltage);
    packVoltage = outOfBoundsAndCast_packVoltage(packVoltageFLOAT, &outOfBounds);

    //Convert units of 100uV to V.
    //Then check if value is out of out of expected bounds, and cast uint16_t to uint8_t.
    minVtgFLOAT = BTM_regValToVoltage(minVtg);
    minVtgBYTE = outOfBoundsAndConvert_moduleVoltage(minVtgFLOAT, &outOfBounds);

    maxVtgFLOAT = BTM_regValToVoltage(maxVtg);
    maxVtgBYTE = outOfBoundsAndConvert_moduleVoltage(maxVtgFLOAT, &outOfBounds);

    minBattModuleSticker = LUT_moduleStickers[minStack][minModule];
    maxBattModuleSticker = LUT_moduleStickers[maxStack][maxModule];


    //setting byte order in aData_series623 array
    aData_series623[0] = (uint8_t)(packVoltage >> 8);//intent: most-sig half of pack_voltage is bit-shifted right by 8 bits, such that ONLY the MSH is casted.
    aData_series623[1] = (uint8_t)(packVoltage);     //intent: only the LSB half is stored. the MSB half is truncated by the casting.
    aData_series623[2] = minVtgBYTE;                 //NOTE: the voltage is in units of 100mV
    aData_series623[3] = minBattModuleSticker;
    aData_series623[4] = maxVtgBYTE;                 //NOTE: the voltage is in units of 100mV
    aData_series623[5] = maxBattModuleSticker;
    //aData_series623[6] = 0;                        //redundant
    aData_series623[7] = outOfBounds;

  //end of function
}
/**
Function name: CAN_CompileMessage627
Function purpose:
    To pull/process information from structs, and format it into a CAN-ready message.
    In this case, the information specified is:
         - average temperature
         - min temperature
         - max temperature
         - cell number with min temp
         - cell number with max temp

Parameters:
    uint8_t aData_series627[8] - Array pre-initialised with zeros.
    BTM_PackData_t * pPH_PACKDATA - pointer to struct containing battery-module measurements.

Return:
    void

"Output":
    aData_series627[8] should have data in the proper order as per the Elithion format.
    See the website: https://www.elithion.com/lithiumate/php/controller_can_specs.php

Algorithm:
    1) Gather/calculate information from structs.
        Note: this step does its best to maintain the original data type.
    2) Translate gathered information to expected units and data type of uint8_t.
        Note: this step is where data is made to match Elithion format's units, and where the numbers are casted to uint8_t.
    3) Place data into message array, while matching the Elithion format.

*/
void CAN_CompileMessage627(uint8_t aData_series627[8], BTM_PackData_t * pPH_PACKDATA){
    uint8_t
        averageTemperatureBYTE = 0,
        minTmpBYTE = 0,
        maxTmpBYTE = 0,
        minTmpModuleSticker = 0,
        maxTmpModuleSticker = 0;
    uint8_t
        minTmpStack = 0,
        maxTmpStack = 0,
        minTmpModule = 0,
        maxTmpModule = 0,
        outOfBounds = 0;
    uint16_t
        minTmp,
        maxTmp;
    float
        averageTemperatureFLOAT,
        minTmpFLOAT,
        maxTmpFLOAT;
    double
        minTmpDOUBLE,
        maxTmpDOUBLE;

    //1) scans the struct and calculates the relevant information needed
    temperatureDataRetrieval(
        &averageTemperatureFLOAT,
        &minTmp,
        &maxTmp,
        &minTmpStack,
        &maxTmpStack,
        &minTmpModule,
        &maxTmpModule
    );

    //2) Translating Data

    averageTemperatureBYTE = (uint8_t)averageTemperatureFLOAT;

    minTmpDOUBLE = BTM_TEMP_volts2temp((double)minTmp);
    minTmpBYTE = TwosComplement_TemperatureConverter(minTmpDOUBLE, &outOfBounds);

    maxTmpDOUBLE = BTM_TEMP_volts2temp((double)maxTmp);
    maxTmpBYTE = TwosComplement_TemperatureConverter(maxTmpDOUBLE, &outOfBounds);

    maxTmpModuleSticker = LUT_moduleStickers[minTmpStack][minTmpModule];
    maxTmpModuleSticker = LUT_moduleStickers[maxTmpStack][maxTmpModule];

    //3) Placing data into message array.
    aData_series627[0] = averageTemperatureBYTE;
    // aData_series627[1] = 0; //redundant
    aData_series627[2] = minTmpBYTE;
    aData_series627[3] = minTmpModuleSticker;
    aData_series627[4] = maxTmpBYTE;
    aData_series627[5] = maxTmpModuleSticker;
    // aData_series627[6] = 0; //redundant
    aData_series627[7] = outOfBounds;
}


/*
Function Name: VoltageInfoRetrieval
Function Purpose: Scan the array of voltages of the modules per stacks, and return the min and max voltages

Parameters:
    pointers:
        pMinVoltage - pointer to variable to hold minimum voltage found.
        pMaxVoltage - pointer to variable to hold maximum voltage found.
        .....
        pMinStack - pointer to variable to hold INDEX of the battery stack containing the battery module with minimum voltage found.
        pMinModule - pointer to variable to hold INDEX of the module with the minimum voltage found.
        .....
        pMaxStack - pointer to variable to hold INDEX of the battery stack containing the battery module with maximum voltage found.
        pMaxModule - pointer to variable to hold INDEX of the module with the maximum voltage found.

Return:
    void

Output:
    Data is stored in variables pointed to. See input.

Algorithm:
    1) Loop one stack and one module at a time.
    Per loop iteration:
    1.1) Retrieve localVoltage.
    1.2) Compare localVoltage to running min and max voltages collected. Replace if needed.
    1.3) If min or max voltages are updated, record stack and module INDICES.
    1.4) Repeat until all modules have been analysed.
    2) Assign final numbers to pointed-to variables.
*/
void VoltageInfoRetrieval(
    BTM_PackData_t * pPH_PACKDATA,
    uint16_t * pMinVoltage,
    uint16_t * pMaxVoltage,
    uint8_t * pMinStack,
    uint8_t * pMinModule,
    uint8_t * pMaxStack,
    uint8_t * pMaxModule
)
{
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
            if(PH_PACKDATA -> stack[i].module[j].enable == 1)
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
    }

  //"return" min and max voltage
    *pMinVoltage = localMinVolt;
    *pMaxVoltage = localMaxVolt;
    *pMinStack = minStack;
    *pMinModule = minModule;
    *pMaxStack = maxStack;
    *pMaxModule = maxModule;

}


/**
Function Name: outOfBoundsAndCast_packVoltage
Function Purpose:
    Check if the pack voltage collected is outside the expected message.
        If out of bounds, assign the broken bound and cast to unsigned int.
        Else, return exact value casted to unsigned int.

Parameters:
    float packVoltageFLOAT - The voltage to check.
    uint8_t * outOfBounds - The pointer to a variable used as a flag for if bounds are broken.

Return:
    The packVoltageFLOAT parameter casted to unsigned int.
*/
unsigned int outOfBoundsAndCast_packVoltage(float packVoltageFLOAT, uint8_t * outOfBounds){
    if(packVoltage < CAN_PACK_MINIMUM){
        *outOfBounds = 1
        return CAN_PACK_MINIMUM;
    }
    else if(packVoltage > CAN_PACK_MAXIMUM){
        *outOfBounds = 1
        return CAN_PACK_MAXIMUM;
    }
    else
    return (unsigned int)packVoltageFLOAT; //DOUBLE CHECK IF THE CASTING WORKS
}

/**
Function Name: outOfBoundsAndConvert_moduleVoltage
Function Purpose:
    Check if the module voltage collected is outside the expected message.
        If out of bounds, assign the broken bound and cast to uint8_t.
        Else, return exact value casted to uint8_t.

Parameters:
    float moduleVoltageFLOAT - The voltage to check.
    uint8_t * outOfBounds - The pointer to a variable used as a flag for if bounds are broken.

Return:
    The moduleVoltage100mV value, casted to uint8_t.
    moduleVoltage100mV is moduleVoltageFLOAT converted to units of 100mV.
*/
uint8_t outOfBoundsAndConvert_moduleVoltage(float moduleVoltageFLOAT, uint8_t * outOfBounds){
    float moduleVoltage100mV = moduleVoltageFLOAT * 10;

    if(moduleVoltage100mV < CAN_MODULE_MINIMUM){
        *outOfBounds = 1
        return CAN_MODULE_MINIMUM;
    }
    else if(moduleVoltage100mV > CAN_MODULE_MAXIMUM){
        *outOfBounds = 1
        return CAN_MODULE_MAXIMUM;
    }
    else
    return (uint8_t)moduleVoltage100mV; //DOUBLE CHECK IF THE CASTING WORKS
}

/*
Function Name: temperatureDataRetrieval
Function Purpose: Scan the array of temperature-voltages of the modules per stacks, find the min and max temperature-voltages, and calculate the average temperature.
Note: The voltage measurements are from thermistors, whose resistances vary with temperature.
    This and related functions do NOT return temperature in units of degree Celcius unless it explicitly says it does.

Parameters:
    Pointers:
        pAverageTemperature - pointer to variable for holding the average temperature-voltage calculated.
        .....
        pMinVoltage - pointer to variable to hold minimum voltage found.
        pMaxVoltage - pointer to variable to hold maximum voltage found.
        .....
        pMinStack - pointer to variable to hold INDEX of the battery stack containing the battery module with minimum voltage found.
        pMinModule - pointer to variable to hold INDEX of the module with the minimum voltage found.
        .....
        pMaxStack - pointer to variable to hold INDEX of the battery stack containing the battery module with maximum voltage found.
        pMaxModule - pointer to variable to hold INDEX of the module with the maximum voltage found.

Algorithm:
1) loop through each stack for each battery-module's measurements.
Per loop iteration:
1.1) Retrieve localTemperature.
1.2) Calculate the average based on the temperatures gathered up to this loop iteration; a running average.
1.3) Compare local temperature to the current minimum and maximum temperatures measured/initialised. Replace as required.
1.4) If minimum and/or maximum temperatures are updated, record the stack and module INDICES.
1.5) Repeat until all modules have been analysed.
2) the final numbers are placed into the pointed variables.
2.1) The running average of all module temperatures is now the entire pack's temperature average.
*/
void temperatureDataRetrieval(
    BTM_PackData_t * pPH_PACKDATA
    float * pAverageTemperature,
    uint16_t * pMinTmp,
    uint16_t * pMaxTmp,
    uint8_t * pMinTmpStack,
    uint8_t * pMaxTmpStack,
    uint8_t * pMinTmpModule,
    uint8_t * pMaxTmpModule
)
{
    uint16_t
        localTemperature = 0,
        localMinTmp = 65585,
        localMaxTmp = 0;
    uint8_t
        minStack,
        maxStack,
        minModule,
        maxModule;
    double
        temperatureTotal = 0,
        localAverage = 0;

    int
        i = 0;
        total_mux_read = 0;

    for(i = 0; i < BTM_NUM_DEVICES; ++i)
    {
        for(int j = 0; j < BTM_NUM_MODULES; ++j)
        {
            if(pPH_PACKDATA -> stack[i].module[j].enable == 1)
            {
                localTemperature = pPH_PACKDATA -> stack[i].module[j].temperature;

                //double type is used to avoid possible integer overflow.
                //localTemperature is uint16_t. Reasonably, in this loop, it should
                //never add up to a number greater than the max double value.
                //assuming that the thermistors can handle up to the solder melting point,
                //about 300 degree Celcius, the three digits multiplied by 12 is way-way-way
                //lower than the max value of data type double.
                //So, the risk of overflow should be negligible.
                temperatureTotal = temperatureTotal + localTemperature;

                if(localTemperature < localMinTmp){
                    localMinTmp = localTemperature;
                    minStack = i;
                    minModule = j;
                }

                if(localTemperature > localMaxTmp){
                    localMaxTmp = localTemperature;
                    maxStack = i;
                    maxModule = j;
                }
                total_mux_read++;
            }

        }
    }

    localAverage = temperatureTotal / total_mux_read;

    //Ensuring that the value fits into the uint16_t size.
    //If it's breaks the bounds, it will be set to the bounds.
    //If the bounds ever appear, that is a sign that something may be off.
    if(localAverage > 65535){
        localAverage = 65535;
    }
    else if(localAverage < 0){
        localAverage = 0;
    }

    *pAverageTemperature = (uint16_t)localAverage;
    *pMinTmp = localMinTmp;
    *pMaxTmp = localMaxTmp;
    *pMinStack = minStack;
    *pMinModule = minModule;
    *pMaxStack = maxStack;
    *pMaxModule = maxModule;
}


/*
Function Name: TwosComplement_TemperatureConverter
Function purpose:
    Check if value is within expected message bounds, and returns the value converted to two's complement.
Parameters:
    double temperatureDOUBLE - the temperature in Celcius.
    uint8_t * outOfBounds - The pointer to a variable used as a flag for if bounds are broken.

Return:
    Tmperature, casted to uint8_t.

Algorithm:
    1) Check if greater-than upper bound.
        If true, set outOfBounds flag and return the bound broken.
    2) Check if less-than lower bound.
        If true, set outOfBounds flag and return the bound broken.
        Note that the bounds are intentionally set to two's complement min and max for uint8_t size.
    3) Cast temperatureDOUBLE to uint8_t and assign to temperatureBYTE
    4) Convert temperatureBYTE to two's complement, then return the value.
*/
uint8_t TwosComplement_TemperatureConverter(double temperatureDOUBLE, uint8_t * outOfBounds)
{
    uint8_t temperatureBYTE;

    if(temperatureDOUBLE > CAN_TEMPERATURE_MAXIMUM)
    {
        *outOfBounds = 1;
        return CAN_TEMPERATURE_MAXIMUM;
    }

    else if(temperatureDOUBLE < CAN_TEMPERATURE_MINIMUM)
    {
        *outOfBounds = 1;
        return CAN_TEMPERATURE_MINIMUM;
    }

    else
    {
        temperatureBYTE = (uint8_t)temperatureDOUBLE;
        if(temperatureBYTE >= 0)
            return temperatureBYTE;
        else
            //Conversion to two's complement, for negative numbers.
            return ~temperatureBYTE + 1;
    }
}


/*

Other Functions

*/
uint8_t PH_OTHER_functions;

uint8_t celciusAverage(BTM_PackData_t * pPH_PACKDATA){
    uint16_t localTemperature = 0;
    double temperatureTotal = 0;
    double localAverage = 0;

    int i = 0;

    for(i = 0; i < BTM_NUM_DEVICES; ++i)
    {
        for(int j = 0; j < BTM_NUM_MODULES; ++j)
        {
            if(pPH_PACKDATA -> stack[i].module[j].enable == 1)
            {
                localTemperature = pPH_PACKDATA -> stack[i].module[j].temperature;
                temperatureTotal = temperatureTotal + BTM_TEMP_volts2temp(localTemperature);
            }
        }
    }

    localAverage = temperatureTotal / PH_TOTAL_MUX;

    if(localAverage > 255)
    {
        return 255;
    }
    else if(localAverage < 0)
    {
        return 0;
    }
    else
    {
        return (uint8_t)localAverage;
    }

}

    //placeholder code, to see the format of struct calls.
    //Looks really clean
    //retreving voltage of module 12 of stack 3 (note the off-by-one array index)
    //uint16_t PH_ModuleVOLTAGE = PACKDATApointer -> stack[2].module[11].voltage;
