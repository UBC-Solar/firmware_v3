#include "stm32f10x.h"

#include "LCD/LCD.h"
#include "CAN.h"
#include "XBee/XBee.h"
#include "virtual_com.h"

#define TRUE 1
#define FALSE 0

#define MC_BASE 0x500
#define BATT_BASE 0x620
#define ARR_BASE 0x700

#define FILTER_LEN 9

CAN_msg_t CAN_rx_msg;

union {
	float float_var;
	uint8_t chars[4];
}vel;

union {
	float float_var;
	uint8_t chars[4];
}cur;

union {
	float float_var;
	uint8_t chars[4];
} u;

/**
 * Initialize Dashboard LED lights
 */
void InitLEDs(void)
{
	
	RCC->APB2ENR |= 0x1UL << 2; 		//Initialize clock for GPIOA, if it hasn't been initialized yet
	GPIOA->CRL &= 0;
	GPIOA->CRH &= 0;
	GPIOA->CRL |= 0x33330033UL;			//Set pins A1, A4, A5, A6, A7 to be Push-Pull Output, 50Mhz
	GPIOA->CRH |= 0x30033333UL;			//SetBar pins A8, A9, A10 to be Push-Pull Output, 50Mhz
	
	RCC->APB2ENR |= 0x1;
	AFIO->MAPR |= 0x2 << 24;
	//GPIOA->BSRR = 0xFFFF;
	
	//GPIOA->BSRR = 0x1 << 11;
	
	GPIOA->BRR = 0xFFFF;

}

/**
 * Main procedure
 */
