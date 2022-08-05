#include "Pack_packdata.h"

BTM_PackData_t * pPACKDATA = NULL;

voltageInfoStruct voltageInfo;

temperatureInfoStruct temperatureInfo;

void Pack_initPackdataPtr(BTM_PackData_t * ptr)
{
    pPACKDATA = ptr;
}

short Pack_getPackCurrent()
{
    return 0;
}

/**
WIP
*/
uint64_t Pack_getFaultAndWarningStatusBits()
{
    uint32_t status_var = pPACKDATA->PH_status;
    // /*
    // Update stateBYTE.
    // */
    // //Bit 0: fault state.
    // if( (status_var & CAN_FAULT_VALUES) != 0)
    // {
    //   stateBYTE |= CAN_BITFLAG_FAULT_STATE;
    // }
    //
    // /*
    // Update faultCodeBYTE.
    // */
    //
    // // if(status_var & BMS_FAULT_COMM != 0)
    // // if(status_var & BMS_FAULT_OT != 0)
    // // if(status_var & BMS_FAULT_UV != 0)
    // // if(status_var & BMS_FAULT_OV != 0)
    // // if(status_var & BMS_FAULT_NO_VOLT != 0)
    //
    //
    // /*
    // Update levelFaultFlagsBYTE.
    // */
    // // Bit 7: Over voltage.
    // if((status_var & BMS_FAULT_OV) != 0)
    // {
    //   levelFaultFlagsBYTE |= CAN_FAULTFLAG_OVERVOLTAGE;
    // }
    // // Bit 6: Under voltage.
    // if((status_var & BMS_FAULT_UV) != 0)
    // {
    //   levelFaultFlagsBYTE |= CAN_FAULTFLAG_UNDERVOLTAGE;
    // }
    //
    // // Bit 5: Over-temperature.
    // if((status_var & BMS_FAULT_OT) != 0)
    // {
    //   levelFaultFlagsBYTE |= CAN_FAULTFLAG_OVERTEMP;
    // }
    //
    // // Bit 4: Discharge overcurrent.
    // // Bit 3: Charge overcurrent.
    //
    // // Bit 2: Communication fault with a bank or cell.
    // if((status_var & BMS_FAULT_COMM) != 0)
    // {
    //   levelFaultFlagsBYTE |= CAN_FAULTFLAG_COMMFAULT;
    // }
    // // Bit 1: Interlock is tripped.
    // // Bit 0: Driving off while plugged in.
    //
    // /*
    // Update warningFlagsBYTE.
    // */
    //
    // // Bit 7 : isolation fault.
    // // Bit 6 : low SOH.
    // // Bit 5 : hot temperature.
    // if((status_var & BMS_WARNING_HIGH_T) != 0)
    // {
    //   warningFlagsBYTE |= CAN_WARNFLAG_HIGHTEMP;
    // }
    //
    // // Bit 4 : cold temperature.
    // if((status_var & BMS_WARNING_LOW_T) != 0)
    // {
    //   warningFlagsBYTE |= CAN_WARNFLAG_LOWTEMP;
    // }
    //
    // // Bit 3 : discharge overcurrent.
    // // Bit 2 : charge overcurrent.
    // // Bit 1 : high voltage.
    // if((status_var & BMS_WARNING_HIGH_V) != 0)
    // {
    //   warningFlagsBYTE |= CAN_WARNFLAG_HIGHVOLTAGE;
    // }
    // // Bit 0 : low voltage.
    // if((status_var & BMS_WARNING_LOW_V) != 0)
    // {
    //   warningFlagsBYTE |= CAN_WARNFLAG_LOWVOLTAGE;
    // }

    return 0;
}

uint8_t Pack_getFaultCode()
{
    return 0;
}

uint8_t Pack_getTimeAfterPowerOn()
{
    return 0;
}
/**
@brief      Function Name: Pack_getPackVoltage
@details    Function purpose:
            getThePackVoltage from the packdata data pool, preconverted to voltage

@returns    int32_t voltage, intended to be casted to uint16_t, but defined as int32_t to use the signed bit as an error flag.

@par        Algorithm:

    1) get value of packvoltage from packdata that is stored as a multiple of 100uV (0.1 mV or 0.0001 V) then convert to volts.
    2) Check if greater-than upper bound.
        If true, set packVoltage to the negative of the minimum value, negative zero (-0).
    3) Check if less-than lower bound.
        If true, set packVoltage to negative of the maximum value, negative 65000 (-65000)
    4) if within bounds, cast to int32_t
    5) return

@note      The packVoltage value is returned as int32_t, but it should be treated and casted to uint16_t after checking for negative value. A negative value indicates an out-of-bounds event.
*/

