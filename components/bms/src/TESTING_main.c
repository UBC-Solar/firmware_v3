#include <stdio.h>
#include <stdint.h>
#include "CANbus_TESTING_ONLY.h"
#include "CANbus_functions.h"
//#include "CANbus_functions.c"

#define PH_MESSAGE_SERIES_LENGTH 4
#define PH_MESSAGE_ARRAY_SIZE 8

#define CODEWORD_DEBUG_BRIGHTSIDE

void setPackdata_striped(BTM_PackData_t* pPackdata);
void setPackdata_incrementing(BTM_PackData_t* pPackdata);


void initMessageSeries(
    Brightside_CAN_MessageSeries* seriesStruct,
    Brightside_CAN_Message* messageWiseContent,
    uint8_t messageArrays[PH_MESSAGE_SERIES_LENGTH][PH_MESSAGE_ARRAY_SIZE],
    int messageSeriesSize,
    int messageArraySize
);

void checkMessageStructLinks(
    Brightside_CAN_MessageSeries* PH_messageSeries,
    Brightside_CAN_Message* PH_message,
    uint8_t messageArray[PH_MESSAGE_SERIES_LENGTH][PH_MESSAGE_ARRAY_SIZE]
);

void assignAndCheckMessageStructLinks(
    Brightside_CAN_MessageSeries* PH_messageSeries,
    Brightside_CAN_Message* PH_message,
    uint8_t messageArray[PH_MESSAGE_SERIES_LENGTH][PH_MESSAGE_ARRAY_SIZE]
);

