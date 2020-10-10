#ifndef STATEOFCHARGEWITHCURRENT_H_
#define STATEOFCHARGEWITHCURRENT_H_

#include <stdint.h>

/*============================================================================*/
/* PUBLIC VARAIBLES */

float GLOBAL_SOC_previousCurrent;
int GLOBAL_SOC_previousTime;
float GLOBAL_SOC_DoDtotal;

/*============================================================================*/
/* PUBLIC CONSTANTS */

#define MIN_VOLTAGE (2.5*32) //voltage of all 32 modules when fully discharged
#define MAX_VOLTAGE (4.2*32) //voltage of all 32 modules when fully charged
#define PH_DISCHARGE_EFFICIENCY 0.85 //according to http://large.stanford.edu/courses/2010/ph240/sun1/
                             //subject to (experimental) change
#define PH_CHARGE_EFFICIENCY 0.85 //according to http://large.stanford.edu/courses/2010/ph240/sun1/
                          //subject to (experimental) change
#define RATED_CAPACITY (3.5*3600*13) // the capacity of each cell is
                                     //3.5Ah=3.5C/s*h=3.5C * 3600s/h.
                                     //There are 13cells in parallel per modules
                                     // the 32 modules do not add in capacity.


/*============================================================================*/
/* FUNCTION PROTOTYPES */

//functions require calling each time a CAN transmits a new current to update SoC
void stateOfChargeWithCurrentInit(void);
float stateOfChargeWithCurrent(uint32_t voltage100uV, int32_t PH_CurrentFromCAN,
                               uint32_t numDischarge, uint32_t PH_time);
float calculateChangeDoD(float presentCurrent, float presentTime,
                         float pastCurrent, float pastTime);
                         //function called in stateOfChargeWithCurrent above

#endif /* STATEOFCHARGEWITHCURRENT_H_ */
