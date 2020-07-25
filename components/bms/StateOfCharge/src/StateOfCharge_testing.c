/*
StateOfCharge_testing.c
Date Started: July 19, 2020

This file contains functions to test the state of charge functions.
Intended only to be run in visual studio debugging mode.
*/
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "StateOfCharge.c"
//#include "StateOfCharge.h"


void rigorLoopPrint_param_ui32_return_float(float (*hello)(uint32_t), float lower, float upper, float increment);
//void rigorLoopPrint_param_float_return_float(float (*hello)(uint32_t), float lower, float upper, float increment);

void rigorousLoopPrint_stateOfCharge(FILE** file_ptr, float lower, float upper, float increment, char * filename);

void check_ErrorIfNull_closeIfNot(FILE** file_ptr, char* function_name);

float PH_conversionFunction(float a);

FILE* fopen_numbered_txt(char* filename, char * mode);


int main()
{
    float lower = 0;
    float upper = 0;

    FILE* fileptr1 = NULL;

    //fileptr1 = fopen_numbered_txt(filename, "w");

    rigorousLoopPrint_stateOfCharge(&fileptr1, -1, 10, 1, "./Text Logs/Testing helper function");
    check_ErrorIfNull_closeIfNot(&fileptr1, "main()");

    rigorousLoopPrint_stateOfCharge(&fileptr1, 70 * 10000, 150 * 10000, 1 * 10000, "./Text Logs/testing full loop");
    check_ErrorIfNull_closeIfNot(&fileptr1, "main()");

    lower = SOC_VOLT_MAX - 1;
    upper = SOC_VOLT_MAX + 1;
    lower = lower * 10000;
    upper = upper * 10000;
    rigorousLoopPrint_stateOfCharge(&fileptr1, lower, upper, 1 * 1000, "./Text Logs/SOC_VOLT_MAX corner test");
    check_ErrorIfNull_closeIfNot(&fileptr1, "main()");

    lower = SOC_VOLT_THRESHOLD_1 - 1;
    upper = SOC_VOLT_THRESHOLD_1 + 1;
    lower = lower * 10000;
    upper = upper * 10000;
    rigorousLoopPrint_stateOfCharge(&fileptr1, lower, upper, 1 * 1000, "./Text Logs/SOC_VOLT_THRESHOLD_1 corner test");
    check_ErrorIfNull_closeIfNot(&fileptr1, "main()");

    lower = SOC_VOLT_THRESHOLD_2 - 1;
    upper = SOC_VOLT_THRESHOLD_2 + 1;
    lower = lower * 10000;
    upper = upper * 10000;
    rigorousLoopPrint_stateOfCharge(&fileptr1, lower, upper, 1 * 1000, "./Text Logs/SOC_VOLT_THRESHOLD_2 corner test");
    check_ErrorIfNull_closeIfNot(&fileptr1, "main()");

    lower = SOC_VOLT_THRESHOLD_3 - 1;
    upper = SOC_VOLT_THRESHOLD_3 + 1;
    lower = lower * 10000;
    upper = upper * 10000;
    rigorousLoopPrint_stateOfCharge(&fileptr1, lower, upper, 1 * 1000, "./Text Logs/SOC_VOLT_THRESHOLD_3 corner test");
    check_ErrorIfNull_closeIfNot(&fileptr1, "main()");

    lower = SOC_VOLT_MIN - 1;
    upper = SOC_VOLT_MIN + 1;
    lower = lower * 10000;
    upper = upper * 10000;
    rigorousLoopPrint_stateOfCharge(&fileptr1, lower, upper, 1 * 1000, "./Text Logs/SOC_VOLT_MIN corner test");
    check_ErrorIfNull_closeIfNot(&fileptr1, "main()");

    return 0;
}





/*
Note that the pointer to a pointer allows for a file-pointer created internally to be returned to the outside of the function.
*/
void rigorousLoopPrint_stateOfCharge(FILE ** file_ptr, float lower, float upper, float increment, char * filename)
{
    float a = 0;
    FILE* file;

    file = fopen_numbered_txt(filename, "w");
    if (file == NULL)
    {
        printf("Error: rigorousLoopPrint_stateOfCharge() cannot create file.");
        return;
    }

    *file_ptr = file;

    //table labels
    //             1234567890    | 1234567890
    fprintf(file, "   voltage(V)          SoC(%%)\n", a);

    for (float x = lower; x <= upper; x = x + increment)
    {
        a = stateOfCharge(x);
        fprintf(file, "%10.1f      %10.4f\n", x * 0.0001, a);
    }
    return;
}


