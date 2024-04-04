/*
 * sd_logger.h
 *
 *  Created on: Apr 2, 2024
 *      Author: ishan
 */

#ifndef INC_SD_LOGGER_H_
#define INC_SD_LOGGER_H_

#include "fatfs.h"

extern FATFS fs;
extern FIL fil;
extern FRESULT fresult;
extern char buffer_sdLib[1024]; // to store data in buffer

void bufclear_sdLib(void);
void sd_open(char *filename);
void sd_append(FIL *file, char *string);
void sd_close(FIL *file);

#endif /* INC_SD_LOGGER_H_ */
