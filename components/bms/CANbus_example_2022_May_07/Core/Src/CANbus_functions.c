/*

CANbus_functions.c
Author: Edward Ma (Github: RootBeer1313)

This file contains the suite of functions used for CAN functionality.

*/



#ifdef ATOM_SYMBOLS_LABELLING
#define Include_statements
#endif

#include "CANbus_functions.h"
#include "Pack_packdata.h"
//#include <stdbool.h>



#ifdef ATOM_SYMBOLS_LABELLING
#define static_and_global_variables
#endif

CAN_HandleTypeDef * Brightside_CAN_handle; // &hcan //global pointer for easy inclusion.
BTM_PackData_t * CAN_PACKDATA_POINTER; // &pack

//BTM_PackData_t PH_PACKDATA;
static uint32_t STATIC_lastInterval;
static uint32_t STATIC_lastSubInterval;
static Brightside_CAN_MessageSeries STATIC_ElithionSeries;
static Brightside_CAN_Message STATIC_messagesWiseContent[CAN_ELITHION_MESSAGE_SERIES_SIZE];
static uint8_t STATIC_messageArrays[CAN_ELITHION_MESSAGE_SERIES_SIZE][CAN_BRIGHTSIDE_DATA_LENGTH] = {{0}};

//uint8_t LUT_moduleStickers[BTM_NUM_DEVICES][BTM_NUM_MODULES] =
//    {
//        { 1, 2, 3, 4, 5,91, 6, 7, 8, 9,10,92},
//        {11,12,13,14,15,93,16,17,18,19,20,94},
//        {21,22,23,24,25,26,27,28,29,30,31,32}
//    };


/**
\brief      Object Purpose: This struct holds the physical labelling -- the stickers -- of the battery modules.

\note       Stickers 91 and 92 don't actually have an actual battery module.
            They are in this list to indicate if the placeholder in the struct
            we pull battery info from is being used, which would be something
            we need to debug and avoid.
*/
uint8_t LUT_moduleStickers[BTM_NUM_DEVICES][BTM_NUM_MODULES] =
{
//    0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17
    { 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,91,92},
    {17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,93,94}
};
    //note that the 9X numbers (91, 92, 93, 94) indicate the garbage channels.
    //Should any of the 9X numbers appear in a CAN message, something is not right.


// uint8_t LUT_moduleStickers[BTM_NUM_DEVICES][BTM_NUM_MODULES] =
// {
// //    0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17
//     { 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18},
//     {19,20,21,22,23,24,25,26,27,28,29,30,31,32,91,92,93,94}
// };

//function prototypes
#define CODEWORD_DEBUG_BRIGHTSIDE
#ifdef CODEWORD_DEBUG_BRIGHTSIDE
/**
\note
    I think this prototype was included for the sake of testing cubeIDE
    compilation. Needs to be double checked.
*/
double BTM_TEMP_volts2temp(double vout);
#endif //CODEWORD_DEBUG_BRIGHTSIDE



//function definitions


/*******************************
*
*      TOP LEVEL FUNCTIONS
*
********************************/
#ifdef ATOM_SYMBOLS_LABELLING
#define TOP_level_functions;
#endif

/**
*@brief
*Function Name: CAN_initStructsAndStuff
*
*@details
*Function purpose:
*    To be the one and only function to call to
*    initialise most things for CAN functionality
*    for UBC Solar's brightside car.
*
*@note
*Difference between this function and cubeMX code:
*    Code generated by cubeMX handles the peripheral
*    hardware initialisation.
*    This function handles the creation and organisation
*    of message structures preinitialised as global static
*    variables, arrays, and structs.
*/
extern Brightside_CAN_MessageSeries* CAN_initStructsAndStuff(CAN_HandleTypeDef * hcan)
{
    //initialize handles from cubeMX
	Brightside_CAN_handle = hcan;

	//Initialize static variables
    STATIC_lastInterval = 0;
    STATIC_lastSubInterval = 0;
    //STATIC_messageArrays = 0;

    //initialize message series structures
    CAN_InitMessageSeries_Dynamic(
        &STATIC_ElithionSeries,
        STATIC_messagesWiseContent,
        STATIC_messageArrays,
        CAN_ELITHION_MESSAGE_SERIES_SIZE);

    return &STATIC_ElithionSeries;
}


