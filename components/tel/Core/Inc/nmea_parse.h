#ifndef STM32_SERIAL_DMA_NEO6M_PARSE_H
#define STM32_SERIAL_DMA_NEO6M_PARSE_H

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "main.h"


/*******************************************************************************
 * @brief GPS data structure for storing parsed data
 ******************************************************************************/
typedef struct NMEA_DATA {
    float latitude; // Latitude in degrees with decimal places
    char latSide;  // Latitude character 'N' or 'S'
    float longitude; // Longitude in degrees with decimal places
    char lonSide; // Longitude character 'E' or 'W'
    char utcTime[7]; // Time when data is recorded in UTC time
    float altitude; // Altitude in meters
    float geodHeight; // Difference between GPS-measured altitude and the height above the Earth's surface
    float hdop; // Horizontal dilution of precision
    float vdop; // Vertical dilution of precision
    float pdop; // Position dilution of precision
    int satelliteCount; // Number of satellites used in measurement
    int fix; // 1 = fix, 0 = no fix
    char lastMeasure[10]; // hhmmss.ss UTC of last successful measurement; time read from the GPS module
    char date[7]; // Date in UTC time
    float trueHeading; // True heading in degrees
    float magneticHeading; // Magnetic heading in degrees
    float speedKmh; // Speed in km/h
    int snr; // Signal to noise ratio
    int satInView; // Number of satellites in view
    uint8_t RMC_Flag; // 0 or 1 to check for RMC message
} GPS;


/*******************************************************************************
 * @brief Parses NMEA data from the GPS module
 * @param gps_data Pointer to GPS struct, writes data to it
 * @param buffer Pointer to buffer string with NMEA data
 ******************************************************************************/
void nmea_parse(GPS *gps_data, uint8_t *buffer);
#endif //STM32_SERIAL_DMA_NEO6M_PARSE_H