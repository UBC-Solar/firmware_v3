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
void sd_str_to_hexnums(char* string, size_t string_length, char* hex_nums_buffer);


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
void sd_append_as_hexnums(FIL * file, char * string, size_t string_length);

#endif
