#include "stdlib.h"
#include "sd_logger.h"


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
        fresult_sdLib = f_puts("\n", file);
        if(fresult_sdLib == EOF)
        {
            return;
        }
        fresult_sdLib = f_sync(file);
        if (fresult_sdLib != FR_OK) {
            return;
        }
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


/**
 * @brief Converts a char buffer to a string of hex numbers based on ASCII values
 *        Ex: "123" -> "313233"
 * @param string: The string to be converted to a series of hex numbers.
 * @param string_length: The length of the string. DO NOT USE strlen or else null values in the string will not be appended
 * 	          Ex: If string contains '/0' then strlen will end early even if you wanted rest of the string
 * @param hex_nums_buffer: A char array that is twice the length of the string param to hold the hex nums
 *                         Will be null terminated.
 * @return None
 */
void sd_str_to_hexnums(char* string, size_t string_length, char* hex_nums_buffer)
{
  for (size_t i = 0; i < string_length; ++i) {
      sprintf(hex_nums_buffer + i * 2, "%02X", string[i]);
  }
}


/**
 * @brief Converts a char buffer to a string of hex numbers based on ASCII values
 *        Ex: "123" -> "313233" AND appends it to the sd card.
 *        This is just a special wrapper for the sd_append function
 * @param file: A pointer to the file object to which the string will be appended
 * @param string: The string to be converted to a series of hex numbers and appended in the SD card
 * @param string_length: The length of the string. DO NOT USE strlen or else null values in the string will not be appended
 * 	          Ex: If string contains '/0' then strlen will end early even if you wanted rest of the string
 * @return None
 */
void sd_append_as_hexnums(FIL * file, char * string, size_t string_length)
{
  char hex_nums[string_length * 2];
  sd_str_to_hexnums(string, string_length, hex_nums);
  sd_append(file, hex_nums);
}
