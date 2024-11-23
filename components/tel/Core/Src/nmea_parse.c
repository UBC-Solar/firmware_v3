// CAN message is only 8 bytes long

#include <stdint.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "nmea_parse.h"

// Test function
// int main() {

//     GPS gps_data = {0};
//     uint8_t buffer[1024];

//     // Simulate loading NMEA sentences from a file
//     FILE *nmeaFile = fopen("gpsData1.txt", "r");

//     // Read the file line by line
//     while (fgets((char *)buffer, sizeof(buffer), nmeaFile) != NULL) {
//         nmea_parse(&gps_data, buffer);

//         // Print parsed results
//         printf("Latitude: %.6f %c\n", gps_data.latitude, gps_data.latSide);
//         printf("Longitude: %.6f %c\n", gps_data.longitude, gps_data.lonSide);
//         printf("UTC Time: %s\n", gps_data.utcTime);
//         printf("Geod height: %.2f m\n", gps_data.geodHeight);
//         printf("Altitude: %.2f m\n", gps_data.altitude);
//         printf("Satellites: %d\n", gps_data.satelliteCount);
//         printf("Fix: %d\n", gps_data.fix);
//         printf("HDOP: %.2f\n", gps_data.hdop);
//         printf("VDOP: %.2f\n", gps_data.vdop);
//         printf("PDOP: %.2f\n", gps_data.pdop);
//         printf("SNR: %d\n", gps_data.snr);
//         printf("Satellites in View: %d\n", gps_data.satInView);
//         printf("True Heading: %.2f\n", gps_data.trueHeading);
//         printf("Magnetic Heading: %.2f\n", gps_data.magneticHeading);
//         printf("Speed (km/h): %.2f\n", gps_data.speedKmh);
//         printf("Date: %s\n", gps_data.date);
//         printf("RMC Flag: %d\n", gps_data.RMC_Flag);
//         printf("-----------------------------\n");
//     }

//     fclose(nmeaFile);
//     return 0;
// }


char *data[15];

int gps_checksum(char *nmea_data)
{
    //if you point a string with less than 5 characters the function will read outside of scope and crash the mcu.
    if(strlen(nmea_data) < 5) return 0;
    char recv_crc[2];
    recv_crc[0] = nmea_data[strlen(nmea_data) - 4];
    recv_crc[1] = nmea_data[strlen(nmea_data) - 3];
    int crc = 0;
    int i;

    //exclude the CRLF plus CRC with an * from the end
    for (i = 0; i < strlen(nmea_data) - 5; i ++) {
        crc ^= nmea_data[i];
    }
    int receivedHash = strtol(recv_crc, NULL, 16);
    if (crc == receivedHash) {
        return 1;
    }
    else{
        return 0;
    }
}


int nmea_GPGGA(GPS *gps_data, char*inputString){
    char *values[25];
    int counter = 0;
    memset(values, 0, sizeof(values));
    char *marker = strtok(inputString, ",");
    while (marker != NULL) {
        values[counter++] = malloc(strlen(marker) + 1); //free later!!!!!!
        strcpy(values[counter - 1], marker);
        marker = strtok(NULL, ",");
    }
    char lonSide = values[5][0];
    char latSide = values[3][0];
    strcpy(gps_data->lastMeasure, values[1]);
    if(latSide == 'S' || latSide == 'N'){
        char lat_d[2];
        char lat_m[7];
        for (int z = 0; z < 2; z++) lat_d[z] = values[2][z];
        for (int z = 0; z < 6; z++) lat_m[z] = values[2][z + 2];

        int lat_deg_strtol = strtol(lat_d, NULL, 10);
        float lat_min_strtof = strtof(lat_m, NULL);
        double lat_deg = lat_deg_strtol + lat_min_strtof / 60;

        char lon_d[3];
        char lon_m[7];

        for (int z = 0; z < 3; z++) lon_d[z] = values[4][z];
        for (int z = 0; z < 6; z++) lon_m[z] = values[4][z + 3];

        int lon_deg_strtol = strtol(lon_d, NULL, 10);
        float lon_min_strtof = strtof(lon_m, NULL);
        double lon_deg = lon_deg_strtol + lon_min_strtof / 60;

        if(lat_deg!=0 && lon_deg!=0 && lat_deg<90 && lon_deg<180){
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

            if (values[1][0] != '\0') {
                strncpy(gps_data->utcTime, values[1], sizeof(gps_data->utcTime) - 1);
                gps_data->utcTime[sizeof(gps_data->utcTime) - 1] = '\0';
            } else {
                gps_data->utcTime[0] = '\0';
            }
        } else {
            for(int i=0; i<counter; i++) free(values[i]);
            return 0;
        }
    }

    for(int i=0; i<counter; i++) free(values[i]);
    return 1;
}

