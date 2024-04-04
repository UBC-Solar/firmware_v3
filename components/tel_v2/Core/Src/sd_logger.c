#include <stdio.h>
#include "sd_logger.h"

/**
 * @brief This function clears a buffer to ensure that strings are actually written to the file.
 * @param None
 * @return None
 */
void bufclear_sdLib(void)
{
    for (int i = 0; i < 1024; i++)
    {
        buffer_sdLib[i] = '\0';
    }
}


/**
 * @brief This function opens a file on the SD card.
 * @param filename: A pointer to a filename string.
 * @return Returns a pointer to the fatfs FIL object. If an error occurs, it returns NULL.
 */
void sd_open(char *filename)
{
    /* mount SD card */
    fresult = f_mount(&fs, "", 0);
    if(fresult != FR_OK)
    {
        printf("error in mounting SD CARD...\n");
        return;
    }

    /* Open the file */
    fresult = f_open(&fil, filename, FA_WRITE | FA_CREATE_ALWAYS);
//    if(fresult == FR_OK) {
//        /* Seek to end of the file to append data */
//	fresult = f_lseek(&fil, f_size(&fil));
//        if(fresult != FR_OK)
//        {
//            printf("Seek to end of file failed..\n\r");
//            f_close(&fil);
//            return;
//        }
//        printf("Seek successful\n\r");
//    }

    printf("returning file pointer %x\n\r", (void*) &fil);
    return;
}


/**
 * @brief This function appends a string to a file on the SD card.
 * @param file: A pointer to the file object.
 * @param string: A pointer to the character array of data to be appended.
 * @return None
 */
void sd_append(FIL *file, char *string)
{
    printf("Appending %s\n\r to file pointer %x\n\r", string, file);
    if(file != NULL) {
        /* Write the string to the file */
	fresult = f_puts(string, file);
        if(fresult == EOF)
        {
            printf("error in writing to file...\n");
        }
        /* Clear the buffer */
        bufclear_sdLib();
        printf("sd append successful\n\r");
    }
}


/**
 * @brief This function closes a file on the SD card.
 * @param file: A pointer to the file object.
 * @return None
 */
void sd_close(FIL *file)
{
    if(file != NULL) {
        /* Close the file */
	fresult = f_close(file);
        if(fresult != FR_OK)
        {
            printf("error in closing file...\n");
        }
    }
}
