/******************************************************************************
* @file    nmea_parse.c
* @brief   Parses GPS message and extracting the fields to the GPS struct
******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "nmea_parse.h"

#define NMEA_MAX_SENTENCE 140
#define GPGGA_MAX_VALUES  12
#define GPGSA_MAX_VALUES  18
#define GPGLL_MAX_VALUES  5
#define GPRMC_MAX_VALUES  10
#define GPVTG_MAX_VALUES  8
#define GPGSV_MAX_VALUES  8

char *data[20];

/**
 * @brief Splits a string by commas creating new individual strings within an array
 * 
 * This function perserves empty fields when tokenizing strings compared to the
 * string function 'strtok' which skips over these fields resulting in the loss
 * of data.
 *
 * @param sentence The input string to split
 * @param tokens The array to store the split tokens
 * @param max_value The maximum number of tokens to extract
 */
void split_commas(char *sentence, char *tokens[], int max_value) {
    int count = 0;
    char *pos = sentence;
    while (pos && (count < max_value)) {
        char *comma = strchr(pos, ',');
        if (comma) *comma = '\0';
        tokens[count++] = pos;
        pos = comma ? (comma + 1) : NULL;
    }
    if (count < max_value) tokens[count] = NULL;
}


/**
 * @brief Copies a string from source to output up to a specified size
 * 
 * This function ensures additional sentences are not parsed by copying
 * until the line "\r\n" is reached or the specified size if the line
 * ending is not found.
 *
 * @param output The destination buffer for the copied string
 * @param source The source string to copy from
 * @param size The maximum number of bytes to copy
 */
void copy_sentence(char *output, const char *source, size_t size) {

    if(!output || size == 0 || !source) return;

    const char *end = strstr(source, "\r\n");

    size_t length = end ? (size_t)(end - source) : strnlen(source, size - 1);

    if (length >= size) length = size - 1;

    memcpy(output, source, length);
    output[length] = '\0';
}

/**
 * @brief Validates the checksum
 *
 * Extracts the checksum from the received NMEA sentence 
 * and compares it against the computed checksum for verification.
 * 
 * @param nmea_data Pointer to the checksum string
 * @return returns true for success, false for failure
 */
int gps_checksum(char *nmea_data)
{
    // Pointing a string with less than 5 characters the function will read outside of scope and crash the mcu.
    if(strlen(nmea_data) < 5) return 0;
    char recv_crc[2];
    recv_crc[0] = nmea_data[strlen(nmea_data) - 4];
    recv_crc[1] = nmea_data[strlen(nmea_data) - 3];
    int crc = 0;
    int i;

    // Exclude the CRLF plus CRC with an * from the end
    for (i = 0; i < strlen(nmea_data) - 5; i ++) {
        crc ^= nmea_data[i];
    }
    int receivedHash = strtol(recv_crc, NULL, 16);
    if (crc == receivedHash) {
        return 1; // Success
    }
    else{
        return 0; // Failure
    }
}


/**
 * @brief Parses strings starting with "GPGGA" or "GNGGA"
 *
 * Extracts the following variables: longitude, latitude, lonSide, latSide, 
 * altitude, geodHeight, and fix from the string and creating a pointer
 * for each to the struct gps_data.
 * 
 * @param gps_data Pointer to struct gps_data with all GPS fields
 * @param inputString Pointer to the string beginning with GPGGA or GNGGA
 * @return returns true for success, false for failure
 */
