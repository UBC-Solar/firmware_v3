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

char *data[20];

/**
 * @brief Alidates the checksum
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
    char *values[25];
    int counter = 0;
    memset(values, 0, sizeof(values));
    char *marker = strtok(inputString, ",");  // Tokenize the input string using ',' as a delimiter

    /* Dynamically allocated */

    // while (marker != NULL) 
    // {
    //     values[counter++] = malloc(strlen(marker) + 1);
    //     strcpy(values[counter - 1], marker);
    //     marker = strtok(NULL, ",");
    // }

    /* Statically allocated */

    while(counter < 25 && marker != NULL) {
        values[counter++] = marker;
        marker = strtok(NULL, ",");
    }

    // Extract direction indicators for longitude and latitude
    char lonSide = values[5][0];
    char latSide = values[3][0];

    strcpy(gps_data->lastMeasure, values[1]);
    if(latSide == 'S' || latSide == 'N')
    {
        char lat_d[2];
        char lat_m[7];
        for (int z = 0; z < 2; z++) lat_d[z] = values[2][z]; // Extract latitude degrees
        for (int z = 0; z < 6; z++) lat_m[z] = values[2][z + 2]; // Extract latitude minutes

        // Convert latitude degrees and minutes to decimal format
        int lat_deg_strtol = strtol(lat_d, NULL, 10);
        float lat_min_strtof = strtof(lat_m, NULL);
        double lat_deg = lat_deg_strtol + lat_min_strtof / 60;

        char lon_d[3];
        char lon_m[7];

        for (int z = 0; z < 3; z++) lon_d[z] = values[4][z]; // Extract longitude degrees
        for (int z = 0; z < 6; z++) lon_m[z] = values[4][z + 3]; // Extract longitude minutes

        // Convert longitude degrees and minutes to decimal format
        int lon_deg_strtol = strtol(lon_d, NULL, 10);
        float lon_min_strtof = strtof(lon_m, NULL);
        double lon_deg = lon_deg_strtol + lon_min_strtof / 60;

        // Validate parsed latitude and longitude values
        if(lat_deg != 0 && lon_deg != 0 && lat_deg < 90 && lon_deg < 180)
        {
            // Extact and store values to gps_data
            gps_data->latitude = lat_deg;
            gps_data->latSide = latSide;
            gps_data->longitude = lon_deg;
            gps_data->lonSide = lonSide;

            float altitude = strtof(values[9], NULL);
            gps_data->altitude = altitude!=0 ? altitude : gps_data->altitude;

            float geodHeight = strtof(values[11], NULL);
            gps_data->geodHeight = geodHeight!=0 ? geodHeight : gps_data->geodHeight;

            gps_data->satelliteCount = strtol(values[7], NULL, 10);

            int fixQuality = strtol(values[6], NULL, 10);
            gps_data->fix = fixQuality > 0 ? 1 : 0;

            if (values[1][0] != '\0') 
            {
                strncpy(gps_data->utcTime, values[1], sizeof(gps_data->utcTime) - 1);
                gps_data->utcTime[sizeof(gps_data->utcTime) - 1] = '\0';
            } else {
                gps_data->utcTime[0] = '\0';
            }
        } else {
            // for(int i=0; i<counter; i++) free(values[i]);
            return 0; // Failure
        }
    }

    // for(int i = 0; i < counter; i++) free(values[i]);
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
    char *values[25];
    int counter = 0;
    memset(values, 0, sizeof(values));
    char *marker = inputString;
    char *token;

    while ((token = strsep(&marker, ",")) != NULL) // Tokenize the input string using ',' as a delimiter
    {
        values[counter++] = strdup(token);
    }
    
    int fix = strtol(values[2], NULL, 10);
    gps_data->fix = fix > 1 ? 1 : 0;
    int satelliteCount = 0;

    for(int i = 3; i < 15; i++)
    {
        if(values[i][0] != '\0')
        {
            satelliteCount++;
        }
    }

    // Extact and store values to gps_data
    gps_data->satelliteCount = satelliteCount;

    float pdop = strtof(values[15], NULL);
    gps_data->pdop = pdop!=0.0 ? pdop : gps_data->pdop;

    float hdop = strtof(values[16], NULL);
    gps_data->hdop = hdop!=0.0 ? hdop : gps_data->hdop;

    float vdop = strtof(values[17], NULL);
    gps_data->vdop = vdop!=0.0 ? vdop : gps_data->vdop;

    for(int i = 0; i < counter; i++) free(values[i]);

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
    char *values[25];
    int counter = 0;
    
    char *marker = strtok(inputString, ","); // Tokenize the input string using ',' as a delimiter

    /* Orginal code using dynamic memory allocation to store tokens */

    // while (marker != NULL) 
    // {
    //     values[counter++] = malloc(strlen(marker) + 1); 
    //     strcpy(values[counter - 1], marker);
    //     marker = strtok(NULL, ",");
    // }

    /* Alternative code using static memory allocation */

    while (counter < 25 && marker != NULL) {
        values[counter++] = marker;
        marker = strtok(NULL, ",");
    }

    char latSide = values[2][0];
    if (latSide == 'S' || latSide == 'N') 
    {
        char lat_d[2];
        char lat_m[7];
        for (int z = 0; z < 2; z++) lat_d[z] = values[1][z]; // Extract latitude degrees
        for (int z = 0; z < 6; z++) lat_m[z] = values[1][z + 2]; // Extract latitude minutes

        int lat_deg_strtol = strtol(lat_d, NULL, 10);
        float lat_min_strtof = strtof(lat_m, NULL);
        float lat_deg = lat_deg_strtol + lat_min_strtof / 60;

        char lon_d[3];
        char lon_m[7];
        char lonSide = values[4][0];
        for (int z = 0; z < 3; z++) lon_d[z] = values[3][z]; // Extract longitude degrees
        for (int z = 0; z < 6; z++) lon_m[z] = values[3][z + 3]; // Extract longitude minutes

        // Convert longitude degrees and minutes to decimal format
        int lon_deg_strtol = strtol(lon_d, NULL, 10);
        float lon_min_strtof = strtof(lon_m, NULL);
        float lon_deg = lon_deg_strtol + lon_min_strtof / 60;

        // Validate parsed latitude and longitude values
        if(lon_deg_strtol == 0 || lon_min_strtof == 0 || lat_deg_strtol == 0 || lat_min_strtof == 0)
        {
            // for (int i = 0; i<counter; i++) free(values[i]);
            return 0;

        } else {
            // Store values to gps_data
            gps_data->latitude = lat_deg;
            gps_data->longitude = lon_deg;
            gps_data->latSide = latSide;
            gps_data->lonSide = lonSide;
            // for (int i = 0; i<counter; i++) free(values[i]);
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
    char *values[25];
    int counter = 0;
    memset(values, 0, sizeof(values));
    char *start = inputString;
    char *end;

    // Loop until the end of the string is reached
    // while (start != NULL && *start != '\0') {

    //     end = strchr(start, ',');

    //     if (end == NULL) 
    //     {
    //         end = start + strlen(start); // Last token reached
    //     }

    //     if (end == start) 
    //     {
    //         values[counter] = malloc(1); // Allocate space for a single character

    //         values[counter][0] = '\0';   // Set it to the empty string
    //     } else {
    //         // Non-empty field found.
    //         values[counter] = malloc(end - start + 1); // Allocate space for the token
    //         strncpy(values[counter], start, end - start); // Copy the token
    //         values[counter][end - start] = '\0'; // Null-terminate it
    //     }

    //     counter++;
    //     if (*end == '\0') 
    //     {
    //         // End of the string reached.
    //         break;
    //     }
    //     start = end + 1; // Move to the start of the next token.
    // }

    while(counter < 25 && start != NULL && *start != '\0') {

        end = strchr(start, ',');

        if (end == NULL) 
        {
            end = start + strlen(start); // Last token reached
        }

        *end = '\0';
        values[counter++] = start;
        start = end + 1;
    }

    // Confirms if the date was successfully extracted
    if (counter > 9 && strlen(values[9]) == 6) 
    {
        strncpy(gps_data->date, values[9], 6);
        gps_data->date[6] = '\0';
        // for (int i = 0; i < counter; i++) free(values[i]);
        gps_data->RMC_Flag = 1;
        return 1; // Success
    } else {
        // for (int i = 0; i < counter; i++) free(values[i]);
        return 0; // Failure
    }
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
    char *values[25];
    int counter = 0;
    memset(values, 0, sizeof(values));
    char *marker = inputString;
    char *token;

    while ((token = strsep(&marker, ",")) != NULL && counter < 25) 
    {
        values[counter++] = strdup(token);
    }

    // Extact and store values to gps_data
    float trueHeading = strtof(values[1], NULL);
    gps_data->trueHeading = trueHeading!=0 ? trueHeading : gps_data->trueHeading;

    float magneticHeading = strtof(values[3], NULL);
    gps_data->magneticHeading = magneticHeading!=0 ? magneticHeading : gps_data->magneticHeading;

    float speedKmh = strtof(values[7], NULL);
    gps_data->speedKmh = speedKmh!=0 ? speedKmh : gps_data->speedKmh; 

    // for (int i = 0; i < counter; i++) 
    // {
    //     free(values[i]);
    // }

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
    char *values[25];
    int counter = 0;
    memset(values, 0, sizeof(values));
    char *marker = strtok(inputString, ",");  // Tokenize the input string using ',' as a delimiter

    // while (marker != NULL) 
    // {
    //     values[counter++] = malloc(strlen(marker) + 1); 
    //     strcpy(values[counter - 1], marker);
    //     marker = strtok(NULL, ",");
    // }

    while (counter < 25 && marker != NULL) {
        values[counter++] = marker;
        marker = strtok(NULL, ",");
    }

    // Extact and store values to gps_data
    int snr = strtol(values[7], NULL, 10);
    gps_data->snr = snr!=0 ? snr : gps_data->snr;

    int satInView = strtol(values[3], NULL, 10);
    gps_data->satInView = satInView!=0 ? satInView : gps_data->satInView;

    // for (int i = 0; i < counter; i++) 
    // {
    //     free(values[i]);
    // }

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

    // char *token = strtok((char *)buffer, "$"); // Tokenize the buffer using '$' as a delimiter
    // int cnt = 0;

    // while(token != NULL)
    // {
    //     data[cnt++] = token;

    //     data[cnt++] = malloc(strlen(token) + 1); // Allocate memory for each sentence
    //     strcpy(data[cnt-1], token); // Copy the token into allocated memory
    //     token = strtok(NULL, "$"); // Get the next sentence
    // }

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
            if(strstr(data[i], "GPGLL")!=NULL)
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
    // for(int i = 0; i<cnt; i++) 
    // {
    //     if (data[i] != NULL) { free(data[i]); }
    // }
}