/**
@brief
Function Name: CAN_InitHeaderStruct

@details
Function Purpose:
    Initialise the aspects of a CAN message that don't change
    after initialisation, as defined by HAL for STM32.

@param Brightside_CAN_Message * CANmessageWiseContent
@n          This input is assumed to be the first entry of an array of type Brightside_CAN_Message.
@n			Note that each element of the array of Brightside_CAN_message is a unique CAN message of type Brightside_CAN_message.
@param int messageSeriesSize
@n          This input is the array-length of the Brightside_CAN_Message array

@note
Design Notes:
    Note that by default, the message addresses are assigned sequential
    addresses, before being overwritten by separate assignments.

@note
    Currently, this function is meant for specific messages with CAN IDs of 622,623,626, and 627.
    This is a lazy bandaid feature and should be moved to a new function if more message series are added.
*/
void CAN_InitHeaderStruct(Brightside_CAN_Message * CANmessageWiseContent, int messageSeriesSize)
{
    Brightside_CAN_Message* elementAddress = CANmessageWiseContent;
    for(int i=0 ; i < messageSeriesSize ; ++i)
    {
        elementAddress -> header.StdId = CAN_INITIAL_ELITHION_SERIES_ADDRESS + i;
        elementAddress -> header.ExtId = CAN_UNUSED_EXT_ID;
        elementAddress -> header.IDE   = CAN_ID_STD;   //Predefined constant in stm32 include file.
        elementAddress -> header.RTR   = CAN_RTR_DATA; //Predefined constant in stm32 include file.
        elementAddress -> header.DLC   = CAN_BRIGHTSIDE_DATA_LENGTH;
        elementAddress -> header.TransmitGlobalTime = DISABLE;//We could use this eventually, if we use a custom message format

        ++elementAddress;
    }

    //separate assignments for setting unique, non-consecutive addresses.
    (CANmessageWiseContent + 0) -> header.StdId = ADDRESS_622;
    (CANmessageWiseContent + 1) -> header.StdId = ADDRESS_623;
    (CANmessageWiseContent + 2) -> header.StdId = ADDRESS_626;
    (CANmessageWiseContent + 3) -> header.StdId = ADDRESS_627;
}


/*

*/


/**
@brief
Function Name:  CAN_InitMessageSeries_Dynamic()

\par
Function Purpose: link pre-defined structs and arrays together

@param      Brightside_CAN_MessageSeries * seriesStruct:
@n              The highest-level struct containing everything else
@param      Brightside_CAN_Message * messageWiseContent:
@n              An array of structs, each element containing a message hedaer,
                dataFrame array, and mailbox.
@param      uint8_t messageArrays[CAN_ELITHION_MESSAGE_SERIES_SIZE][CAN_BRIGHTSIDE_DATA_LENGTH]:
@n              A 2D array for easy assignment of message dataFrames to each struct.
@param      int messageSeriesSize:
@n              The total number of messages in one series.

@par        Algorithm
      1) Initialise headers wrt messageSeriesSize.
@n    2) Assign to each message-struct a pointer reference to a
    dataFrame, stored in the 2D array.
@n    3) Assign to the messageSeries struct the message struct,
       and initialise the runningIndex and the messageSeriesSize.
*/
void CAN_InitMessageSeries_Dynamic(
        Brightside_CAN_MessageSeries * seriesStruct,
        Brightside_CAN_Message * messageWiseContent,
        uint8_t messageArrays[CAN_ELITHION_MESSAGE_SERIES_SIZE][CAN_BRIGHTSIDE_DATA_LENGTH],
        int messageSeriesSize)
{

    CAN_InitHeaderStruct(messageWiseContent, messageSeriesSize);

    //intended function: assign to the Message struct the first address of the 2nd dimension of the message array
    for (int series_i = 0; series_i < messageSeriesSize; ++series_i)
    {
    	//reminder: only giving the first dimension effectively passes the first address
    	//of a 1D array that contains the 2nd dimension's data.
        messageWiseContent[series_i].dataFrame = messageArrays[series_i];
    }

    seriesStruct->message = messageWiseContent;
    seriesStruct->runningIndex = 0;
    seriesStruct->messageSeriesSize = messageSeriesSize;

}