// add hdop, vdop and etc
int nmea_GPGSA(GPS *gps_data, char* inputString) {
    char *values[25];
    int counter = 0;
    memset(values, 0, sizeof(values));
    char *marker = inputString;
    char *token;
    while ((token = strsep(&marker, ",")) != NULL) {
        values[counter++] = strdup(token); // Remember to free later
    }
    
    int fix = strtol(values[2], NULL, 10);
    gps_data->fix = fix > 1 ? 1 : 0;
    int satelliteCount = 0;
    for(int i=3; i<15; i++){
        if(values[i][0] != '\0'){
            satelliteCount++;
        }
    }
    gps_data->satelliteCount = satelliteCount;

    float pdop = strtof(values[15], NULL);
    gps_data->pdop = pdop!=0.0 ? pdop : gps_data->pdop;

    float hdop = strtof(values[16], NULL);
    gps_data->hdop = hdop!=0.0 ? hdop : gps_data->hdop;

    float vdop = strtof(values[17], NULL);
    gps_data->vdop = vdop!=0.0 ? vdop : gps_data->vdop;

    for(int i = 0; i < counter; i++) free(values[i]);
}


int nmea_GPGLL(GPS *gps_data, char*inputString) {

    char *values[25];
    int counter = 0;
    memset(values, 0, sizeof(values));
    char *marker = strtok(inputString, ",");
    while (marker != NULL) {
        values[counter++] = malloc(strlen(marker) + 1); //free later!!!!!!
        strcpy(values[counter - 1], marker);
        marker = strtok(NULL, ",");
    }
    char latSide = values[2][0];
    if (latSide == 'S' || latSide == 'N') { //check if data is sorta intact
        char lat_d[2];
        char lat_m[7];
        for (int z = 0; z < 2; z++) lat_d[z] = values[1][z];
        for (int z = 0; z < 6; z++) lat_m[z] = values[1][z + 2];

        int lat_deg_strtol = strtol(lat_d, NULL, 10);
        float lat_min_strtof = strtof(lat_m, NULL);
        double lat_deg = lat_deg_strtol + lat_min_strtof / 60;

        char lon_d[3];
        char lon_m[7];
        char lonSide = values[4][0];
        for (int z = 0; z < 3; z++) lon_d[z] = values[3][z];
        for (int z = 0; z < 6; z++) lon_m[z] = values[3][z + 3];

        int lon_deg_strtol = strtol(lon_d, NULL, 10);
        float lon_min_strtof = strtof(lon_m, NULL);
        double lon_deg = lon_deg_strtol + lon_min_strtof / 60;
        //confirm that we aren't on null island
        if(lon_deg_strtol == 0 || lon_min_strtof == 0 || lat_deg_strtol == 0 || lat_min_strtof == 0) {
            for(int i = 0; i<counter; i++) free(values[i]);
            return 0;
        }
        else{
            gps_data->latitude = lat_deg;
            gps_data->longitude = lon_deg;
            gps_data->latSide = latSide;
            gps_data->lonSide = lonSide;
            for(int i = 0; i<counter; i++) free(values[i]);
            return 1;
        }
    }
    else return 0;
}