void main()
{
    BTM_PackData_t PH_VS_PACKDATA;
    Brightside_CAN_MessageSeries PH_messageSeries;
    Brightside_CAN_Message PH_message[PH_MESSAGE_SERIES_LENGTH];
    uint8_t messageArray[PH_MESSAGE_SERIES_LENGTH][PH_MESSAGE_ARRAY_SIZE];

    //Sanity check: if printf works.
    printf("Hello world!\r\n\r\nBiscuits.\r\n\r\n");

    //Sanity check: setting and changing pack voltage
    PH_VS_PACKDATA.pack_voltage = 111222333;
    printf("pack voltage: %i \r\n", PH_VS_PACKDATA.pack_voltage);
    PH_VS_PACKDATA.pack_voltage = 444555666;
    printf("pack voltage: %i \r\n", PH_VS_PACKDATA.pack_voltage);

    //Sanity check: setting and changing packdata data, and printing the contents
    setPackdata_striped(&PH_VS_PACKDATA);
    setPackdata_incrementing(&PH_VS_PACKDATA);


    //creating the message structs
    CAN_InitHeaderStruct(PH_message, PH_MESSAGE_SERIES_LENGTH);

    //assignAndCheckMessageStructLinks(&PH_messageSeries, PH_message, messageArray);

    initMessageSeries(
        &PH_messageSeries,
        PH_message,
        messageArray,
        PH_MESSAGE_SERIES_LENGTH,
        PH_MESSAGE_ARRAY_SIZE
    );

    checkMessageStructLinks(
        &PH_messageSeries,
        PH_message,
        messageArray
    );
#ifdef NOT_EXISTING
    //initialising the message array
    printf("Initialising messageArray.\r\n");
    for (int series_i = 0; series_i < PH_MESSAGE_SERIES_LENGTH; ++series_i)
    {
        for (int message_i = 0; message_i < PH_MESSAGE_ARRAY_SIZE; ++message_i)
        {
            messageArray[series_i][message_i] = series_i + message_i;
            printf("%2.1i  ", messageArray[series_i][message_i]);
        }
        printf("\r\n");
    };
    
    printf("Assigning and checking messageArray in messageSeries struct\r\n");
    for (int series_i = 0; series_i < PH_MESSAGE_SERIES_LENGTH; ++series_i)
    {
        PH_message[series_i].dataFrame = &messageArray[series_i];
        for (int message_i = 0; message_i < PH_MESSAGE_ARRAY_SIZE; ++message_i)
        {
            printf("%2.1i  ", PH_message[series_i].dataFrame[message_i]);
        }
        printf("\r\n");
    };

    printf("0 + 1 = %2.1i  \r\n", PH_message[0].dataFrame[1]);
    printf("1 + 2 = %2.1i  \r\n", PH_message[1].dataFrame[2]);
    printf("3 + 5 = %2.1i  \r\n", PH_message[3].dataFrame[5]);
    
    printf("Initialising messageArray.\r\n");
    for (int series_i = 0; series_i < PH_MESSAGE_SERIES_LENGTH; ++series_i)
    {
        for (int message_i = 0; message_i < PH_MESSAGE_ARRAY_SIZE; ++message_i)
        {
            messageArray[series_i][message_i] = series_i + message_i + 20;
            printf("%2.1i  ", messageArray[series_i][message_i]);
        }
        printf("\r\n");
    };

    printf("Assigning and checking messageArray in messageSeries struct\r\n");
    for (int series_i = 0; series_i < PH_MESSAGE_SERIES_LENGTH; ++series_i)
    {
        PH_message[series_i].dataFrame = &messageArray[series_i];
        for (int message_i = 0; message_i < PH_MESSAGE_ARRAY_SIZE; ++message_i)
        {
            printf("%2.1i  ", PH_message[series_i].dataFrame[message_i]);
        }
        printf("\r\n");
    };

    printf("0 + 1 + 20 = %2.1i  \r\n", PH_message[0].dataFrame[1]);
    printf("2 + 4 + 20 = %2.1i  \r\n", PH_message[2].dataFrame[4]);
    printf("3 + 6 + 20 = %2.1i  \r\n", PH_message[3].dataFrame[6]);

    //setting up the rest of the message series struct
    PH_messageSeries.message      = &PH_message; //assigns address of message-wise content to series struct
    PH_messageSeries.runningIndex = 0;
    PH_messageSeries.messageSeriesSize = PH_MESSAGE_SERIES_LENGTH;

    //checking that the array of structs, PH_message[], was linked properly to the messageSeries struct
    printf("Checking that the structs are linked correctly.\r\n");
    for (int series_i = 0; series_i < PH_MESSAGE_SERIES_LENGTH; ++series_i)
    {
        for (int message_i = 0; message_i < PH_MESSAGE_ARRAY_SIZE; ++message_i)
        {
            printf("%2.1i  ", PH_messageSeries.message[series_i].dataFrame[message_i]);
        }
        printf("\r\n");
    };
    printf("Initialising messageArray.\r\n");
    for (int series_i = 0; series_i < PH_MESSAGE_SERIES_LENGTH; ++series_i)
    {
        for (int message_i = 0; message_i < PH_MESSAGE_ARRAY_SIZE; ++message_i)
        {
            messageArray[series_i][message_i] = series_i + message_i;
            printf("%2.1i  ", messageArray[series_i][message_i]);
        }
        printf("\r\n");
    };
    printf("Checking that the structs are linked correctly.\r\n");
    for (int series_i = 0; series_i < PH_MESSAGE_SERIES_LENGTH; ++series_i)
    {
        for (int message_i = 0; message_i < PH_MESSAGE_ARRAY_SIZE; ++message_i)
        {
            printf("%2.1i  ", PH_messageSeries.message[series_i].dataFrame[message_i]);
        }
        printf("\r\n");
    };
#endif
    




#ifdef NOT_EXISTING
    for (int device_i = 0; device_i < BTM_NUM_DEVICES; ++device_i)
    {
        for (int module_i = 0; module_i < BTM_NUM_MODULES; ++module_i)
        {
            PH_VS_PACKDATA.stack[device_i].module[module_i].enable      = MODULE_ENABLED;
            PH_VS_PACKDATA.stack[device_i].module[module_i].voltage     = 2.2;
            PH_VS_PACKDATA.stack[device_i].module[module_i].temperature = 3;
            PH_VS_PACKDATA.stack[device_i].module[module_i].bal_status  = DISCHARGE_ON;
            uint16_t* pPD_enable      = &PH_VS_PACKDATA.stack[device_i].module[module_i].enable     ;
            uint16_t* pPD_voltage     = &PH_VS_PACKDATA.stack[device_i].module[module_i].voltage    ;
            uint16_t* pPD_temperature = &PH_VS_PACKDATA.stack[device_i].module[module_i].temperature;
            uint16_t* pPD_bal_status  = &PH_VS_PACKDATA.stack[device_i].module[module_i].bal_status ;
            printf("device: %2.1i  module: %2.1i  enable: %2.1i  voltage: %2.1i  temp.: %2.1i  bal_status: %2.1i\r\n", 
                device_i,
                module_i,
                *pPD_enable,
                *pPD_voltage,
                *pPD_temperature,
                *pPD_bal_status);
        };
    };
#endif

    //Adding variety to the assignments to check that the initial for-loop actually worked per-element.




    //simple functions calls to ensure the functions are linked properly in Visual Studio
    

    //function-wise testing

    //full function-chain testing


    //system("pause");
    return;

}