/**
@brief      Function Name: CAN_main
@details    Function Purpose: TO limit the number of CANstate calls to at most 5 times per second.

@param      Brightside_CAN_MessageSeries * pSeries : pointer to the full messageSeries struct.

@par        GLOBAL_variables_used:
                uint32_t STATIC_lastInterval : Keeps track of the last 1.0s interval.
@n              uint32_t STATIC_lasSubInterval : keeps track of the last 0.2s interval.

@return     BTM_Error

@par        Algorithm:

    1) Calculate tickDelta and tickSubDelta.
    2) Check if a 1.0s interval has passed.
    2.1) if so, run extra functions, update lastInterval, then return.
    3) Else, check if a 0.2s interval has passed.
    3.1) if not, return.
    3.2) if so, run only queue(), update lastSubInterval, then return.

@note
    Design Notes:
        - Note that lastInterval and lastSubInterval are always multiples of their
          intervals, 1.0s and 0.2s respectively. This is to make it easier to debug.

        - this function used to be called "CANstate_EntryCheck"
*/


extern HAL_StatusTypeDef CAN_main(Brightside_CAN_MessageSeries * pSeries)
{
    uint32_t
        tickValue = HAL_GetTick(),
        tickDelta,
        tickSubDelta;
    HAL_StatusTypeDef
        status = HAL_OK;

    //gets the absolute difference between tickValue and lastInterval
    //avoids counter reset edge-case
    if(tickValue >= STATIC_lastInterval)
    {
        tickDelta = tickValue - STATIC_lastInterval;
    }
    else //if(tickValue < lastInterval) //if overflow
    {
        tickDelta = PH_MAX_VALUE - STATIC_lastInterval + tickValue;
    }

    //gets the absolute difference between tickValue and lastSubInterval
    if(tickValue >= STATIC_lastSubInterval)
    {
        tickSubDelta = tickValue - STATIC_lastSubInterval;
    }
    else //if(tickValue < lastSubInterval)
    {
        tickSubDelta = PH_MAX_VALUE - STATIC_lastSubInterval + tickValue;
    }

    //check if called at 1.0s interval or greater
    //if so, run additional functions.
    if(tickDelta >= ONE_THOUSAND_MILLISECONDS)
    {
        //update lastInterval to be a multiple of 1.0s.
        STATIC_lastInterval = tickValue - (tickValue % ONE_THOUSAND_MILLISECONDS);
        if(CAN_staleCheck() != CAN_NOT_STALE)
        {
            //at this moment, there isn't anything special to do with stale messages
        }
        CAN_compileAllMessages(pSeries);
        CAN_resetRequestQueue(pSeries);
        status = CAN_requestQueue(pSeries);
        // if(status != HAL_OK)
        // {
        //     return status;
        // }
        // else
            return status;
    }

    else if(tickSubDelta < TWO_HUNDRED_MILLISECONDS)
    {
        return status;
    }

    else //if(tickSubDelta >= TWO_HUNDRED_MILLISECONDS)
    {
        //update lastSubInterval to be a multiple of 0.2s.
        STATIC_lastSubInterval = tickValue - (tickValue % TWO_HUNDRED_MILLISECONDS);
        status = CAN_requestQueue(pSeries);
        // if(status != HAL_OK)
        // {
        //     return status;
        // }
        // else
            return status;
    }
}



