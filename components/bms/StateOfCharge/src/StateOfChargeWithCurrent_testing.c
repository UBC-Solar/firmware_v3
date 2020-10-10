/*

...

*/

#define _CRT_SECURE_NO_WARNINGS
#include "StateOfChargeWithCurrent.h" //watch out if the file is "missing" from a branch in Github

#include <stdio.h>
#include <stdint.h>

//test     inputs and outputs of function calculating State of Charge by Coulomb counting
//float    stateOfChargeWithCurrent(uint32_t voltage100uV, uint32_t PH_CurrentFromCAN,
//uint32_t numDischarge, uint32_t PH_time);

void loopStateOfChargeWithCurrent(int Initial, int Final, int increment, char variable, int timeIncrement,
                                  int defaultVoltage, int defaultCurrent, int defaultdischargingModules);
                                  //change timeIncrement (in ms) and default values between tests

int main()
{
    int i;
    uint32_t Voltage = 2.5; //in V /min voltage of a cell when battery is completely discharged
    uint32_t Current = 0; //in A /min current of a cell when battery is disconnected
    uint32_t time = 0;
    uint32_t dischargingModules = 0; //min number of modules discharging during balancing

    stateOfChargeWithCurrentInit();
    loopStateOfChargeWithCurrent( 23000, 44000, 1000, 'V', 250, 30000, 5, 16); // Voltage / every 250ms
    printf("\n");

    stateOfChargeWithCurrentInit();
    loopStateOfChargeWithCurrent( -12  , 12   , 1   , 'I', 250, 30000, 5, 16); // Current / every 250ms
    printf("\n");

    stateOfChargeWithCurrentInit();
    loopStateOfChargeWithCurrent( 0    , 32   , 1   , 'd', 250, 30000, 5, 16); //number of discharging modules / every 250ms
    printf("\n");
    return 0;
}

//change timeIncrement (in ms) and default values between tests
void loopStateOfChargeWithCurrent(int Initial, int Final, int increment, char variable, int timeIncrement,
                                  int defaultVoltage, int defaultCurrent, int defaultdischargingModules)
{
    int quantity;
    int time = 0;
    float SoC=20;
    for (quantity = Initial; quantity < Final; quantity = quantity + increment)
    {


        //check which variable is changing and make other two varaibles constants
        if (variable == 'V')
        {
            SoC = stateOfChargeWithCurrent(quantity, defaultCurrent, defaultdischargingModules, time);
        }
        else if (variable == 'I')
        {
            SoC = stateOfChargeWithCurrent(defaultVoltage, quantity, defaultdischargingModules, time);
        }
        else if (variable == 'd')
        {
            SoC = stateOfChargeWithCurrent(defaultVoltage, defaultCurrent, quantity, time);
        }
        else
        {
            printf("Variable is invalid. Variable = V, I, or d \r\n");
        }

        printf("for %c = %i, SoC = %f \r\n", variable, quantity, SoC);
        time = time + timeIncrement;
    }

}