int nmea_GPRMC(GPS *gps_data, char* inputString) {

    char *values[25];
    int counter = 0;
    memset(values, 0, sizeof(values));
    char *start = inputString;
    char *end;

    // Loop until the end of the string is reached.
    while (start != NULL && *start != '\0') {
        end = strchr(start, ',');
        if (end == NULL) {
            // Last token reached.
            end = start + strlen(start);
        }

        if (end == start) {
            // Empty field found.
            values[counter] = malloc(1); // Allocate space for a single character
            values[counter][0] = '\0';   // Set it to the empty string
        } else {
            // Non-empty field found.
            values[counter] = malloc(end - start + 1); // Allocate space for the token
            strncpy(values[counter], start, end - start); // Copy the token
            values[counter][end - start] = '\0'; // Null-terminate it
        }

        counter++;
        if (*end == '\0') {
            // End of the string reached.
            break;
        }
        start = end + 1; // Move to the start of the next token.
    }

    // ... (Rest of your code for checking and handling the date and freeing memory)

    // Make sure to check if the date was successfully extracted
    if (counter > 9 && strlen(values[9]) == 6) {
        strncpy(gps_data->date, values[9], 6);
        gps_data->date[6] = '\0';
        // Free allocated memory
        for (int i = 0; i < counter; i++) free(values[i]);
        gps_data->RMC_Flag = 1;
        return 1; // Success
    } else {
        // Free allocated memory before returning
        for (int i = 0; i < counter; i++) free(values[i]);
        return 0; // Failure
    }
}


int nmea_GPVTG(GPS *gps_data, char* inputString) {
    char *values[25];
    int counter = 0;
    memset(values, 0, sizeof(values));
    char *marker = inputString;
    char *token;
    while ((token = strsep(&marker, ",")) != NULL) {
        values[counter++] = strdup(token); // Remember to free later
    }

    float trueHeading = strtof(values[1], NULL);
    gps_data->trueHeading = trueHeading!=0 ? trueHeading : gps_data->trueHeading;

    float magneticHeading = strtof(values[3], NULL);
    gps_data->magneticHeading = magneticHeading!=0 ? magneticHeading : gps_data->magneticHeading;

    float speedKmh = strtof(values[7], NULL);
    gps_data->speedKmh = speedKmh!=0 ? speedKmh : gps_data->speedKmh;

    for (int i = 0; i < counter; i++) {
        free(values[i]);
    }

    return 1;
}


int nmea_GPGSV(GPS *gps_data, char* inputString){
    char *values[25];
    int counter = 0;
    memset(values, 0, sizeof(values));
    char *marker = strtok(inputString, ",");
    while (marker != NULL) {
        values[counter++] = malloc(strlen(marker) + 1); 
        strcpy(values[counter - 1], marker);
        marker = strtok(NULL, ",");
    }

    int snr = strtol(values[7], NULL, 10);
    gps_data->snr = snr!=0 ? snr : gps_data->snr;

    int satInView = strtol(values[3], NULL, 10);
    gps_data->satInView = satInView!=0 ? satInView : gps_data->satInView;

    for (int i = 0; i < counter; i++) {
        free(values[i]);
    }

    return 1;
}


void nmea_parse(GPS *gps_data, uint8_t *buffer){
    memset(data, 0, sizeof(data));
    char *token = strtok((char *)buffer, "$"); // TODO: Check if buffer can be casted to a char * for strtok argument
    int cnt = 0;
    while(token !=NULL){
        data[cnt++] = malloc(strlen(token)+1); //free later!!!!!
        strcpy(data[cnt-1], token);
        token = strtok(NULL, "$");
    }
    for(int i = 0; i<cnt; i++){
       if(strstr(data[i], "\r\n")!=NULL){
           if(strstr(data[i], "GPGLL")!=NULL){
               nmea_GPGLL(gps_data, data[i]);
            }
            else if(strstr(data[i], "GNGSA")!=NULL || (strstr(data[i], "GPGGA"))!=NULL){
                nmea_GPGSA(gps_data, data[i]);
            }
            else if(strstr(data[i], "GNGGA") != NULL || (strstr(data[i], "GPGGA"))!=NULL){
                nmea_GPGGA(gps_data, data[i]);
            }
            else if(strstr(data[i], "GNRMC") != NULL || (strstr(data[i], "GPRMC"))!=NULL){
	            nmea_GPRMC(gps_data, data[i]);
	        }
            else if(strstr(data[i], "GNVTG") != NULL || (strstr(data[i], "GPVTG"))!=NULL){
                nmea_GPVTG(gps_data, data[i]);
            }
            else if(strstr(data[i], "GNGSV") != NULL || (strstr(data[i], "GPGSV"))!=NULL){
	            nmea_GPGSV(gps_data, data[i]);
            }
       }

    }
    for(int i = 0; i<cnt; i++) free(data[i]);
}