/**
@brief      Function Name: CAN_staleCheck
@details    Function Purpose: check if the CAN-bus mailboxes still contain messages from the previous interval.

@param      None.

@returns    returns 1 if there is stale data
@returns    else, returns 0 if the mailboxes are empty, i.e. without stale data to send.
*/
uint8_t CAN_staleCheck() //PH_ removed "static inline" to allow compilation. Consider adding keywords later or refactoring this function to be inline.
{
    if(HAL_CAN_GetTxMailboxesFreeLevel(Brightside_CAN_handle) != 3)
    {
        return CAN_STALE;
    }
    return CAN_NOT_STALE;
}

/**
@brief      Function Name: CAN_compileAllMessages

@details    Function Purpose:
            Run all functions that compile messages.
            Each message contains different data arranged in different orders,
            hence why each message has a unique compile function.

@param      Brightside_CAN_MessageSeries * pSeries : pointer to message series struct.
@return     None.

@note
Design Notes:
    Each CompileMessage() function call takes the message[].dataFrame element of the struct parameter.
*/
void CAN_compileAllMessages(Brightside_CAN_MessageSeries * pSeries)
{
    // CAN_CompileMessage622(pSeries->message[0].dataFrame, CAN_PACKDATA_POINTER);
    // CAN_CompileMessage623(pSeries->message[1].dataFrame, CAN_PACKDATA_POINTER);
    // CAN_CompileMessage626(pSeries->message[2].dataFrame, CAN_PACKDATA_POINTER);
    // CAN_CompileMessage627(pSeries->message[3].dataFrame, CAN_PACKDATA_POINTER);
}

/**
@brief      Function Name: CAN_requestQueue
@details    Function Purpose:
                Queue messages within a message series into the 3 transmission
                mailboxes of the stm32 chip we're using.

@param      Brightside_CAN_MessageSeries * pSeries

@returns    HAL_StatusTypeDef status
                - if the message queuing fails, then
                  HAL_OK is not the return.

@par        Algorithm:

@note       Design Notes: This function should NOT reset the runningIndex.
*/
HAL_StatusTypeDef CAN_requestQueue(Brightside_CAN_MessageSeries * pSeries)
{

    //else

    HAL_StatusTypeDef
        status = HAL_OK;
    uint8_t attempt = 0;

    int messageIndex = pSeries -> runningIndex;
    if(messageIndex >= pSeries->messageSeriesSize)
    {
        return HAL_OK;
    }
    while
        (HAL_CAN_GetTxMailboxesFreeLevel(Brightside_CAN_handle) > 0
         && messageIndex < pSeries->messageSeriesSize)
    {
#if CAN_ENABLE_REQUEST_QUEUE_REDUNDANCY == 1
        //attempt transmission at most three times
        do{
            status =
                HAL_CAN_AddTxMessage
                    (
                    Brightside_CAN_handle,
                    &pSeries->message[messageIndex].header,
                    pSeries->message[messageIndex].dataFrame,//intent: pass the array using call by value.
                    &pSeries->message[messageIndex].mailbox
                    );
            attempt++;
        }
        while(status != HAL_OK && attempt < CAN_REQUEST_ATTEMPT_MAX);

        //if all three transmission retrys fail.
        if(status != HAL_OK)
        {
            pSeries -> runningIndex = messageIndex;
            return status;
        }

#else
        status =
            HAL_CAN_AddTxMessage
                (
                Brightside_CAN_handle,
                &pSeries->message[messageIndex].header,
                pSeries->message[messageIndex].dataFrame,//intent: pass the array using call by value.
                &pSeries->message[messageIndex].mailbox
                );
#endif

        messageIndex++;
    }

    pSeries -> runningIndex = messageIndex;
    return status;
}

/**
@note This is the function that resets the running index in the struct
*/
void CAN_resetRequestQueue(Brightside_CAN_MessageSeries * pSeries)//PH_ removed "static inline" to allow compilation. Consider adding keywords later or refactoring this function to be inline.
{
    pSeries -> runningIndex = 0;
}

