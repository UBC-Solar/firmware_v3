#include "fatfs.h"
#include "fatfs_sd.h"
#include "string.h"
#include "stdio.h"
#include "time.h"
#include "stdlib.h"
#include "main.h"


FATFS fs_sdLib; 					// file system
FRESULT fresult_sdLib; 				// to store the result

FRESULT sd_mount() {
    /* mount SD card */
    fresult_sdLib = f_mount(&fs_sdLib, "", 0);
    return fresult_sdLib;
}



/**
 * @brief This function opens a file on the SD card.
 * @param filename: A pointer to a filename string.
 * @return Returns a pointer to the fatfs FIL object. If an error occurs, it returns NULL.
 */
FIL* sd_open(char *filename) {
	static FIL fil;

    fresult_sdLib = f_mount(&fs_sdLib, "", 0);
    if(fresult_sdLib != FR_OK)
    {
	printf("Bad mount\n\r");
        return NULL;
    }
    printf("double mount ok\n\r");

    /* Open the file */
    fresult_sdLib = f_open(&fil, filename, FA_WRITE | FA_OPEN_ALWAYS);
    printf("Got file pointer %x\n\r", &fil);
    if(fresult_sdLib == FR_OK) {
        /* Seek to end of the file to append data */
	printf("Opened file\n\r");
        fresult_sdLib = f_lseek(&fil, f_size(&fil));
        if(fresult_sdLib != FR_OK)
        {
            printf("Bad seek\n\r");
            f_close(&fil);
            return NULL;
        }
        printf("Opened file ok\n\r");
    }
    return &fil;
}


/**
 * @brief This function appends a string to a file on the SD card.
 * @param file: A pointer to the file object.
 * @param string: A pointer to the character array of data to be appended.
 * @return None
 */
void sd_append(FIL *file, char *string) {
    if(file != NULL) {
        /* Write the string to the file */
	printf("Writing to file\n\r");
        fresult_sdLib = f_puts(string, file);
        fresult_sdLib = f_puts("\n", file);
        if(fresult_sdLib == EOF)
        {
            printf("Got EOF\n\r");
            return;
        }
        printf("Done write\n\r");
        fresult_sdLib = f_sync(file);
        if (fresult_sdLib != FR_OK) {
            printf("Bad fsync - got %d\n\r", fresult_sdLib);
            return;
        }
        printf("Done fsync\n\r");
    }
}


/**
 * @brief This function closes a file on the SD card.
 * @param file: A pointer to the file object.
 * @return None
 */
void sd_close(FIL *file) {
    if(file != NULL) {
        /* Close the file */
        fresult_sdLib = f_close(file);
        if(fresult_sdLib != FR_OK)
        {
        	return;
        }
    }
}
