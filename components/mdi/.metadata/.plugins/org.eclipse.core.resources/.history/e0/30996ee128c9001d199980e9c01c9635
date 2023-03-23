/**
 * Function implementations for enabling and using CAN messaging.
 */
#include "CAN.h"
#include "stm32f1xx_hal_can.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx.h"
#include <stdio.h>
#include <stdlib.h>
#include <cstdint>




/**
 * @brief Sets up the struct with the filters information
 */
void CAN_Set_Filters(CAN_FilterTypeDef* fltr){

	fltr->FilterActivation = CAN_FILTER_ENABLE; //enable the filter
	fltr->FilterBank = 10; //specifies filter bank that will be enabled between the 14 filter banks for single instance CAN
	fltr->FilterFIFOAssignment = CAN_FILTER_FIFO0; //Either CAN_FILTER_FIFO0 or CAN_FILTER_FIFO1
	fltr->FilterIdHigh = 0x401<<5; //MSBs of Filter ID (Comparing the STD ID of the incoming msg) (shifted by 5 because EXID takes the LSBs of this register
	fltr->FilterIdLow = 0; //LSBs of Filter ID
	fltr->FilterMaskIdHigh = 0x401<<5; //higher 16 bits of mask register
	fltr->FilterMaskIdHigh = 0;
	fltr->FilterMode = CAN_FILTERMODE_IDMASK; //sets either mask or list mode (NOTE: might be better to use list mode in this application)
	fltr->FilterScale = CAN_FILTERSCALE_32BIT; //either 1 32 bit filter register or 2 16 bit filter registers
	fltr->SlaveStartFilterBank = 13; //the filter bank after which the rest of the filter banks will be given to the slave (our board has 13 filter banks for CAN1)


}


/** NOT USED (using MX_CAN_Init)
 * Initializes the CAN controller with bit rate of [].
 */
 /*
 void CANInit(void)
 {
	RCC->APB1ENR |= 0x2000000UL;  // Enable CAN clock 
	RCC->APB2ENR |= 0x1UL;				 // Enable AFIO clock
	AFIO->MAPR   &= 0xFFFF9FFF;   // reset CAN remap
	AFIO->MAPR   |= 0x00004000;   //   set CAN remap, use PB8, PB9
 
	RCC->APB2ENR |= 0x8UL;				// Enable GPIOB clock
	GPIOB->CRH   &= ~(0xFFUL);
	GPIOB->CRH   |= 0xB8UL;			// Configure PB8 and PB9
	GPIOB->ODR |= 0x1UL << 8;
  
	CAN1->MCR     = 0x11UL;      // Set CAN to initialization mode
	 
	// Set bit rates 
	CAN1->BTR &= ~(((0x03) << 24) | ((0x07) << 20) | ((0x0F) << 16) | (0x1FF)); 
	CAN1->BTR |=  (((2-1) & 0x07) << 20) | (((15-1) & 0x0F) << 16) | ((2-1) & 0x1FF);
 

	// Configure Filters
	CAN1->FMR  |=   0x1UL;                // Set to filter initialization mode
	CAN1->FA1R &= ~(0x1UL);               // Deactivate filter 0
	CAN1->FS1R |=   0x1UL;                // Set first filter to single 32 bit configuration
 
	CAN1->sFilterRegister[0].FR1 = 0x0UL; // Set filter registers to 0
	CAN1->sFilterRegister[0].FR2 = 0x0UL; // Set filter registers to 0
	CAN1->FM1R &= ~(0x1UL);               // Set filter to mask mode
 
	CAN1->FFA1R &= ~(0x1UL);			  // Apply filter to FIFO 0  
	CAN1->FA1R  |=   0x1UL;               // Activate filter 0
	
	CAN1->FMR   &= ~(0x1UL);			  // Deactivate initialization mode
	CAN1->MCR   &= ~(0x1UL);              // Set CAN to normal mode 
	while (CAN1->MSR & 0x1UL); 
 
 }
 */
/** NOT USED (using HAL_CAN_RxFifo0MsgPendingCallback)
 * Decodes CAN messages from the data registers and populates a 
 * CAN message struct with the data fields.
 * 
 * @preconditions A valid CAN message is received
 * @params CAN_rx_msg - CAN message struct that will be populated
 * 
 */
 /*
 void CANReceive(CAN_msg_t* CAN_rx_msg)
 {																	
	CAN_rx_msg->id  = (CAN1->sFIFOMailBox[0].RIR >> 21) & 0x7FFUL; 	
	CAN_rx_msg->len = (CAN1->sFIFOMailBox[0].RDTR) & 0xFUL;			
	
	CAN_rx_msg->data[0] = 0xFFUL &  CAN1->sFIFOMailBox[0].RDLR;
	CAN_rx_msg->data[1] = 0xFFUL & (CAN1->sFIFOMailBox[0].RDLR >> 8);
	CAN_rx_msg->data[2] = 0xFFUL & (CAN1->sFIFOMailBox[0].RDLR >> 16);
	CAN_rx_msg->data[3] = 0xFFUL & (CAN1->sFIFOMailBox[0].RDLR >> 24);
	CAN_rx_msg->data[4] = 0xFFUL &  CAN1->sFIFOMailBox[0].RDHR;
	CAN_rx_msg->data[5] = 0xFFUL & (CAN1->sFIFOMailBox[0].RDHR >> 8);
	CAN_rx_msg->data[6] = 0xFFUL & (CAN1->sFIFOMailBox[0].RDHR >> 16);
	CAN_rx_msg->data[7] = 0xFFUL & (CAN1->sFIFOMailBox[0].RDHR >> 24);
	
	CAN1->RF0R |= 0x20UL;
 }
 */