/*******************************
*
*      HELPER FUNCTIONS
*
********************************/
#ifdef ATOM_SYMBOLS_LABELLING
#define HELPER_functions;
#endif
/**
@brief  Function name: CAN_CompileMessage622

@par    Function purpose:
        Retrieve data, translate it, then format it into a message matching Elithion's format.
@n      See the website for formatting details: https://www.elithion.com/lithiumate/php/controller_can_specs.php

@par    Algorithm:

    1) Retrieve fault flags specified.
    2) Place fault flag data into Elithion format.
    3) Place data into message array, while following Elithion format.
*/
void CAN_CompileMessage622(uint8_t aData_series622[CAN_BRIGHTSIDE_DATA_LENGTH])
{
    uint8_t
        stateBYTE           = 0,
        //timerBYTE           = 0,
        //flagsBYTE           = 0,
        faultCodeBYTE       = 0,
        levelFaultFlagsBYTE = 0,
        warningFlagsBYTE    = 0;



    int
        // status_var = pPACKDATA->PH_status; //should access the variable that summarizes the whole pack's warning and fault flags
				status_var = Pack_getFaultAndWarningStatusBits();
    /*
    Update stateBYTE.
    */
    //Bit 0: fault state.
    if( (status_var & CAN_FAULT_VALUES) != 0)
    {
        stateBYTE |= CAN_BITFLAG_FAULT_STATE;
    }

    /*
    Update faultCodeBYTE.
    */

    // if(status_var & BMS_FAULT_COMM != 0)
    // if(status_var & BMS_FAULT_OT != 0)
    // if(status_var & BMS_FAULT_UV != 0)
    // if(status_var & BMS_FAULT_OV != 0)
    // if(status_var & BMS_FAULT_NO_VOLT != 0)


/*
Update levelFaultFlagsBYTE.
*/
    // Bit 7: Over voltage.
    if((status_var & BMS_FAULT_OV) != 0)
    {
        levelFaultFlagsBYTE |= CAN_FAULTFLAG_OVERVOLTAGE;
    }
    // Bit 6: Under voltage.
    if((status_var & BMS_FAULT_UV) != 0)
    {
        levelFaultFlagsBYTE |= CAN_FAULTFLAG_UNDERVOLTAGE;
    }

    // Bit 5: Over-temperature.
    if((status_var & BMS_FAULT_OT) != 0)
    {
        levelFaultFlagsBYTE |= CAN_FAULTFLAG_OVERTEMP;
    }

    // Bit 4: Discharge overcurrent.
    // Bit 3: Charge overcurrent.

    // Bit 2: Communication fault with a bank or cell.
    if((status_var & BMS_FAULT_COMM) != 0)
    {
        levelFaultFlagsBYTE |= CAN_FAULTFLAG_COMMFAULT;
    }
    // Bit 1: Interlock is tripped.
    // Bit 0: Driving off while plugged in.

    /*
    Update warningFlagsBYTE.
    */

    // Bit 7 : isolation fault.
    // Bit 6 : low SOH.
    // Bit 5 : hot temperature.
    if((status_var & BMS_WARNING_HIGH_T) != 0)
    {
        warningFlagsBYTE |= CAN_WARNFLAG_HIGHTEMP;
    }

    // Bit 4 : cold temperature.
    if((status_var & BMS_WARNING_LOW_T) != 0)
    {
        warningFlagsBYTE |= CAN_WARNFLAG_LOWTEMP;
    }

    // Bit 3 : discharge overcurrent.
    // Bit 2 : charge overcurrent.
    // Bit 1 : high voltage.
    if((status_var & BMS_WARNING_HIGH_V) != 0)
    {
        warningFlagsBYTE |= CAN_WARNFLAG_HIGHVOLTAGE;
    }
    // Bit 0 : low voltage.
    if((status_var & BMS_WARNING_LOW_V) != 0)
    {
        warningFlagsBYTE |= CAN_WARNFLAG_LOWVOLTAGE;
    }

    //setting byte order in aData_series622 array
    aData_series622[0] = stateBYTE;
    //aData_series623[1] = timerBYTE;
    //aData_series623[2] = timerBYTE;
    //aData_series622[3] = flagsBYTE;
    aData_series622[4] = faultCodeBYTE;
    aData_series622[5] = levelFaultFlagsBYTE;
    aData_series622[6] = warningFlagsBYTE;
    //aData_series623[7] = outOfBounds;

  //end of function
}