int nmea_GPGGA(GPS *gps_data, char*inputString) 
{
    char sentence[NMEA_MAX_SENTENCE];
    // strncpy(line, inputString, sizeof(line)-1);
    // line[sizeof(line)-1] = '\0';

    copy_sentence(sentence, inputString, sizeof(sentence));

    char *values[GPGGA_MAX_VALUES];
    memset(values, 0, sizeof(values));

    split_commas(sentence, values, GPGGA_MAX_VALUES);

    if (values[1] && values[1][0] != '\0') 
    {
        strncpy(gps_data->utcTime, values[1], sizeof(gps_data->utcTime) - 1);
        gps_data->utcTime[sizeof(gps_data->utcTime) - 1] = '\0';
    } else {
        gps_data->utcTime[0] = '\0';
    }

    // -- Currently not using these following fields - altitude, geodHeight, satelliteCount, fixQuality -- //

    // float altitude = values[9] ? strtof(values[9], NULL) : 0.0f;
    // gps_data->altitude = altitude!=0 ? altitude : gps_data->altitude;

    // float geodHeight = values[11] ? strtof(values[11], NULL) : 0.0f;
    // gps_data->geodHeight = geodHeight!=0 ? geodHeight : gps_data->geodHeight;

    // gps_data->satelliteCount = values[7] ? strtol(values[7], NULL, 10) : gps_data->satelliteCount;

    // int fixQuality = values[6] ? strtol(values[6], NULL, 10) : 0;
    // gps_data->fix = fixQuality > 0 ? 1 : 0;

    // Extract direction indicators for longitude and latitude
    char lonSide = values[5] ? values[5][0] : '\0';
    char latSide = values[3] ? values[3][0] : '\0';

    if (values[1]) strcpy(gps_data->lastMeasure, values[1]);

    if(latSide == 'S' || latSide == 'N')
    {
        char lat_d[3];
        char lat_m[8];
        for (int z = 0; z < 2 && values[2] && values[2][z]; z++) lat_d[z] = values[2][z]; 
        lat_d[2] = '\0';
        for (int z = 0; z < 6 && values[2] && values[2][z + 2]; z++) lat_m[z] = values[2][z + 2];
        lat_m[6] = '\0';

        // Convert latitude degrees and minutes to decimal format
        int lat_deg_strtol = strtol(lat_d, NULL, 10);
        float lat_min_strtof = strtof(lat_m, NULL);
        double lat_deg = lat_deg_strtol + lat_min_strtof / 60.0;

        char lon_d[4];
        char lon_m[8];

        for (int z = 0; z < 3 && values[4] && values[4][z]; z++) lon_d[z] = values[4][z]; 
        lon_d[3] = '\0';
        for (int z = 0; z < 6 && values[4] && values[4][z + 3]; z++) lon_m[z] = values[4][z + 3];
        lon_m[6] = '\0';

        // Convert longitude degrees and minutes to decimal format
        int lon_deg_strtol = strtol(lon_d, NULL, 10);
        float lon_min_strtof = strtof(lon_m, NULL);
        double lon_deg = lon_deg_strtol + lon_min_strtof / 60.0;

        // Validate parsed latitude and longitude values
        if(lat_deg != 0 && lon_deg != 0 && lat_deg < 90 && lon_deg < 180)
        {
            // Extact and store values to gps_data
            gps_data->latitude = lat_deg;
            gps_data->latSide = latSide;
            gps_data->longitude = lon_deg;
            gps_data->lonSide = lonSide;
        }
        else
        {
            return 0; // Failure
        }
    }

    return 1; // Success
}


/**
 * @brief Parses strings starting with "GPGSA" or "GNGSA"
 *
 * Extracts the following variables: satelliteCount, pdop, hdop, vdop
 * and fix and creating a pointer for each to the struct gps_data.
 * 
 * @param gps_data Pointer to the struct gps_data with all GPS fields
 * @param inputString Pointer to the string beginning with GPGSA or GNGSA
 * @return returns true for success
 */
int nmea_GPGSA(GPS *gps_data, char* inputString) 
{
    char sentence[NMEA_MAX_SENTENCE];
    // strncpy(line, inputString, sizeof(line)-1);
    // line[sizeof(line)-1] = '\0';

    copy_sentence(sentence, inputString, sizeof(sentence));

    char *values[GPGSA_MAX_VALUES];
    memset(values, 0, sizeof(values));

    split_commas(sentence, values, GPGSA_MAX_VALUES);

    int fix = values[2] ? strtol(values[2], NULL, 10) : 0;
    gps_data->fix = fix > 1 ? 1 : 0;
    int satelliteCount = 0;

    for(int i = 3; i < 15; i++)
    {
        if(values[i] && values[i][0] != '\0')
        {
            satelliteCount++;
        }
    }

    gps_data->satelliteCount = satelliteCount;

    // -- Currently not using these following fields - pdop, hdop, vdop -- //

    // float pdop = values[15] ? strtof(values[15], NULL) : 0.0f;
    // gps_data->pdop = pdop!=0.0 ? pdop : gps_data->pdop;

    // float hdop = values[16] ? strtof(values[16], NULL) : 0.0f;
    // gps_data->hdop = hdop!=0.0 ? hdop : gps_data->hdop;

    // float vdop = values[17] ? strtof(values[17], NULL) : 0.0f;
    // gps_data->vdop = vdop!=0.0 ? vdop : gps_data->vdop;

    return 1; // Success
}