/*
    Copied from ltc6811_btm.c, created by UBC Solar.
*/

#define BTM_VOLTAGE_CONVERSION_FACTOR 0.0001

/**
 * @brief Converts a voltage reading from a register in the LTC6811 to a float
 * Each cell voltage is provided as a 16-bit value where
 * voltage = 0.0001V * raw value
 *
 * @param raw_reading The 16-bit reading from an LTC6811
 * @return Returns a properly scaled floating-point version of raw_reading
 */
float BTM_regValToVoltage(uint16_t raw_reading)
{
    return raw_reading * BTM_VOLTAGE_CONVERSION_FACTOR;
}

/*
    End of copy.
*/

/*
    Copied from thermistor.h, created by UBC Solar.
/*

/**
 * @brief Converts a raw thermistor voltage reading from an LTC6811 into a temperature
 *
 * @param[in] Vout the thermistor voltage reading to convert
 * @return the temperature of the thermistor in degrees celcius
 */
double BTM_TEMP_volts2temp(double Vout)
{
    const double Vs = 5.0; // assuming the supply is 5V - measure Vref2 to check
    const double beta = 3435.0;
    const double room_temp = 298.15;
    const double R_balance = 10000.0; //from LTC6811 datasheet p.85. note: this doesn't account for tolerance. to be exact, measure the 10k resistor with a multimeter
    const double R_room_temp = 10000.0; //resistance at room temperature (25C)
    double R_therm = 0;
    double temp_kelvin = 0;
    double temp_celsius = 0;

    // to get the voltage in volts from the LTC6811's value,
    // multiply it by 0.0001 as below.
    R_therm = R_balance * ((Vs / (Vout * 0.0001)) - 1);
    temp_kelvin = (beta * room_temp)
        / (beta + (room_temp * log(R_therm / R_room_temp)));
    return (double)(temp_celsius = temp_kelvin - 273.15);
}

/*
    End of copy.
*/



/*
    Helper Functions for testing.
*/

void setPackdata_striped(BTM_PackData_t * pPackdata) 
{
    printf("Running setPackdata_striped().\r\n");
    for (int device_i = 0; device_i < BTM_NUM_DEVICES; ++device_i)
    {
        for (int module_i = 0; module_i < BTM_NUM_MODULES; ++module_i)
        {
            pPackdata -> stack[device_i].module[module_i].enable      = MODULE_ENABLED;
            pPackdata -> stack[device_i].module[module_i].voltage     = 2.2;
            pPackdata -> stack[device_i].module[module_i].temperature = 3;
            pPackdata -> stack[device_i].module[module_i].bal_status  = DISCHARGE_ON;
            uint16_t* pPD_enable      = &pPackdata -> stack[device_i].module[module_i].enable      ;
            uint16_t* pPD_voltage     = &pPackdata -> stack[device_i].module[module_i].voltage     ;
            uint16_t* pPD_temperature = &pPackdata -> stack[device_i].module[module_i].temperature ;
            uint16_t* pPD_bal_status  = &pPackdata -> stack[device_i].module[module_i].bal_status  ;

            //print contents after setting.
            printf("device: %2.1i  module: %2.1i  enable: %2.1i  voltage: %2.1i  temp.: %2.1i  bal_status: %2.1i\r\n",
                device_i,
                module_i,
                *pPD_enable,
                *pPD_voltage,
                *pPD_temperature,
                *pPD_bal_status);
        };
    };
    printf("\r\n");
}

