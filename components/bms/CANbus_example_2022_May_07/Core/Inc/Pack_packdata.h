#ifndef PACK_PACKDATA_H
#define PACK_PACKDATA_H

#define CAN_PACK_MINIMUM (0) //0 kV
#define CAN_PACK_MAXIMUM (65000) //65 kV

#define CAN_TEMPERATURE_MINIMUM (-128) //twos complement; 0x1000'0000
#define CAN_TEMPERATURE_MAXIMUM (127) //twos complement; 0x0111'1111

#include "math.h"
#include "stdint.h"
#include "ltc6813_btm.h"

typedef struct voltageInfoStruct{
    uint16_t MinVoltage;
    uint16_t MaxVoltage;
    uint8_t MinStackIndex;
    uint8_t MinModuleIndex;
    uint8_t MaxStackIndex;
    uint8_t MaxModuleIndex;
}voltageInfoStruct;

typedef struct temperatureInfoStruct{
    int16_t averageTemperature;
    int16_t minTmp;
    int16_t maxTmp;
    uint8_t minTmpStackIndex;
    uint8_t minTmpModuleIndex;
    uint8_t maxTmpStackIndex;
    uint8_t maxTmpModuleIndex;
}temperatureInfoStruct;

extern voltageInfoStruct voltageInfo;

extern temperatureInfoStruct temperatureInfo;

void Pack_initPackdataPtr(BTM_PackData_t * ptr);

short Pack_getPackCurrent();
uint32_t Pack_getPackVoltage();
uint64_t Pack_getFaultAndWarningStatusBits();
uint8_t Pack_getSOC();
uint16_t Pack_getDOD();
uint16_t Pack_getCapacity();

voltageInfoStruct * Pack_getVoltageInfo();

voltageInfoStruct * Pack_setVoltageInfo(
    uint16_t MinVoltage,
    uint16_t MaxVoltage,
    uint8_t MinStackIndex,
    uint8_t MinModuleIndex,
    uint8_t MaxStackIndex,
    uint8_t MaxModuleIndex
);

temperatureInfoStruct * Pack_getTemperatureInfo();
temperatureInfoStruct * Pack_setTemperatureInfo(
    int16_t averageTemperature,
    int16_t minTmp,
    int16_t maxTmp,
    uint8_t minTmpStackIndex,
    uint8_t minTmpModuleIndex,
    uint8_t maxTmpStackIndex,
    uint8_t maxTmpModuleIndex
);

int8_t Pack_checkAndCastTemperature(double temperatureDOUBLE);

#endif