/**
 * @brief Parses strings starting with "GPGLL" or "GNGLL"
 *
 * Extracts the following variables: latitude, longitude, lonSide and latSide
 * and creating a pointer for each to the struct gps_data. The parsing is the
 * same as the parsing in GPGGA for the longitude and latitude variables.
 * 
 * @param gps_data Pointer to the struct gps_data with all GPS fields
 * @param inputString Pointer to the string beginning with GPGLL or GNGLL
 * @return returns true for success, false for failure
 */
int nmea_GPGLL(GPS *gps_data, char*inputString)
{
    char sentence[NMEA_MAX_SENTENCE];
    // strncpy(line, inputString, sizeof(line)-1);
    // line[sizeof(line)-1] = '\0';

    copy_sentence(sentence, inputString, sizeof(sentence));

    char *values[GPGLL_MAX_VALUES];
    memset(values, 0, sizeof(values));

    split_commas(sentence, values, GPGLL_MAX_VALUES);

    char latSide = values[2] ? values[2][0] : '\0';
    if (latSide == 'S' || latSide == 'N') 
    {
        char lat_d[3];
        char lat_m[8];
        for (int z = 0; z < 2 && values[1] && values[1][z]; z++) lat_d[z] = values[1][z];
        lat_d[2] = '\0';
        for (int z = 0; z < 6 && values[1] && values[1][z + 2]; z++) lat_m[z] = values[1][z + 2];
        lat_m[6] = '\0';

        int lat_deg_strtol = strtol(lat_d, NULL, 10);
        float lat_min_strtof = strtof(lat_m, NULL);
        float lat_deg = lat_deg_strtol + lat_min_strtof / 60.0f;

        char lon_d[4];
        char lon_m[8];
        char lonSide = values[4] ? values[4][0] : '\0';
        for (int z = 0; z < 3 && values[3] && values[3][z]; z++) lon_d[z] = values[3][z];
        lon_d[3] = '\0';
        for (int z = 0; z < 6 && values[3] && values[3][z + 3]; z++) lon_m[z] = values[3][z + 3];
        lon_m[6] = '\0';

        // Convert longitude degrees and minutes to decimal format
        int lon_deg_strtol = strtol(lon_d, NULL, 10);
        float lon_min_strtof = strtof(lon_m, NULL);
        float lon_deg = lon_deg_strtol + lon_min_strtof / 60.0f;

        // Validate parsed latitude and longitude values
        if(lon_deg_strtol == 0 || lon_min_strtof == 0 || lat_deg_strtol == 0 || lat_min_strtof == 0)
        {
            return 0;

        } else {
            // Store values to gps_data
            gps_data->latitude = lat_deg;
            gps_data->longitude = lon_deg;
            gps_data->latSide = latSide;
            gps_data->lonSide = lonSide;
            return 1; // Success
        }
    }
    else return 0; // Failure
}


/**
 * @brief Parses strings starting with "GPRMC" or "GNRMC"
 *
 * Extracts the following variables: date and RMC_Flag and creating a pointer 
 * for each to the struct gps_data.
 * 
 * @param gps_data Pointer to the struct gps_data with all GPS fields
 * @param inputString Pointer to the string beginning with GPRMC or GNGRMC
 * @return returns true for success, false for failure
 */
int nmea_GPRMC(GPS *gps_data, char* inputString) 
{
    char sentence[NMEA_MAX_SENTENCE];
    // strncpy(line, inputString, sizeof(line)-1);
    // line[sizeof(line)-1] = '\0';

    copy_sentence(sentence, inputString, sizeof(sentence));

    char *values[GPRMC_MAX_VALUES];
    memset(values, 0, sizeof(values));

    split_commas(sentence, values, GPRMC_MAX_VALUES);

    // -- Currently not using these following fields - date -- //
    
    // if (values[9] && strlen(values[9]) == 6) 
    // {
    //     strncpy(gps_data->date, values[9], 6);
    //     gps_data->date[6] = '\0';
    //     gps_data->RMC_Flag = 1;
    //     return 1; // Success
    // } else {
    //     return 0; // Failure
    // }
}


/**
 * @brief Parses strings starting with "GPVTG" or "GNVTG"
 *
 * Extracts the following variables: trueHeading, magneticHeading, speedKmh
 * and creating a pointer for each to the struct gps_data.
 * 
 * @param gps_data Pointer to the struct gps_data with all GPS fields
 * @param inputString Pointer to the string beginning with GPVTG or GNVTG
 * @return returns true for success
 */
