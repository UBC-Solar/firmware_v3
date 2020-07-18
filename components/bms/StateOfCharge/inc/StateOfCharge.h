/*
StateOfCharge.h
*/

#ifndef STATEOFCHARGE_H_
#define STATEOFCHARGE_H_

#define SOC_VOLT_MAX         134.4 //4.2 * 32
#define SOC_VOLT_THRESHOLD_1 116.8  //placeholder: 3.65*32
#define SOC_VOLT_THRESHOLD_2 115.2   //placeholder: 3.6*32
#define SOC_VOLT_THRESHOLD_3 89.6    //placeholder: 2.8*32
#define SOC_VOLT_MIN         80    //2.5*32

#define SOC_PERCENT_THRESHOLD_1 97
#define SOC_PERCENT_THRESHOLD_2 85
#define SOC_PERCENT_THRESHOLD_3 9


//#define HARDCODED_REGION_SLOPES
#ifndef HARDCODED_REGION_SLOPES
global float SOCregion1slope = (SOC_PERCENT_THRESHOLD_1 - 100) / (SOC_VOLT_THRESHOLD_1 - SOC-SOC_VOLT_MAX);
global float SOCregion2slope = (SOC_PERCENT_THRESHOLD_2 - SOC-SOC_PERCENT_THRESHOLD_1) / (SOC_VOLT_THRESHOLD_2 - SOC_VOLT_THRESHOLD_1);
global float SOCregion3slope = (SOC_PERCENT_THRESHOLD_3 - SOC-SOC_PERCENT_THRESHOLD_2) / (SOC_VOLT_THRESHOLD_3 - SOC_VOLT_THRESHOLD_2);
global float SOCregion4slope = (0 - SOC-SOC_PERCENT_THRESHOLD_3) / (SOC_VOLT_MIN - SOC_VOLT_THRESHOLD_3);
#endif

#ifdef HARDCODED_REGION_SLOPES
#define SOCregion1slope 0.183333
#define SOCregion2slope 0.004166
#define SOCregion3slope 0.010526
#define SOCregion4slope 0.033333
#endif

#endif
