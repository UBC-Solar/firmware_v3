#include "fatfs.h"
#include "fatfs_sd.h"

#ifndef SDCARD_H
#define SDCARD_H


/**
 * @brief Mounts FATFS file system onto the SD Card.
 * @return success value of the result
 */
FRESULT sd_mount();

/**
 * @brief This function opens a file on the SD card.
 * @param filename: A pointer to a filename string.
 * @return Returns a pointer to the fatfs FIL object. If an error occurs, it returns NULL.
 */
FIL* sd_open(char *filename);


/**
 * @brief This function appends a string to a file on the SD card.
 * @param file: A pointer to the file object.
 * @param string: A pointer to the character array of data to be appended.
 * @return None
 */
void sd_append(FIL *file, char *string);


/**
 * @brief This function closes a file on the SD card.
 * @param file: A pointer to the file object.
 * @return None
 */
void sd_close(FIL *file);

#endif