void setPackdata_incrementing(BTM_PackData_t* pPackdata)
{
    printf("Running setPackdata_incrementing().\r\n");
    for (int device_i = 0; device_i < BTM_NUM_DEVICES; ++device_i)
    {
        for (int module_i = 0; module_i < BTM_NUM_MODULES; ++module_i)
        {
            pPackdata->stack[device_i].module[module_i].enable = MODULE_DISABLED;
            pPackdata->stack[device_i].module[module_i].voltage = module_i;
            pPackdata->stack[device_i].module[module_i].temperature = BTM_NUM_MODULES - module_i;
            pPackdata->stack[device_i].module[module_i].bal_status = DISCHARGE_OFF;
            uint16_t* pPD_enable = &pPackdata->stack[device_i].module[module_i].enable;
            uint16_t* pPD_voltage = &pPackdata->stack[device_i].module[module_i].voltage;
            uint16_t* pPD_temperature = &pPackdata->stack[device_i].module[module_i].temperature;
            uint16_t* pPD_bal_status = &pPackdata->stack[device_i].module[module_i].bal_status;

            //print contents after setting.
            printf("device: %2.1i  module: %2.1i  enable: %2.1i  voltage: %2.1i  temp.: %2.1i  bal_status: %2.1i\r\n",
                device_i,
                module_i,
                *pPD_enable,
                *pPD_voltage,
                *pPD_temperature,
                *pPD_bal_status);
        };
    };
    printf("\r\n");
};

//Note that message series size refers to the number of messages in one series,
//while     message array  size refers to the length of a single message wrt units of bytes.
void initMessageSeries(
    Brightside_CAN_MessageSeries* seriesStruct, 
    Brightside_CAN_Message* messageWiseContent, 
    uint8_t messageArrays[PH_MESSAGE_SERIES_LENGTH][PH_MESSAGE_ARRAY_SIZE], 
    int messageSeriesSize,  
    int messageArraySize)
{
#ifdef CODEWORD_DEBUG_BRIGHTSIDE
    printf("Running initMessageSeries().\r\n");
    printf("Initialising header.\r\n");
#endif
    CAN_InitHeaderStruct(messageWiseContent, messageSeriesSize);

#ifdef CODEWORD_DEBUG_BRIGHTSIDE
    printf("Assigning messageArray to messageSeries struct\r\n");
#endif
    for (int series_i = 0; series_i < PH_MESSAGE_SERIES_LENGTH; ++series_i)
    {
        messageWiseContent[series_i].dataFrame = &messageArrays[series_i];
    };

    seriesStruct->message           = messageWiseContent;
    seriesStruct->runningIndex      = 0;
    seriesStruct->messageSeriesSize = messageArraySize;

#ifdef CODEWORD_DEBUG_BRIGHTSIDE
    printf("End of initMessageSeries().\r\n\r\n");
#endif
};