/**
\brief
Function name: CAN_CompileMessage623
\par    Function purpose:
        Retrieve data, translate it, then format it into a message matching Elithion's format.
\n      See the website for formatting details: https://www.elithion.com/lithiumate/php/controller_can_specs.php

\par Algorithm:

        1) Retrieve voltage data specified.
                Note: this step does its best to maintain the original data type.
        2) Translate gathered data into expected units and cast into uint8_t.
                Note: this step is where data is made to match Elithion format's
                units, and where the numbers are casted to uint8_t.
        3) Place data into message array, while following Elithion format.
*/
void CAN_CompileMessage623(uint8_t aData_series623[CAN_BRIGHTSIDE_DATA_LENGTH])
{
    uint32_t
        packVoltage = 0;
    uint16_t
        minVtg = 0,
        maxVtg = 0;
    uint8_t
        minStack = 0,
        minModule = 0,
        maxStack = 0,
        maxModule = 0;
    uint8_t
        minVtgBYTE = 0,
        maxVtgBYTE = 0,
        minBattModuleSticker = 0,
        maxBattModuleSticker = 0;
    uint8_t
        outOfBounds = 0;
    float
//        packVoltageFLOAT = 0,
        minVtgFLOAT = 0,
        maxVtgFLOAT = 0;
    voltageInfoStruct *
        voltageInfoPtr = NULL;

  //Collecting and translating the collected data into CAN frame format

  //gather min and max voltages
    voltageInfoPtr = Pack_getVoltageInfo();
    minVtg      = voltageInfoPtr->MinVoltage;
    maxVtg      = voltageInfoPtr->MaxVoltage;
    minStack    = voltageInfoPtr->MinStackIndex;
    minModule   = voltageInfoPtr->MinModuleIndex;
    maxStack    = voltageInfoPtr->MaxStackIndex;
    maxModule   = voltageInfoPtr->MaxModuleIndex;

    packVoltage = Pack_getPackVoltage();
    if(packVoltage < +0)
    {
        //this would check for out of bounds events, but idk if we care enough to raise a fault or warning flag
    }

    //Convert units of 100uV to V.
    //Then check if value is out of out of expected bounds, and cast uint16_t to uint8_t.

    // minVtgFLOAT = (float)minVtg * 0.0001;
    // minVtgBYTE = checkAndConvertModuleVoltage(minVtgFLOAT);

    minVtgBYTE = CAN_convertVoltage_100uVto100mV(minVtg);

    // maxVtgFLOAT = (float)maxVtg * 0.0001;
    // maxVtgBYTE = checkAndConvertModuleVoltage(maxVtgFLOAT);

    maxVtgBYTE = CAN_convertVoltage_100uVto100mV(maxVtg);

    minBattModuleSticker = CAN_lookupModuleSticker(minStack,minModule);
    maxBattModuleSticker = CAN_lookupModuleSticker(maxStack,maxModule);

    //setting byte order in aData_series623 array
    aData_series623[0] = (uint8_t)(packVoltage >> 8);//intent: most-sig half of pack_voltage is bit-shifted right by 8 bits, such that ONLY the MSH is casted.
    aData_series623[1] = (uint8_t)(packVoltage);     //intent: only the LSB half is stored. the MSB half is truncated by the casting.
    aData_series623[2] = (uint8_t)minVtgBYTE;                 //NOTE: the voltage is in units of 100mV
    aData_series623[3] = (uint8_t)minBattModuleSticker;
    aData_series623[4] = (uint8_t)maxVtgBYTE;                 //NOTE: the voltage is in units of 100mV
    aData_series623[5] = (uint8_t)maxBattModuleSticker;
    //aData_series623[6] = 0;                        //redundant
    aData_series623[7] = (uint8_t)outOfBounds;

		// return ;
  //end of function
}

