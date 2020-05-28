#include "CANbus_functions.h"
#include "stm32f3xx_hal.h"
#include <math.h>

//private variables
BTM_PackData_t PH_PACKDATA;

//function prototypes
void CANinfoPullAndFormatSeries623();
void VoltageComparator(uint16_t * pMinVoltage, uint16_t * pMaxVoltage);
void packVoltageEncoder(unsigned int pack_voltage);


//function definitions
void CANinfoPullAndFormatSeries623(){
  unsigned int packVoltage = 0;
  uint16_t MinVtg = 0;
  uint16_t MaxVtg = 0;
  uint8_t aData_series623[8] = { 0 };

  //placeholder code, to see the format of struct calls.
  //Looks really clean
  //retreving voltage of module 12 of stack 3 (note the off-by-one array index)
  uint16_t PH_ModuleVOLTAGE = PH_PACKDATA.stack[2].module[11].voltage;

  //gather min and max voltages
  VoltageComparator(&MinVtg, &MaxVtg);

  /**
  pack Voltage
  format:
    unsigned, 0 to 65 kV -> ASSUMING decimal values from 0 to 65 000
    because it's alloted 2 bytes in the data frame
  */
  packVoltage = packVoltageEncoder(PH_PACKDATA.pack_voltage);

}

void VoltageComparator(uint16_t * pMinVoltage, uint16_t * pMaxVoltage){
  uint16_t
    localMinVolt = 1000000, //note that the raw register readings are decimal-shifted to avoid storing floating points
    localMaxVolt = 0;

  //combines the minVolt and maxVolt loops to reduce redundant struct pulls.
  for(int i = 0; i < BTM_NUM_DEVICES; i++){
    for(int j = 0; j < BTM_NUM_MODULES; j++){

      localVoltage = PH_PACKDATA.stack[i].module[j].voltage;

      if(localVoltage < localMinVolt)
        localMinVolt = localVoltage;

      if(localVoltage > localMaxVolt)
        localMaxVolt = localVoltage;
    }
  }

  //"return" min and max voltage
  *pMinVoltage = localMinVolt;
  *pMaxVoltage = localMaxVolt;

}

unsigned int packVoltageEncoder(pack_voltage){
  return 1337;
}