void checkMessageStructLinks(
    Brightside_CAN_MessageSeries* PH_messageSeries,
    Brightside_CAN_Message* PH_message,
    uint8_t messageArray[PH_MESSAGE_SERIES_LENGTH][PH_MESSAGE_ARRAY_SIZE]
)
{
    printf("Running checkMessageStructLinks().\r\n");
    //initialising the message array
    printf("Initialising messageArray.\r\n");
    for (int series_i = 0; series_i < PH_MESSAGE_SERIES_LENGTH; ++series_i)
    {
        for (int message_i = 0; message_i < PH_MESSAGE_ARRAY_SIZE; ++message_i)
        {
            messageArray[series_i][message_i] = series_i + message_i;
            printf("%2.1i  ", messageArray[series_i][message_i]);
        }
        printf("\r\n");
    };

    printf("Assigning and checking messageArray in messageSeries struct.\r\n");
    for (int series_i = 0; series_i < PH_MESSAGE_SERIES_LENGTH; ++series_i)
    {
        for (int message_i = 0; message_i < PH_MESSAGE_ARRAY_SIZE; ++message_i)
        {
            printf("%2.1i  ", PH_message[series_i].dataFrame[message_i]);
        }
        printf("\r\n");
    };

    printf("0 + 1 = %2.1i  \r\n", PH_message[0].dataFrame[1]);
    printf("1 + 2 = %2.1i  \r\n", PH_message[1].dataFrame[2]);
    printf("3 + 5 = %2.1i  \r\n", PH_message[3].dataFrame[5]);

    printf("Initialising messageArray.\r\n");
    for (int series_i = 0; series_i < PH_MESSAGE_SERIES_LENGTH; ++series_i)
    {
        for (int message_i = 0; message_i < PH_MESSAGE_ARRAY_SIZE; ++message_i)
        {
            messageArray[series_i][message_i] = series_i + message_i + 20;
            printf("%2.1i  ", messageArray[series_i][message_i]);
        }
        printf("\r\n");
    };

    printf("checking messageArray in messageSeries struct.\r\n");
    for (int series_i = 0; series_i < PH_MESSAGE_SERIES_LENGTH; ++series_i)
    {
        for (int message_i = 0; message_i < PH_MESSAGE_ARRAY_SIZE; ++message_i)
        {
            printf("%2.1i  ", PH_message[series_i].dataFrame[message_i]);
        }
        printf("\r\n");
    };

    printf("0 + 1 + 20 = %2.1i  \r\n", PH_message[0].dataFrame[1]);
    printf("2 + 4 + 20 = %2.1i  \r\n", PH_message[2].dataFrame[4]);
    printf("3 + 6 + 20 = %2.1i  \r\n", PH_message[3].dataFrame[6]);

    //checking that the array of structs, PH_message[], was linked properly to the messageSeries struct
    printf("Checking that the structs are linked correctly.\r\n");
    for (int series_i = 0; series_i < PH_MESSAGE_SERIES_LENGTH; ++series_i)
    {
        for (int message_i = 0; message_i < PH_MESSAGE_ARRAY_SIZE; ++message_i)
        {
            printf("%2.1i  ", PH_messageSeries->message[series_i].dataFrame[message_i]);
        }
        printf("\r\n");
    };
    printf("changing messageArray.\r\n");
    for (int series_i = 0; series_i < PH_MESSAGE_SERIES_LENGTH; ++series_i)
    {
        for (int message_i = 0; message_i < PH_MESSAGE_ARRAY_SIZE; ++message_i)
        {
            messageArray[series_i][message_i] = series_i + message_i;
            printf("%2.1i  ", messageArray[series_i][message_i]);
        }
        printf("\r\n");
    };
    printf("Checking that the structs are linked correctly by seeing if the array updated.\r\n");
    for (int series_i = 0; series_i < PH_MESSAGE_SERIES_LENGTH; ++series_i)
    {
        for (int message_i = 0; message_i < PH_MESSAGE_ARRAY_SIZE; ++message_i)
        {
            printf("%2.1i  ", PH_messageSeries->message[series_i].dataFrame[message_i]);
        }
        printf("\r\n");
    };
    printf("End of checkMessageStructLinks().\r\n\r\n");
}