/**
\brief
Function name: CAN_CompileMessage626

\par    Function purpose:
\n      Retrieve data, translate it, then format it into a message matching Elithion's format.
\n      See the website for formatting details: https://www.elithion.com/lithiumate/php/controller_can_specs.php

\par    Algorithm:

        1) Retrieve state of charge
        2) Place data into message array, while following Elithion format.
*/
void CAN_CompileMessage626(uint8_t aData_series626[CAN_BRIGHTSIDE_DATA_LENGTH])
{
    uint8_t StateOfChargeBYTE;

    // StateOfChargeBYTE = pPACKDATA->PH_SOC_LOCATION;
		StateOfChargeBYTE = Pack_getSOC();

    //setting byte order in aData_series626 array
    aData_series626[0] = StateOfChargeBYTE;
    // aData_series623[1] = 0;
    // aData_series623[2] = 0;
    // aData_series626[3] = 0;
    // aData_series626[4] = 0;
    // aData_series626[5] = 0;
    // aData_series626[6] = 0;
    // aData_series623[7] = 0;
}

/**
\brief
Function name: CAN_CompileMessage627

\par        Function purpose:
            To pull/process information from structs, and format it into a CAN-ready message.
\n          In this case, the information specified is:
             - average temperature
             - min temperature
             - max temperature
             - cell number with min temp
             - cell number with max temp

\param      uint8_t aData_series627[8] - Array pre-initialised with zeros.
\param      BTM_PackData_t * pPACKDATA - pointer to struct containing battery-module measurements.

\return     void

\par    \"Output\":

        aData_series627[8] should have data in the proper order as per the Elithion format.
        See the website: https://www.elithion.com/lithiumate/php/controller_can_specs.php

\par    Algorithm

        1) Gather/calculate information from structs.
                Note: this step does its best to maintain the original data type.
        2) Translate gathered information to expected units and data type of uint8_t.
                Note: this step is where data is made to match Elithion format's units,
                and where the numbers are casted to uint8_t.
        3) Place data into message array, while matching the Elithion format.

*/
void CAN_CompileMessage627(uint8_t aData_series627[CAN_BRIGHTSIDE_DATA_LENGTH]){
    uint8_t
        averageTemperatureBYTE = 0,
        minTmpBYTE = 0,
        maxTmpBYTE = 0,
        minTmpModuleSticker = 255,
        maxTmpModuleSticker = 255;
    uint8_t
        minTmpStack = 0,
        maxTmpStack = 0,
        minTmpModule = 0,
        maxTmpModule = 0,
        outOfBounds = 0;
    uint16_t
        averageTemperature2BYTE = 0,
        minTmp = 0,
        maxTmp = 0;

    double
        averageTemperatureDOUBLE = 0,
        minTmpDOUBLE = 0,
        maxTmpDOUBLE = 0;

    //1) scans the struct and calculates the relevant information needed
    Pack_getTemperatureInfo();

    //2) Translating Data

    averageTemperatureDOUBLE = BTM_TEMP_volts2temp((double)averageTemperature2BYTE);
    averageTemperatureBYTE = Pack_checkAndCastTemperature(averageTemperatureDOUBLE);

    minTmpDOUBLE = BTM_TEMP_volts2temp((double)minTmp);
    minTmpBYTE = Pack_checkAndCastTemperature(minTmpDOUBLE);

    maxTmpDOUBLE = BTM_TEMP_volts2temp((double)maxTmp);
    maxTmpBYTE = Pack_checkAndCastTemperature(maxTmpDOUBLE);

    minTmpModuleSticker = LUT_moduleStickers[minTmpStack][minTmpModule];
    maxTmpModuleSticker = LUT_moduleStickers[maxTmpStack][maxTmpModule];

    //3) Placing data into message array.
    aData_series627[0] = averageTemperatureBYTE;
    // aData_series627[1] = 0; //redundant
    aData_series627[2] = minTmpBYTE;
    aData_series627[3] = minTmpModuleSticker;
    aData_series627[4] = maxTmpBYTE;
    aData_series627[5] = maxTmpModuleSticker;
    // aData_series627[6] = 0; //redundant
    aData_series627[7] = outOfBounds;
}