int main(void)
{
	
	int32_t tempInt32;
	CAN_msg_t newCanMsg;
	uint16_t acceptMessages[FILTER_LEN] = {BATT_BASE + 2, BATT_BASE + 3, BATT_BASE + 4, BATT_BASE + 6, BATT_BASE + 7, MC_BASE + 2, MC_BASE + 3, MC_BASE + 0xB, ARR_BASE};
	uint8_t c = 0;
	uint8_t d = 0;

	//RCC->APB2ENR |= 0x1 << 2;
	//GPIOA->CRL &= ~(0xFF0F00FF);
	//GPIOA->CRH &= ~(0x00000FFF);
	
	//GPIOA->CRL |= 0x33030033;
	//GPIOA->CRH |= 0x00000333;
	
		
	//GPIOA->BRR = 0x1 << 5;
		
	InitialiseLCDPins();
	ScreenSetup();
	CANInit(CAN_500KBPS);
	//CANSetFilters(acceptMessages, FILTER_LEN);
	InitLEDs();
	VirtualComInit();
	//XBeeInit();
		
	//GPIOA->BSRR = 0x1 << 5;
		
					//Battery Low:	A0
					//Battery Full: A1				
					//Battery Communications Fault: A4				
					//Battery charge over-current: A6
					//Battery discharge over-current: A7
					//Battery over-temperature: A8
					//Battery under-voltage: A9
					//Battery over-voltage: A10
					
					//Motor Fault: PA12
					

	//GPIOA->BSRR = 0x1;
		
	//GPIOA->BSRR = (CAN_rx_msg.data[6] & 0x3) || \
	//				( ( (CAN_rx_msg.data[5] >> 2) & 0x1) << 4 ) || \
	//				( ( (CAN_rx_msg.data[5] >> 4) & 0x1F) << 6);
	
	
	
	while(1)
	{
		//If a message can be received,		
		if (CANMsgAvail())
		{
			
			GPIOA->BRR = 0x1 << 5;
			CANReceive(&CAN_rx_msg);		//Receive the msg currently in buffer
			
			//TEST
			
			if (CAN_rx_msg.id >= 0x400 && CAN_rx_msg.id < 0x500)
			{
				SendInt(CAN_rx_msg.id);
				
				vel.chars[0] = CAN_rx_msg.data[0];
				vel.chars[1] = CAN_rx_msg.data[1];
				vel.chars[2] = CAN_rx_msg.data[2];
				vel.chars[3] = CAN_rx_msg.data[3];
				
				cur.chars[0] = CAN_rx_msg.data[4];
				cur.chars[1] = CAN_rx_msg.data[5];
				cur.chars[2] = CAN_rx_msg.data[6];
				cur.chars[3] = CAN_rx_msg.data[7];
				
				SendString(" ");
				SendInt( (int) (vel.float_var));
				
				SendString(" ");
				SendInt( (int) (cur.float_var * 100) );
				
				SendLine();
			}
			
			//ENDTEST
			
			GPIOA->BSRR = 0x1 << 5;
			
			//Check the CAN ID against several known IDs
			//Several need to be parsed, especially the ones designated for LCD and dashboard
			//SendInt(CAN_rx_msg.id);
			//SendLine();
			switch(CAN_rx_msg.id)
			{
				
				//Battery: Pack Voltage(For LCD Display). Values are unsigned 16-bit integers in Volts (V). Period: 1s
				//Battery: Minimum Cell Voltage (For LCD Display). Values are unsigned 8-bit integers in 100mv intervals. Period: 1s
				//Battery: Maximum Cell Voltage (For LCD Display). Values are unsigned 8-bit integers in 100mv intervals. Period: 1s
				case BATT_BASE + 3:
					UpdateScreenParameter(BATTERY_VOLTAGE_XPOS, BATTERY_VOLTAGE_YPOS, (uint16_t) (CAN_rx_msg.data[1] | CAN_rx_msg.data[0] << 8), 0);
					
					UpdateScreenParameter(BATTERY_MINVOLT_XPOS, BATTERY_MINVOLT_YPOS, (uint8_t) CAN_rx_msg.data[2] / 10, (uint8_t) CAN_rx_msg.data[2] % 10);
					
					UpdateScreenParameter(BATTERY_MAXVOLT_XPOS, BATTERY_MAXVOLT_YPOS, (uint8_t) CAN_rx_msg.data[4] / 10, (uint8_t) CAN_rx_msg.data[4] % 10);
					
					//XBeeTransmitCan(&CAN_rx_msg);								
					break;
				
				//Battery: Pack Current(For LCD Display). Values are signed 16-bit integers in Amperes (A). Period: 1s
				case BATT_BASE + 4:
					
					UpdateScreenParameter(BATTERY_CURRENT_XPOS, BATTERY_CURRENT_YPOS, (int16_t) (CAN_rx_msg.data[1] | CAN_rx_msg.data[0] << 8), 0);
					
					//XBeeTransmitCan(&CAN_rx_msg);							
					break;
				
				//Battery: Pack Maximum Temperature (For LCD Display). Values are signed 8-bit integers in Celsius (C). Period: 1s
				case BATT_BASE + 7:
					UpdateScreenParameter(BATTERY_MAXTEMP_XPOS, BATTERY_MAXTEMP_YPOS, (int8_t) CAN_rx_msg.data[4], 0);
				
					//XBeeTransmitCan(&CAN_rx_msg);	
					break;
					
				//Battery: State of Charge (For LCD Display). Values are unsigned 8-bit integers. Period: 1s
				case BATT_BASE + 6:
					
					UpdateScreenParameter(BATTERY_CHARGE_XPOS, BATTERY_CHARGE_YPOS, (int8_t) CAN_rx_msg.data[0], 0); 
					
					//This one is different; it is used to set a battery percentage bar
					//SetBar(0xFF & CAN_rx_msg.data[0], 100, CHARGE_BAR_YPOS);
				
					//XBeeTransmitCan(&CAN_rx_msg);	
					break;
				
				//NOT FULLY IMPLEMENTED
				case BATT_BASE + 30:
					
					UpdateScreenParameter(BATTERY_SUPPVOLT_XPOS, BATTERY_SUPPVOLT_YPOS, 0, 0);
					
					break;
				
				//Motor Drive Unit: Speed (For LCD Display). Values are IEEE 32-bit floating point in m/s. Period: 200ms
				case MC_BASE + 3:
					
					u.chars[0] = CAN_rx_msg.data[4];
					u.chars[1] = CAN_rx_msg.data[5];
					u.chars[2] = CAN_rx_msg.data[6];
					u.chars[3] = CAN_rx_msg.data[7];
				
					u.float_var = u.float_var * -3.6;
					tempInt32 = (int32_t) u.float_var;
				
					if (u.float_var < 0)
					{
						u.float_var = u.float_var * -1;
					}
							
					UpdateScreenParameter(MOTOR_SPEED_XPOS, MOTOR_SPEED_YPOS, tempInt32, ((uint32_t) (u.float_var * 10)) % 10 );
					
					//send the CAN message once every second
					if (d == 5)
					{
						//XBeeTransmitCan(&CAN_rx_msg);
						d = 0;
					}
					d++;
					
					break;
				
				//Motor Drive Unit: Temperature (For LCD Display). Values are IEEE 32-bit floating point in Celsius (C). Period: 1s
				case MC_BASE + 0xB:
					
					
					u.chars[0] = CAN_rx_msg.data[0];
					u.chars[1] = CAN_rx_msg.data[1];
					u.chars[2] = CAN_rx_msg.data[2];
					u.chars[3] = CAN_rx_msg.data[3];
					
					tempInt32 = (int32_t) u.float_var;
				
					while(u.float_var < 0)
					{
						u.float_var = u.float_var * -1;
					}
					
					//XBeeTransmitCan(&CAN_rx_msg);
					
					UpdateScreenParameter(MOTOR_TEMP_XPOS, MOTOR_TEMP_YPOS, tempInt32, ((uint32_t) (u.float_var * 10)) % 10 );
					break;
					
				//Motor Drive Unit: Current (For LCD Display). Values are IEEE 32-bit floating point in Amperes(A). Period: 200ms
				case MC_BASE + 2:
					
					u.chars[0] = CAN_rx_msg.data[4];
					u.chars[1] = CAN_rx_msg.data[5];
					u.chars[2] = CAN_rx_msg.data[6];
					u.chars[3] = CAN_rx_msg.data[7];
					
					tempInt32 = (int32_t) u.float_var;
				
					if (u.float_var < 0)
					{
						u.float_var = u.float_var * -1;
					}
					
					//Send the CAN message once every second
					if (c == 5)
					{
						//XBeeTransmitCan(&CAN_rx_msg);
						c = 0;
					}
					c++;
					
					UpdateScreenParameter(MOTOR_CURRENT_XPOS, MOTOR_CURRENT_YPOS, tempInt32, ((uint32_t) (u.float_var * 10)) % 10 );
					break;
				
				//DEPRECATED
				//Array: Maximum Temperature (For LCD Display). Values are 16-bit unsigned integer in Celsius(C). Period: 1s
				//case ARR_BASE + 2:
				//
				//	tempInt32 = CAN_rx_msg.data[6] << 8 | CAN_rx_msg.data[7];
				//
				//	UpdateScreenParameter(ARRAY_MAXTEMP_XPOS, ARRAY_MAXTEMP_YPOS, tempInt32, (uint32_t) (tempInt32 / 10) % 10);
				//
				//	XBeeTransmitCan(&CAN_rx_msg);
				//
				//	break;
				
				//Battery: Faults, Battery High and Battery Low (For Dashboard Indicator)
				case BATT_BASE + 2:
					
					//Turn on LEDs if the appropriate battery fault exists.
					
					//Battery Low:	A0
					//Battery Full: A1				
					//Battery Communications Fault: A4				
					//Battery charge over-current: A6
					//Battery discharge over-current: A7
					//Battery over-temperature: A8
					//Battery under-voltage: A9
					//Battery over-voltage: A10
						
					GPIOA->BSRR = (CAN_rx_msg.data[6] & 0x3) || \
									( ( (CAN_rx_msg.data[5] >> 2) & 0x1) << 4 ) || \
									( ( (CAN_rx_msg.data[5] >> 4) & 0x1F) << 6);
					
					
					/*
					//A10: Check if the high voltage bit is set, meaning battery is full
					if ( (CAN_rx_msg.data[6] >> 1) & 0x1)
					{
						//Reset pp;in A10, turn on LED
						GPIOA->BSRR = 0x1 << 10;
					}
					
					//A9: Check if the low voltage bit is set, meaning battery is low
					if ( (CAN_rx_msg.data[6] ) & 0x1 )
					{
						//Reset pin A9, turn on LED
						GPIOA->BSRR = 0x1 << 9;
					}
					
					//A1: battery over-temperature fault
					if ( ( CAN_rx_msg.data[5] >> 5 ) & 0x1 )
					{
						//Reset pin A1, turn on LED
						GPIOA->BSRR = 0x1 << 1;
					}
					
					//A4: battery discharge over-current fault
					if ( ( CAN_rx_msg.data[5] >> 4 ) & 0x1 )
					{
						//Reset pin A4, turn on LED
						GPIOA->BSRR = 0x1 << 4;
					}
										
					//A6: battery charge over-current fault
					if ( ( CAN_rx_msg.data[5] >> 3 ) & 0x1 )
					{
						//Reset pin A6, turn on LED
						GPIOA->BSRR = 0x1 << 6;
					}
					
					//A8: battery over-voltage fault
					if ( ( CAN_rx_msg.data[5] >> 7 ) & 0x1 )
					{
						//Reset pin A8, turn on LED
						GPIOA->BSRR = 0x1 << 8;
					}
					
					//A7: battery under-voltage fault
					if ( ( CAN_rx_msg.data[5] >> 6 ) & 0x1 )
					{
						//Reset pin A7, turn on LED
						GPIOA->BSRR = 0x1 << 7;
					}
					
					//A11: Communications Fault
					if ( (CAN_rx_msg.data[5] >> 2) & 0x1)
					{
						//Reset pin A11: turn on LED
						GPIOA->BSRR = 0x1 << 11;
					}
					
					//XBeeTransmitCan(&newCanMsg);
					*/
						
					break;
							
			}		
		}
		
	}
	
	
	
}
