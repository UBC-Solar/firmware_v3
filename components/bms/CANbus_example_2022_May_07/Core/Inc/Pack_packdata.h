#ifndef PACK_PACKDATA_H
#define PACK_PACKDATA_H

#define CAN_PACK_MINIMUM (0) //0 kV
#define CAN_PACK_MAXIMUM (65000) //65 kV

#define CAN_TEMPERATURE_MINIMUM (-128) //twos complement; 0x1000'0000
#define CAN_TEMPERATURE_MAXIMUM (127) //twos complement; 0x0111'1111

#include "math.h"
#include "stdint.h"
#include "ltc6813_btm.h"

short Pack_getPackCurrent();
int32_t Pack_getPackVoltage();

void VoltageInfoRetrieval(
    BTM_PackData_t * pPACKDATA,
    uint16_t * pMinVoltage,
    uint16_t * pMaxVoltage,
    uint8_t * pMinStack,
    uint8_t * pMinModule,
    uint8_t * pMaxStack,
    uint8_t * pMaxModule);

void temperatureDataRetrieval(
    BTM_PackData_t * pPACKDATA,
    uint16_t * averageTemperature,
    uint16_t * minTmp,
    uint16_t * maxTmp,
    uint8_t * minTmpStack,
    uint8_t * maxTmpStack,
    uint8_t * minTmpModule,
    uint8_t * maxTmpModule);

uint8_t TwosComplement_TemperatureConverter(double temperatureDOUBLE, uint8_t * outOfBounds);

int32_t outOfBoundsAndCast_packVoltage(float packVoltageFLOAT, uint8_t * outOfBounds);
#endif
