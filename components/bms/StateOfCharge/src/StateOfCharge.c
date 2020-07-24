/*
StateOfCharge.c

This file contains functions for calculating the state of charge for UBC Solar's V3 solar car, Brightside.
*/

/*
Function Name: stateOfCharge
Function purpose: takes in voltage measurements and outputs the estimated state of charge in units of percent.

Parameters:
    uint32t voltage100mV : Voltage as multiples of 100 millivolts.
                           Units are for consistency with rest of code.
                           Also, this function will internally
                           convert the voltage to units of volts.

Return:
    float SoCpercent : state of charge in units of percent.

Design Notes:
    Internally, the function will use floats or double in its calculations.
    The units will be sensible, i.e. SI units of volts.
    The decimal places will be maintained by the return type of float,
    but this can and should be easily changed to an int type if needed.

Algorithm:
    1) Convert voltage paremeter to units of volts (if needed)
    2) Calculate state-of-charge from voltage.
    3) Check if state-of-charge percent is outside the range of 0% to 100%
    3) Output state-of-charge.
*/
#include "StateOfCharge.h"

#ifdef STATEOFCHARGE_DEBUG
void main()
{
    return;
}
#endif

float stateOfCharge(uint32_t voltage100uV);

float stateOfCharge(uint32_t voltage100uV)
{
    float
        voltage,
        SoCpercent;

    //convert to volts
    //voltage = BTM_regValToVoltage(voltage100uV);
    voltage = voltage100uV * 0.0001;

    //calculate state of charge.
    if(voltage > SOC_VOLT_MAX)
    {
        SoCpercent = 100;
    }
    else if(voltage > SOC_VOLT_THRESHOLD_1)
    {
        SoCpercent = SOCregion1slope * (voltage - SOC_VOLT_MAX) + 100;
    }
    else if(voltage > SOC_VOLT_THRESHOLD_2)
    {
        SoCpercent = SOCregion2slope * (voltage - SOC_VOLT_THRESHOLD_1) + SOC_PERCENT_THRESHOLD_1;
    }
    else if(voltage > SOC_VOLT_THRESHOLD_3)
    {
        SoCpercent = SOCregion3slope * (voltage - SOC_VOLT_THRESHOLD_2) + SOC_PERCENT_THRESHOLD_2;
    }
    else if(voltage > SOC_VOLT_MIN)
    {
        SoCpercent = SOCregion4slope * (voltage - SOC_VOLT_THRESHOLD_3) + SOC_PERCENT_THRESHOLD_3;
    }
    else //if(voltage <= SOC_VOLT_MIN)
    {
        SoCpercent = 0;
    }

    /*
    if for some reason the state of charge percent exceeds
    the design limits of 0% to 100%, this function will return
    +1337 if it exceeds 100%, else it will return -1337 if it
    is below 0%.
    */
    if(SoCpercent > 100)
    {
        return 1337;
    }
    else if(SoCpercent < 0)
    {
        return -1337;
    }

    return SoCpercent;
}
