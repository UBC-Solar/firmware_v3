pre.CodeMirror-line {
    line-height: 1.3 !important;
}

## Purpose

-   The `sdCardLib` library aims to abstract the following functions (and potentially others related to SD Card operations). Currently, the operations involve **opening, appending,** and **closing** a file on the SD Card.
    -   `FIL* sd_open(char *filename)`
        -   This function takes a `filename` as input and will mount and create a file on the SD Card with that `filename`. It returns the pointer to the file on the file system of the SD Card. **Note that this function will open an already existing file or will create one if one with this filename does not exist.**
    -   `void sd_append(FIL *file, char *string)`
        -   This function takes the pointer to the file on the SD Card (given by `sd_open`, for example) and the string to append to this file as inputs. It will append the string to the file wherever it left off (the end of the file by default using `f_lseek`).
    -   `void sd_close(FIL *file)`
        -   This functions takes the pointer to the file on the SD Card and then closes it.

## Usage

1. First ensure your STM32Cube IDE project is configured with `FATFS` in the `Middleware and Software Packages` section.
2. Either download the `sdCardLib` folder or reference it from the `firmware_v3` folder for the next step.
3. To create a **symlink** of `sdCardLib` in your project by following these steps:
    - Go to Project -> Properties -> C/C++ General -> Paths and Symbols.
    - Go to the `Includes` tab and click `Add...`. Navigate and select the `Inc` subfolder inside `Core` inside of `sdCardLib`. The path should look like: `/<PARENT_DIRECTORY>/sdCardLib/Core/Inc`.
    - Now go to the `Source Location` tab and click `Link Folder...`. Check the 'Link to folder in the file system' option and hit `Browse...`. Navigate and select the `Src` folder inside of `Core` of `sdCardLib`. Finally, rename this folder name to something like `sdLibSrc` (you are creating the name for the symlink).
    - Finally, hit `Apply and Close`.
4. Use `sdCardLib` in your project by first including it: `#include "sdcard.h"`.
5. Here is some example usage:
    - The output of this code on the SD Card should be 2 lines which say `ID: 0xbbb, Data: 0000, Timestamp: 33572, Length: 0` and `ID: 0xbbb, Data: 0000, Timestamp: 33572, Length: 0`.

```
    // Define struct for CAN data
    struct CAN_DATA{
        uint16_t ID;
        uint64_t DATA;
        uint64_t TIME;
        uint8_t LENGTH;
    };


    // Create the struct for CAN data
    struct CAN_DATA CAN_DATA;

    FATFS fs;				// file system
    FRESULT fresult;		// to store the result
    char buffer[1024];		// to store data in buffer

    // Create a pointer to the FATFS type file
    FIL *file_pointer = sd_open("my_file_name.txt");

    // Append to the file (regardless of empty or not)

    /* Set Example CAN Data */
    CAN_DATA.ID = 0xBBB;
    CAN_DATA.DATA = 0x8324;
    CAN_DATA.TIME = 28492;
    CAN_DATA.LENGTH = 8;

    char SD_message[64] = "";
    sprintf(SD_message, "ID: %#.3x, Data: %#.4x, Timestamp: %d, Length: %d\n", CAN_DATA.ID, CAN_DATA.DATA, CAN_DATA.TIME, CAN_DATA.LENGTH);

    // Append to the file 2 times
    for (int i = 0; i < 2; i++)
    {
        sd_append(file_pointer, SD_message);
    }

    // Close the file
    sd_close(file_pointer);

```

6. Happy Coding!
