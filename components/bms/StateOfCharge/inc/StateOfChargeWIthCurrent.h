#ifndef STATEOFCHARGEWITHCURRENT_H_

#define STATEOFCHARGEWITHCURRENT_H_
#include <stdint.h>

//global variables
float GLOBAL_SOC_previousCurrent = 0;
int GLOBAL_SOC_previousTime = 0;
float GLOBAL_SOC_DoDtotal = 0;

//local variables
#define MIN_VOLTAGE 2.5*32 //voltage of all 32 modules when fully discharged
#define MAX_VOLTAGE 4.2*32 //voltage of all 32 modules when fully charged
#define PH_DISCHARGE_EFFICIENCY 0.85 //according to http://large.stanford.edu/courses/2010/ph240/sun1/
                             //subject to (experimental) change
#define PH_CHARGE_EFFICIENCY 0.85 //according to http://large.stanford.edu/courses/2010/ph240/sun1/
                          //subject to (experimental) change

#endif
