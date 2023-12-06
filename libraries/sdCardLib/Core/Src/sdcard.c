#include "sdcard.h"
#include "fatfs.h"
#include "fatfs_sd.h"
#include "string.h"
#include "stdio.h"
#include "time.h"
#include "stdlib.h"
#include "main.h"


FATFS fs_sdLib; 					// file system
FRESULT fresult_sdLib; 				// to store the result
UART_HandleTypeDef huart2_sdLib;
char buffer_sdLib[1024];					// to store data in buffer


/**
 * @brief This function sends a string of data via UART.
 * @param string: A pointer to the character array of data to be sent.
 * @return None
 */
void send_uart_sdLib (char *string)
{
    uint8_t len = strlen(string);
    HAL_UART_Transmit(&huart2_sdLib, (uint8_t *) string, len, 2000); // transmit in blocking mode
}


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
FIL* sd_open(char *filename) {
    static FIL fil; // Not global for safety

    /* mount SD card */
    fresult_sdLib = f_mount(&fs_sdLib, "", 0);
    if(fresult_sdLib != FR_OK)
    {
        send_uart ("error in mounting SD CARD...\n");
        return NULL;
    }

    /* Open the file */
    fresult_sdLib = f_open(&fil, filename, FA_WRITE | FA_OPEN_ALWAYS);
    if(fresult_sdLib == FR_OK) {
        /* Seek to end of the file to append data */
        fresult_sdLib = f_lseek(&fil, f_size(&fil));
        if(fresult_sdLib != FR_OK)
        {
            f_close(&fil);
            return NULL;
        }
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
        fresult_sdLib = f_puts(string, file);
        if(fresult_sdLib == EOF)
        {
            send_uart("error in writing to file...\n");
        }
        /* Clear the buffer */
        bufclear_sdLib();
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
            send_uart("error in closing file...\n");
        }
    }
}