uint32_t Pack_getPackVoltage()
{
    float packVoltageFLOAT;
    uint32_t packVoltage;

    //Convert units of 100uV to V.

    packVoltageFLOAT = (float)(pPACKDATA->pack_voltage) * 0.0001;

    //check if value is outside of expected bounds
    if(packVoltageFLOAT < CAN_PACK_MINIMUM){
        packVoltage = CAN_PACK_MINIMUM;
    }
    else if(packVoltageFLOAT > CAN_PACK_MAXIMUM){
        packVoltage = CAN_PACK_MAXIMUM;
    }
    else{ //if packVolage is within bounds
        packVoltage = (uint32_t)(round(packVoltageFLOAT)); //TODO: DOUBLE CHECK IF THE CASTING WORKS
    }

    return packVoltage;
}



/**
@brief      Function Name: Pack_getVoltageInfo
@details    Function Purpose: Scan the array of voltages of the modules per stacks, and return the min and max voltages

@param      pMinVoltage - pointer to variable to hold minimum voltage found.
@param      pMaxVoltage - pointer to variable to hold maximum voltage found.
@n          .....
@param      pMinStack - pointer to variable to hold INDEX of the battery stack containing the battery module with minimum voltage found.
@param      pMinModule - pointer to variable to hold INDEX of the module with the minimum voltage found.
@n          .....
@param      pMaxStack - pointer to variable to hold INDEX of the battery stack containing the battery module with maximum voltage found.
@param      pMaxModule - pointer to variable to hold INDEX of the module with the maximum voltage found.

@returns    void

@note       Data is stored in variables pointed to. See input.

@par        Algorithm:

    1) Loop one stack and one module at a time.
        Per loop iteration:
        1.1) Retrieve localVoltage.
        1.2) Compare localVoltage to running min and max voltages collected. Replace if needed.
        1.3) If min or max voltages are updated, record stack and module INDICES.
        1.4) Repeat until all modules have been analysed.
    2) Assign final numbers to pointed-to variables.

*/

uint8_t Pack_getSOC()
{
    // StateOfChargeBYTE = pPACKDATA->PH_SOC_LOCATION;
    return pPACKDATA->PH_SOC_LOCATION;
}

uint16_t Pack_getDOD()
{
    return 0;
}
uint16_t Pack_getCapacity()
{
    return 0;
}