/**
@brief      Function Name: CAN_convertVoltage_100uVto100mV
@details    Function Purpose:
            Convert a voltage in units of 100uV to units of 100mV

@param      uint16_t voltage_100uV

@returns    uint8_t voltage_100mV

@note       The math for converting from values in units of 100uV to units of 100mV:

            gpio voltage [100mV] = (cell value)[100uV] * ([V]x10^-6*100/[uV*100]) x ([mV]*10*100/[V])
                                 = (cell value)*10^-3

            or, 100uV to 100mV is the same magnitude difference as uV to mV; 10^-3
*/
uint8_t CAN_convertVoltage_100uVto100mV(uint16_t voltage_100uV)
{
    uint8_t voltage_100mV = 0;
    voltage_100mV = (uint8_t)((float)voltage_100uV * 0.001);

    return voltage_100mV;
}


/**
@brief      Function Name:  CAN_lookupModuleSticker
@details    Function Purpose:
            Lookup the sticker number for a module based on the stack index (the
            ltc6813 it's connected to) and the module index (the pin on the
            ltc6813 it's connected to)

            If the indices given are out of bounds, the max value 255 is
            returned to indicate an error.

@param      uint8_t stackIndex, moduleIndex

@returns    uint8_t LUT_moduleStickers[stackIndex][moduleIndex]

@note
*/
uint8_t CAN_lookupModuleSticker(uint8_t stackIndex, uint8_t moduleIndex)
{
    if
    (
        stackIndex  >= 0 && stackIndex  <= 1
     && moduleIndex >= 0 && moduleIndex <= 17
    )
    {
        return LUT_moduleStickers[stackIndex][moduleIndex];
    }
    //else
    {
        return 255;
    }

}
/*

Other Functions

*/
#ifdef ATOM_SYMBOLS_LABELLING
#define OTHER_functions;
#endif


// /**
// \note I don't remember why this function is here. No other function calls this function.
// */
// uint8_t celciusAverage(BTM_PackData_t * pPACKDATA){
//     uint16_t localTemperature = 0;
//     double temperatureTotal = 0;
//     double localAverage = 0;
//
//     int i = 0;
//
//     for(i = 0; i < BTM_NUM_DEVICES; ++i)
//     {
//         for(int j = 0; j < BTM_NUM_MODULES; ++j)
//         {
//             if(pPACKDATA -> stack[i].module[j].enable == 1)
//             {
//                 localTemperature = pPACKDATA -> stack[i].module[j].temperature;
//                 temperatureTotal = temperatureTotal + BTM_TEMP_volts2temp(localTemperature);
//             }
//         }
//     }
//
//     localAverage = temperatureTotal / PH_TOTAL_MUX;
//
//     if(localAverage > 255)
//     {
//         return 255;
//     }
//     else if(localAverage < 0)
//     {
//         return 0;
//     }
//     else
//     {
//         return (uint8_t)localAverage;
//     }
//
// }

#ifdef CODEWORD_DEBUG_BRIGHTSIDE
/**
\note
    I think this prototype was included for the sake of testing cubeIDE
    compilation. Needs to be double checked.
*/
double BTM_TEMP_volts2temp(double vout)
{
	return 1337.1337;
}
#endif

    //placeholder code, to see the format of struct calls.
    //Looks really clean
    //retreving voltage of module 12 of stack 3 (note the off-by-one array index)
    //uint16_t PH_ModuleVOLTAGE = PACKDATApointer -> stack[2].module[11].voltage;


    // /*self-notes:
    // POINTER2struct->member is already a dereference.
    // &POINTER2struct->member is the address of the member
    // POINTER2struct->array and &POINTER2struct->array[0] are different somehow?
    // I think POINTER2struct->array is the pointer to the first element of the array,
    // like arr == &arr[0].
    // I think &POINTER2struct->array[0] is the same as above, but &POINTER2struct->array[1] is not.
    // */