int nmea_GPVTG(GPS *gps_data, char* inputString) 
{
    char sentence[NMEA_MAX_SENTENCE];
    // strncpy(line, inputString, sizeof(line)-1);
    // line[sizeof(line)-1] = '\0';

    copy_sentence(sentence, inputString, sizeof(sentence));

    char *values[GPVTG_MAX_VALUES];
    memset(values, 0, sizeof(values));

    split_commas(sentence, values, GPVTG_MAX_VALUES);

    // -- Currently not using these following fields - trueHeading, magneticHeading, speedKmh -- //
    // float trueHeading = values[1] ? strtof(values[1], NULL) : 0.0f;
    // gps_data->trueHeading = trueHeading!=0 ? trueHeading : gps_data->trueHeading;

    // float magneticHeading = values[3] ? strtof(values[3], NULL) : 0.0f;
    // gps_data->magneticHeading = magneticHeading!=0 ? magneticHeading : gps_data->magneticHeading;

    // float speedKmh = values[7] ? strtof(values[7], NULL) : 0.0f;
    // gps_data->speedKmh = speedKmh!=0 ? speedKmh : gps_data->speedKmh; 

    return 1;
}


/**
 * @brief Parses strings starting with "GPGSV" or "GNGSV"
 *
 * Extracts the following variables: snr, satInView and creating a 
 * pointer for each to the struct gps_data.
 * 
 * @param gps_data Pointer to the struct gps_data with all GPS fields
 * @param inputString Pointer to the string beginning with GPGSV or GNGSV
 * @return returns true for success, false for failure
 */
int nmea_GPGSV(GPS *gps_data, char* inputString)
{
    char sentence[NMEA_MAX_SENTENCE];
    // strncpy(line, inputString, sizeof(line)-1);
    // line[sizeof(line)-1] = '\0';

    copy_sentence(sentence, inputString, sizeof(sentence));

    char *values[GPGSV_MAX_VALUES];
    memset(values, 0, sizeof(values));

    split_commas(sentence, values, GPGSV_MAX_VALUES);

    // Extact and store values to gps_data
    int snr = values[7] ? strtol(values[7], NULL, 10) : 0;
    gps_data->snr = snr!=0 ? snr : gps_data->snr;

    int satInView = values[3] ? strtol(values[3], NULL, 10) : 0;
    gps_data->satInView = satInView!=0 ? satInView : gps_data->satInView;

    return 1;
}


/**
 * @brief Organizies parsing by field from given GPS message
 *
 * Searches for instances of a GPS field in the buffer containing
 * all the GPS fields and organizes parsing by calling seperate
 * functions responsible for parsing each respective field.
 * 
 * @param gps_data The struct gps_data with all GPS fields
 * @param buffer Buffer containing all GPS messages
 */
void nmea_parse(GPS *gps_data, uint8_t *buffer) {
    
    memset(data, 0, sizeof(data)); // Clear array

    int cnt = 0;
    char *token = strtok((char *)buffer, "$");

    while (token && cnt < 20) {
        data[cnt++] = token;
        token = strtok(NULL, "$");
    }

    for(int i = 0; i < cnt; i++)
    {
        if(strstr(data[i], "\r\n") != NULL) // Check if the sentence is complete with a newline
        {
            // Identify sentence type and call corresponding parsing function
            if(strstr(data[i], "GNGLL")!=NULL || (strstr(data[i], "GPGLL"))!=NULL)
            {
               nmea_GPGLL(gps_data, data[i]);
            }
            else if(strstr(data[i], "GNGSA")!= NULL || (strstr(data[i], "GPGSA"))!=NULL)
            {
                nmea_GPGSA(gps_data, data[i]);
            }
            else if(strstr(data[i], "GNGGA") != NULL || (strstr(data[i], "GPGGA"))!=NULL)
            {
                nmea_GPGGA(gps_data, data[i]);
            }
            else if(strstr(data[i], "GNRMC") != NULL || (strstr(data[i], "GPRMC"))!=NULL)
            {
	            nmea_GPRMC(gps_data, data[i]);
	        }
            else if(strstr(data[i], "GNVTG") != NULL || (strstr(data[i], "GPVTG"))!=NULL)
            {
                nmea_GPVTG(gps_data, data[i]);
            }
            else if(strstr(data[i], "GNGSV") != NULL || (strstr(data[i], "GPGSV"))!=NULL)
            {
	            nmea_GPGSV(gps_data, data[i]);
            }
        }
    }
}