/*
parameter:
    function_name : indicates what function called this helper function.
*/
void check_ErrorIfNull_closeIfNot(FILE** file_ptr, char * function_name)
{
    if (*file_ptr == NULL)
    {
        printf("Error: %s: file not opened.\r\n", function_name);
        return 0;
    }
    else
    {
        fclose(*file_ptr);
        printf("File closed.\r\n");
    }
}

void rigorLoopPrint_param_ui32_return_float(float (*hello)(uint32_t), float lower, float upper, float increment)
{
    float
        a;


    for (float x = lower; x <= upper; x = x + increment)
    {
        a = (*hello)(x);
        printf("%07.2f,     %f     %f\r\n", a, a, x);
    }


    return;
}

//void rigorLoopPrint_param_float_return_float(float (*hello)(float), float lower, float upper, float increment)
//{
//
//}

float PH_conversionFunction(float a)
{
    return a;
    //return a + 0.12345;
}

//note that this function is meant only for logging INSECURE data
FILE* fopen_numbered_txt(char * filename, char * mode)
{
    int copy_num = 1;
    char copy_label[] = "(   )";
    int count = 1;
    char char_var = *filename;


    FILE * file;
    char full_filename[255] = "";
    char txt[] = ".txt";

    /*size_t
        size1 = sizeof(filename),
        size2 = sizeof(txt);*/

    //manual counting due to dynamic size of filename[]
    while (char_var != '\0' && count < 255)
    {
        char_var = *(filename + count);
        count++;
    }

    //check for if file name exceeds Windows maximum file name length or an arbitrary too-large size
    size_t filename_length = count - 1; //subtract 1 for the NULL char.
    size_t txt_length = (sizeof(txt)) / (sizeof txt[0]);
    if (filename_length + txt_length > 255)
    {
        printf("Error: %s.txt exceeds 255 characters.\r\n", filename);
        return;
    }
    else if (filename_length + txt_length > 200)
    {
        printf("Warning: %s.txt exceeds 200 characters.\r\n", filename);
    }

    strcat(full_filename, filename);
    strcat(full_filename, txt);
    printf("The initial file name is: %s\r\n", full_filename);

    file = fopen(full_filename, "r");
    if(file == NULL) //file does not exist yet.
    {
        file = fopen(full_filename, "w");
        if (file == NULL) //file cannot be opened.
        {
            printf("Error: fopen() failed to open %s\r\n", full_filename);
            goto FAILURE;
        }
        else
        {
            goto SUCCESS;
        }
    }

    else //(file != NULL) //file already exists.
    {
        //check for a free file name
        while (file != NULL && copy_num < 300)
        {
            fclose(file);
            strcpy(full_filename, "");
            strcpy(copy_label, "");
            sprintf(copy_label, "(%i)\0", copy_num);

            strcat(full_filename, filename);
            strcat(full_filename, copy_label);
            strcat(full_filename, txt);
#ifdef DEBUG
            printf("The processing file name is: %s\r\n", full_filename);
#endif
            file = fopen(full_filename, "r");
            copy_num++;
        }
        if (file != NULL)//300 copies of the file exist already.
        {
                printf("Error: %s could not be opened. Are there already 300 or more copies of the file?\r\n", full_filename);
                goto FAILURE;
        }
        else //if (file == NULL) //file does not exist yet.
        {
            file = fopen(full_filename, "w");
            if (file == NULL)
            {
                printf("Error: fopen() failed to open %s\r\n", full_filename);
                goto FAILURE;
            }
            else
                goto SUCCESS;
        }
    }



FAILURE:
    if(file != NULL)
    {
        fclose(file);
    }
    return NULL;

SUCCESS:
    printf("The final file name is: %s\r\n", full_filename);
    return file;
}//end of fopen_numbered_txt()