voltageInfoStruct * Pack_getVoltageInfo()
{
    uint16_t
        localMinVolt = 65535, //note that the raw register readings are decimal-shifted to avoid storing floating points. 2^16 - 1
        localMaxVolt = 0,
        localVoltage = 0;
    uint8_t
        minStack = 0,
        minModule = 0,
        maxStack = 0,
        maxModule = 0;

  //combines the minVolt and maxVolt loops to reduce redundant struct pulls.
    //localMinVolt = pPACKDATA->stack[0].module[0].voltage; //Meant to avoid edge case where values are not updated from default.
    //localMaxVolt = pPACKDATA->stack[0].module[0].voltage; //Meant to avoid edge case where values are not updated.
    for(int i = 0; i < BTM_NUM_DEVICES; ++i)
    {
        for(int j = 0; j < BTM_NUM_MODULES; ++j)
        {
            if(pPACKDATA -> stack[i].module[j].enable == 1)
            {
                localVoltage = pPACKDATA -> stack[i].module[j].voltage;

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
    voltageInfo.MinVoltage = localMinVolt;
    voltageInfo.MaxVoltage = localMaxVolt;
    voltageInfo.MinStackIndex = minStack;
    voltageInfo.MinModuleIndex = minModule;
    voltageInfo.MaxStackIndex = maxStack;
    voltageInfo.MaxModuleIndex = maxModule;

    return &voltageInfo;
}

voltageInfoStruct * Pack_setVoltageInfo(
    uint16_t MinVoltage,
    uint16_t MaxVoltage,
    uint8_t MinStackIndex,
    uint8_t MinModuleIndex,
    uint8_t MaxStackIndex,
    uint8_t MaxModuleIndex
)
{
    voltageInfo.MinVoltage      = MinVoltage;
    voltageInfo.MaxVoltage      = MaxVoltage;
    voltageInfo.MinStackIndex   = MinStackIndex;
    voltageInfo.MinModuleIndex  = MinModuleIndex;
    voltageInfo.MaxStackIndex   = MaxStackIndex;
    voltageInfo.MaxModuleIndex  = MaxModuleIndex;

    return &voltageInfo;
}


/**
@brief      Function Name: Pack_getTemperatureInfo
@details    Function Purpose: Scan the array of temperature-voltages of the modules per stacks, find the min and max temperature-voltages, and calculate the average temperature.
@note       Note: The voltage measurements are from thermistors, whose resistances vary with temperature.
            This and related functions do NOT return temperature in units of degree Celcius unless it explicitly says it does.

@param      pAverageTemperature - pointer to variable for holding the average temperature-voltage calculated.
@n          .....
@param      pMinVoltage - pointer to variable to hold minimum voltage found.
@param      pMaxVoltage - pointer to variable to hold maximum voltage found.
@n          .....
@param      pMinStack - pointer to variable to hold INDEX of the battery stack containing the battery module with minimum voltage found.
@param      pMinModule - pointer to variable to hold INDEX of the module with the minimum voltage found.
@n          .....
@param      pMaxStack - pointer to variable to hold INDEX of the battery stack containing the battery module with maximum voltage found.
@param      pMaxModule - pointer to variable to hold INDEX of the module with the maximum voltage found.

@par        Algorithm:

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
temperatureInfoStruct * Pack_getTemperatureInfo()
{
    uint16_t
        localTemperature = 0,
        localMinTmp = 65535,
        localMaxTmp = 0;
    uint8_t
        minStack = 255,
        maxStack = 255,
        minModule = 255,
        maxModule = 255;
    double
        temperatureTotal = 0,
        localAverage = 0;

    int
        i = 0,
        total_mux_read = 0; //i have no idea why it's called "mux_read"

    for(i = 0; i < BTM_NUM_DEVICES; ++i)
    {
        for(int j = 0; j < BTM_NUM_MODULES; ++j)
        {
            if(pPACKDATA -> stack[i].module[j].enable == MODULE_ENABLED)
            {
                localTemperature = pPACKDATA -> stack[i].module[j].temperature;

                //double type is used to avoid possible integer overflow.
                //localTemperature is uint16_t. Reasonably, in this loop, it should
                //never add up to a number greater than the max double value.
                //assuming that the thermistors can handle up to the solder melting point,
                //about 300 degree Celcius, the three digits multiplied by 12 is way-way-way
                //lower than the max value of data type double.
                //So, the risk of overflow should be negligible.
                temperatureTotal = temperatureTotal + localTemperature;

                if(localTemperature <= localMinTmp){
                    localMinTmp = localTemperature;
                    minStack = i;
                    minModule = j;
                }

                if(localTemperature >= localMaxTmp){
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

    temperatureInfo.averageTemperature = (uint16_t)localAverage;
    temperatureInfo.minTmp = localMinTmp;
    temperatureInfo.maxTmp = localMaxTmp;
    temperatureInfo.minTmpStackIndex = minStack;
    temperatureInfo.minTmpModuleIndex = minModule;
    temperatureInfo.maxTmpStackIndex = maxStack;
    temperatureInfo.maxTmpModuleIndex = maxModule;

    return &temperatureInfo;
}

temperatureInfoStruct * Pack_setTemperatureInfo(
    uint16_t averageTemperature,
    uint16_t minTmp,
    uint16_t maxTmp,
    uint8_t minTmpStackIndex,
    uint8_t maxTmpStackIndex,
    uint8_t minTmpModuleIndex,
    uint8_t maxTmpModuleIndex
)
{
    temperatureInfo.averageTemperature = averageTemperature;
    temperatureInfo.minTmp = minTmp;
    temperatureInfo.maxTmp = maxTmp;
    temperatureInfo.minTmpStackIndex = minTmpStackIndex;
    temperatureInfo.minTmpModuleIndex = maxTmpStackIndex;
    temperatureInfo.maxTmpStackIndex = minTmpModuleIndex;
    temperatureInfo.maxTmpModuleIndex = maxTmpModuleIndex;

    return &temperatureInfo;
}

/**
@brief      Function Name: Pack_checkAndCastTemperature
@details    Function purpose:
            Check if value is within expected message bounds, and returns the value converted to two's complement.

@param      double temperatureDOUBLE - the temperature in Celcius.

@returns    Tmperature, casted to uint8_t.

@par        Algorithm:

    1) Check if greater-than upper bound.
        If true, return the bound broken.
    2) Check if less-than lower bound.
        If true, return the bound broken.
        Note that the bounds are intentionally set to min and max for int8_t,
        -128 and 127, which should be using two's complement
        (i.e. the first bit is the signed bit)
    3) Cast temperatureDOUBLE to int8_t and assign to temperatureBYTE, then return the value.

@note       We use twos complement to match the Elithion Lithiumate format.
*/
int8_t Pack_checkAndCastTemperature(double temperatureDOUBLE)
{
    int8_t temperatureBYTE;

    if(temperatureDOUBLE > CAN_TEMPERATURE_MAXIMUM)
    {
        temperatureBYTE = CAN_TEMPERATURE_MAXIMUM;
    }
    else if(temperatureDOUBLE < CAN_TEMPERATURE_MINIMUM)
    {
        temperatureBYTE = CAN_TEMPERATURE_MINIMUM;
    }
    else
    {
        temperatureBYTE = (int8_t)temperatureDOUBLE;
    }
    return temperatureBYTE;
}
