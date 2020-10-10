/*
StateOfChargeWithCurrent.c

This file contains functions for calculating the state of charge for UBC Solar's
V3 solar car, Brightside.
*/

#include "StateOfChargeWithCurrent.h"

/**
 * @brief Iniitialize state of charge global variables
 *
 * This function must be called prior to using state of charge functions.
 */
void stateOfChargeWithCurrentInit()
{
    GLOBAL_SOC_DoDtotal = 0; //update DoD globally
    GLOBAL_SOC_previousCurrent = 0; //update current globally
    GLOBAL_SOC_previousTime = 0; //update time globally
}

/*
Function Name: stateOfCharge
Function purpose: takes in voltage and current measurements with a time-stamp and outputs the estimated state
                  of charge in units of percent.

Parameters:
    uint32t voltage100mV :       Voltage as multiples of 100 millivolts.
    int32_t PH_Current_From_CAN: Current from CAN messages converted to Amps
                               Units are for consistency with rest of code.
                               Also, this function will internally
                               convert the voltage to units of volts.
    uint32t PH_time:           time elapsed between each measurement of Current
                               sent through CAN (along with current) or
                               from systick timer in BMS' state machine

Return:
    float SoCpercent : state of charge in units of percent.

    Errors: if DoDinitial greater than 100 then return -1
            if DoDinitial less than 0 then return -2

            if DoDinstantaneous greater than 100 then return -3
            if DoDinstantaneous less than 100 then return -4

            if SoC greater than 100 then return -5
            if SoC less than 0 then return -6

            if SoH greater than 100 then return -7
            if SoH less than 0 then return -8

Design Notes:
    Internally, the function will use floats or double in its calculations.
    The units will be sensible, i.e. SI units of volts.
    The decimal places will be maintained by the return type of float,
    but this can and should be easily changed to an int type if needed.
    If global variables need to be removed, watch out for the declaration of
    variables (ie: DoDinitial)

Algorithm:
    1) Convert current paremeter to units of Amperes (if needed)
    2) Calculate state-of-charge from current.
    3) Check if state-of-charge percent is outside the range of 0% to 100%
    3) Output state-of-charge.
*/

float stateOfChargeWithCurrent(uint32_t voltage100uV, int32_t PH_CurrentFromCAN,
    	                         uint32_t numDischarge, uint32_t PH_time)
{
    float //signed values
        // sign matters
        BalancingCurrent = 0,
        TotalCurrent = 0,
        DoDchange = 0, //percentage
        // should be >0
        voltage = 25000, //assume always positive because voltage100uV is unsiged
        DoDinstantaneous = 0, //percentage
        DoDinitial = GLOBAL_SOC_DoDtotal, //percentage
        SoC = 0, //percentage
        SoH = 100;

    //convert to volts
    //voltage = BTM_regValToVoltage(voltage100uV);
    voltage = voltage100uV * 0.0001; //conversion from 'double' to 'float', possible loss of data


    //calculate state of charge.

    BalancingCurrent = - (float)numDischarge * 0.420; //the discharging current for
    //each cell is 0.420A
    TotalCurrent = (float)PH_CurrentFromCAN + BalancingCurrent;
    SoH = 100; //the battery is in full health the first time is it used
    SoC = 100-GLOBAL_SOC_DoDtotal; //the battery is in full health the first time is it used

    if (PH_CurrentFromCAN < 0) //discharge
     {
       if(voltage > MIN_VOLTAGE)
       {
         DoDchange = calculateChangeDoD(TotalCurrent, PH_time,
                     GLOBAL_SOC_previousCurrent, GLOBAL_SOC_previousTime);
         DoDinstantaneous = DoDinitial + PH_DISCHARGE_EFFICIENCY*DoDchange;
         SoC = SoH - DoDinstantaneous;
       }
       else //voltage <= MIN_VOLTAGE
       {
         SoH = DoDinitial;
       }
     }
     else if(PH_CurrentFromCAN >= 0) //charge
     {
       if(voltage < MAX_VOLTAGE)
       {
         DoDchange = calculateChangeDoD(TotalCurrent, PH_time,
                     GLOBAL_SOC_previousCurrent, GLOBAL_SOC_previousTime);
         DoDinstantaneous = DoDinitial + PH_CHARGE_EFFICIENCY*DoDchange;
         SoC = SoH - DoDinstantaneous;
       }
       else //voltage >= MAX_VOLTAGE
       {
         SoH = SoC;
       }
     }

        GLOBAL_SOC_DoDtotal = DoDinstantaneous; //update DoD globally
        GLOBAL_SOC_previousCurrent = TotalCurrent; //update current globally
        GLOBAL_SOC_previousTime = PH_time; //update time globally

    /*
    if for some reason the state of charge, initial and instantaneous depth of
    discharge exceeds the design limits of 0% to 100% then return an error code
    */

  //  if(DoDinitial > 100)
  //  {
  //      return -1;
  //  }
  //  else if(DoDinitial < 0)
  //  {
  //      return -2; // this can occur when there are more modules discharging more current than what the battery's charging current -> check calculation of current
  //  }

  //  if(DoDinstantaneous > 100)
  //  {
  //      return -3;
  //  }
  //  else if(DoDinstantaneous < 0)
  //  {
  //      return -4;
  //  }

  //if(SoC > 100)
  //{
  //    return -5;
  //}
  //else if(SoC < 0)
  //{
  //    return -6;
  //}

  //if(SoH > 100)
  //{
  //    return -7;
  //}
  //else if(SoH < 0)
  //{
  //    return -8;
  //}

return SoC;
}

float calculateChangeDoD(float presentCurrent, float presentTime,
                          float pastCurrent, float pastTime)
                          //function called in stateOfChargeWithCurrent above
{
    float //signed value
      ChangeDoD = 0;

      ChangeDoD = (-(presentCurrent+pastCurrent)/2*(presentTime-pastTime)/1000 ) // divide time by 1, 000 to convert to s
                  / (3.5*3600*32)*100; // EDIT: PLEASE VERIFY AND DEFINE CONSTANTS FOR THIS

      return ChangeDoD;
}
