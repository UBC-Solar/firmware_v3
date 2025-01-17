#ifndef STM32_SERIAL_DMA_NEO6M_PARSE_H
#define STM32_SERIAL_DMA_NEO6M_PARSE_H

#include <string.h>
#include <stdlib.h>
#include <stdint.h>


/*******************************************************************************
 * @brief GPS data structure for storing parsed data
 ******************************************************************************/
typedef struct NMEA_DATA {
    float latitude; //latitude in degrees with decimal places
    char latSide;  // N or S
    float longitude; //longitude in degrees with decimal places
    char lonSide; // E or W
    char utcTime[7]; // time when data is recorded in UTC time
    float altitude; // altitude in meters
    float geodHeight; // difference between GPS-measured altitude and the height above the Earth's surface
    float hdop; // horizontal dilution of precision
    float vdop; // vertical dilution of precision
    float pdop; // position dilution of precision
    int satelliteCount; //number of satellites used in measurement
    int fix; // 1 = fix, 0 = no fix
    char lastMeasure[10]; // hhmmss.ss UTC of last successful measurement; time read from the GPS module
    char date[7]; 
    float trueHeading;
    float magneticHeading;
    float speedKmh;
    int snr;
    int satInView;
    uint8_t RMC_Flag; // 0 or 1 to check for RMC message
} GPS;


/*******************************************************************************
 * @brief Parses NMEA data from the GPS module
 * @param gps_data Pointer to GPS struct, writes data to it
 * @param buffer Pointer to buffer string with NMEA data
 ******************************************************************************/
void nmea_parse(GPS *gps_data, uint8_t *buffer);
#endif //STM32_SERIAL_DMA_NEO6M_PARSE_H