/** NOT USED (using CAN_Tx)
 * Encodes CAN messages using the CAN message struct and populates the 
 * data registers with the sent.
 * 
 * @preconditions A valid CAN message is received
 * @params CAN_rx_msg - CAN message struct that will be populated
 * 
 */
 void CANSend(CAN_msg_t* CAN_tx_msg)
 {
	CAN1->sTxMailBox[0].TIR   = (CAN_tx_msg->id) << 21;
	
	CAN1->sTxMailBox[0].TDTR &= ~(0xF);
	CAN1->sTxMailBox[0].TDTR |= CAN_tx_msg->len & 0xFUL;
	
	CAN1->sTxMailBox[0].TDLR  = (((uint32_t) CAN_tx_msg->data[3] << 24) |
								 ((uint32_t) CAN_tx_msg->data[2] << 16) |
								 ((uint32_t) CAN_tx_msg->data[1] <<  8) |
								 ((uint32_t) CAN_tx_msg->data[0]      ));
	CAN1->sTxMailBox[0].TDHR  = (((uint32_t) CAN_tx_msg->data[7] << 24) |
								 ((uint32_t) CAN_tx_msg->data[6] << 16) |
								 ((uint32_t) CAN_tx_msg->data[5] <<  8) |
								 ((uint32_t) CAN_tx_msg->data[4]      ));

	CAN1->sTxMailBox[0].TIR  |= 0x1UL;
	while(CAN1->sTxMailBox[0].TIR & 0x1UL);
 }

 /**
 * Returns whether there are CAN messages available.
 *
 * @returns If pending CAN messages are in the CAN controller
 *
 */
 uint8_t CANMsgAvail(void)
 {
	 return CAN1->RF0R & 0x3UL;
}


void CAN_Tx(CAN_HandleTypeDef* hcan, CAN_TxHeaderTypeDef* TxHeader, uint32_t* TxMailbox, uint8_t* msg, unsigned int len){

	if(len > 8 || len == 0){
		//error handling??
		return;
	}
	TxHeader->DLC = len; //length of message to send in bytes
	TxHeader->StdId = 0x401; //Standard Identifier 0 to 0x7FF for message (temporary) (need our ID for this board)
	TxHeader->ExtId = 0; //Using Std ID so this is not needed
	TxHeader->IDE = CAN_ID_STD; //Tells it that the IDs are standard format
	TxHeader->RTR = CAN_RTR_DATA; //"Remote Transmission Request" set to data type (data or remote frame)
	TxHeader->TransmitGlobalTime = DISABLE; //I like my women how I like my TxHeader.TransmitGlobalTime variable

	HAL_CAN_AbortTxRequest(hcan, 0);
	HAL_CAN_AbortTxRequest(hcan, 1);
	HAL_CAN_AbortTxRequest(hcan, 2);
	HAL_CAN_GetTxMailboxesFreeLevel(hcan);

	if(HAL_CAN_AddTxMessage(hcan, TxHeader, msg, TxMailbox) != HAL_OK) //puts message into mailbox and checks if everything worked ok
	{
		//Error_Handler(); //impliment another error handler?

	}

	while(HAL_CAN_IsTxMessagePending(hcan, *TxMailbox)); //waits until messsage is no longer pending


}

uint32_t parse(uint8_t one, uint8_t two ,uint8_t three ,uint8_t four){
    uint32_t result;
    result = (one << 24) | (two << 16)  | (three << 8) | four;
    //puts the 4 8 bit arrays correlated with the current part of the message in to one 32 bit integer
    return result;
}
​
void CAN_process(CAN_msg_t *msg1){
    
    
    //variable declarations
    uint32_t result;
    double current;
    
    //the parsing of the 4 arrays
    //arrows are for the pointer input instead of the dot operator
    result = parse(msg1->data[4], msg1->data[5], msg1->data[6], msg1->data[7]);
    
    //giving a percentage of the recieved signal out of the maximum possible current
    current = (float) result / MAX;
​
    //mostly just for testing, but can export to wherever or do whatever we need to 
    //printf("%f", current);
}
