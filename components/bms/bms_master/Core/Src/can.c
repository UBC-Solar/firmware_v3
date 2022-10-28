/**
 *  @file can.c
 *  @brief CAN drivers for the BMS
 *
 *  @date 
 *  @author
 */


#include "CANbus_functions.h"

/*============================================================================*/
/* PRIVATE FUNCTION PROTOTYPES */



/*============================================================================*/
/* CAN DRIVER FUNCTIONS */


/**
 * @brief 
 *
 * @param 
 */
void CAN_InitTxMessages(CAN_Tx_Messages_t txMessageArray[NUM_CAN_MESSAGES]); {
    for(int i=0; i<NUM_CAN_MESSAGES; i++) {
        txMessageArray[i].tx_header.StdId = INITIAL_MESSAGE_INDEX + i;
        txMessageArray[i].tx_header.ExtId = 0;
        txMessageArray[i].tx_header.IDE = CAN_ID_STD;
        txMessageArray[i].tx_header.RTR = CAN_RTR_DATA;
        txMessageArray[i].tx_header.DLC = MAX_CAN_DATAFRAME_BYTES;
        txMessageArray[i].tx_header.TransmitGlobalTime = DISABLE;
    }
}    

/**
 * @brief 
 *
 * @param 
 */
void CAN_CompileMessage623(CAN_Tx_Message_t message623, BTM_PackData_t *pack); {
    uint16_t // defining range of voltage in pack
        totalPackVoltage = 0;
        minModuleVoltage = 65535; 
        maxModuleVoltage = 0; 
    uint8_t // define module ID
        minModule = 0;
        maxModule = 0;
        minStack = 0;
        maxStack = 0;  
    
    for(int i = 0; i < BTM_NUM_DEVICES; i++) { // initializing cell IDs in stack 1      
        for(int j = 0; j < BTM_NUM_MODULES; j++) { // initializing cell IDs in stack 2
            if(pPACKDATA -> stack[i].module[j].enable == MODULE_ENABLED) {
                moduleVoltage = (pPACKDATA -> stack[i].module[j].voltage);
                if(moduleVoltage < minModuleVoltge) {
                    minModuleVoltage = moduleVoltage;
                    minStack = i;
                    minModule = j;
                }
                if(moduleVoltage > maxModuleVoltge) {
                    maxModuleVoltage = moduleVoltage;
                    maxStack = i;
                    maxModule = j;
                }   
            }
        }
    }
}

//TODO: implement reciept of CAN message (in CAN init, probably need to add filter initilization)
void CAN_RecieveMessage()


/*============================================================================*/
/* HELPER FUNCTIONS */