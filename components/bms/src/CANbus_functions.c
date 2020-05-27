#include "CANbus_functions.h"
#include "stm32f3xx_hal.h"
#include <math.h>

//private variables
BTM_PackData_t PH_PACKDATA;

//function prototypes
void CANinfoPullAndFormat();


//function definitions
void CANinfoPullAndFormatSeries623(){
  unsigned int packVoltage = PH_PACKDATA.packvoltage;
  uint16_t MinVtg = 1;
  uint16_t MaxVtg = 1;

  //placeholder code, to see the format of struct calls.
  //Looks really clean
  //retreving voltage of module 12 of stack 3 (note the off-by-one array index)
  uint16_t PH_ModuleVOLTAGE = PH_PACKDATA.stack[2].module[11].voltage;


}