void assignAndCheckMessageStructLinks(
    Brightside_CAN_MessageSeries * PH_messageSeries,
    Brightside_CAN_Message * PH_message,
    uint8_t messageArray[PH_MESSAGE_SERIES_LENGTH][PH_MESSAGE_ARRAY_SIZE]
)
{
    printf("Running checkMessageStructLinks().\r\n");
    //creating the message structs
    CAN_InitHeaderStruct(PH_message, PH_MESSAGE_SERIES_LENGTH);

    //initialising the message array
    printf("Initialising messageArray.\r\n");
    for (int series_i = 0; series_i < PH_MESSAGE_SERIES_LENGTH; ++series_i)
    {
        for (int message_i = 0; message_i < PH_MESSAGE_ARRAY_SIZE; ++message_i)
        {
            messageArray[series_i][message_i] = series_i + message_i;
            printf("%2.1i  ", messageArray[series_i][message_i]);
        }
        printf("\r\n");
    };

    printf("Assigning and checking messageArray in messageSeries struct\r\n");
    for (int series_i = 0; series_i < PH_MESSAGE_SERIES_LENGTH; ++series_i)
    {
        PH_message[series_i].dataFrame = &messageArray[series_i];
        for (int message_i = 0; message_i < PH_MESSAGE_ARRAY_SIZE; ++message_i)
        {
            printf("%2.1i  ", PH_message[series_i].dataFrame[message_i]);
        }
        printf("\r\n");
    };

    printf("0 + 1 = %2.1i  \r\n", PH_message[0].dataFrame[1]);
    printf("1 + 2 = %2.1i  \r\n", PH_message[1].dataFrame[2]);
    printf("3 + 5 = %2.1i  \r\n", PH_message[3].dataFrame[5]);

    printf("Initialising messageArray.\r\n");
    for (int series_i = 0; series_i < PH_MESSAGE_SERIES_LENGTH; ++series_i)
    {
        for (int message_i = 0; message_i < PH_MESSAGE_ARRAY_SIZE; ++message_i)
        {
            messageArray[series_i][message_i] = series_i + message_i + 20;
            printf("%2.1i  ", messageArray[series_i][message_i]);
        }
        printf("\r\n");
    };

    printf("Assigning and checking messageArray in messageSeries struct\r\n");
    for (int series_i = 0; series_i < PH_MESSAGE_SERIES_LENGTH; ++series_i)
    {
        PH_message[series_i].dataFrame = &messageArray[series_i];
        for (int message_i = 0; message_i < PH_MESSAGE_ARRAY_SIZE; ++message_i)
        {
            printf("%2.1i  ", PH_message[series_i].dataFrame[message_i]);
        }
        printf("\r\n");
    };

    printf("0 + 1 + 20 = %2.1i  \r\n", PH_message[0].dataFrame[1]);
    printf("2 + 4 + 20 = %2.1i  \r\n", PH_message[2].dataFrame[4]);
    printf("3 + 6 + 20 = %2.1i  \r\n", PH_message[3].dataFrame[6]);

    //setting up the rest of the message series struct
    PH_messageSeries -> message = PH_message; //assigns address of message-wise content to series struct
    PH_messageSeries -> runningIndex = 0;
    PH_messageSeries -> messageSeriesSize = PH_MESSAGE_SERIES_LENGTH;

    //checking that the array of structs, PH_message[], was linked properly to the messageSeries struct
    printf("Checking that the structs are linked correctly.\r\n");
    for (int series_i = 0; series_i < PH_MESSAGE_SERIES_LENGTH; ++series_i)
    {
        for (int message_i = 0; message_i < PH_MESSAGE_ARRAY_SIZE; ++message_i)
        {
            printf("%2.1i  ", PH_messageSeries -> message[series_i].dataFrame[message_i]);
        }
        printf("\r\n");
    };
    printf("Initialising messageArray.\r\n");
    for (int series_i = 0; series_i < PH_MESSAGE_SERIES_LENGTH; ++series_i)
    {
        for (int message_i = 0; message_i < PH_MESSAGE_ARRAY_SIZE; ++message_i)
        {
            messageArray[series_i][message_i] = series_i + message_i;
            printf("%2.1i  ", messageArray[series_i][message_i]);
        }
        printf("\r\n");
    };
    printf("Checking that the structs are linked correctly.\r\n");
    for (int series_i = 0; series_i < PH_MESSAGE_SERIES_LENGTH; ++series_i)
    {
        for (int message_i = 0; message_i < PH_MESSAGE_ARRAY_SIZE; ++message_i)
        {
            printf("%2.1i  ", PH_messageSeries -> message[series_i].dataFrame[message_i]);
        }
        printf("\r\n");
    };
    printf("\r\n");
}

void printMessageContent() {
    return;
}

void PH_CAN_FillDataFrames_TESTING(Brightside_CAN_MessageSeries* CANmessages_elithionSeries, BTM_PackData_t* pPH_PACKDATA)
{
    CAN_CompileMessage623(CANmessages_elithionSeries->message[0].dataFrame, pPH_PACKDATA);
    CAN_CompileMessage627(CANmessages_elithionSeries->message[1].dataFrame, pPH_PACKDATA);

    //print contents of dataFrame for testing purposes
    for (int i = 0; i < PH_SERIES_SIZE; ++i)
    {
        for (int j = 0; i < CAN_BRIGHTSIDE_DATA_LENGTH; ++i)
        {
            printf("Address: %u    Index: %i    Data: %u /r/n",
                CANmessages_elithionSeries->message[i].header.StdId,
                j,
                CANmessages_elithionSeries->message[i].dataFrame[i]);
        }
    }
}
