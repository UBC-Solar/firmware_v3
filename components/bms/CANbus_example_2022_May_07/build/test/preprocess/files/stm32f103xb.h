#include "Drivers/CMSIS/Device/ST/STM32F1xx/Include/system_stm32f1xx.h"
#include "Drivers/CMSIS/Include/core_cm3.h"
typedef enum

{



  NonMaskableInt_IRQn = -14,

  HardFault_IRQn = -13,

  MemoryManagement_IRQn = -12,

  BusFault_IRQn = -11,

  UsageFault_IRQn = -10,

  SVCall_IRQn = -5,

  DebugMonitor_IRQn = -4,

  PendSV_IRQn = -2,

  SysTick_IRQn = -1,





  WWDG_IRQn = 0,

  PVD_IRQn = 1,

  TAMPER_IRQn = 2,

  RTC_IRQn = 3,

  FLASH_IRQn = 4,

  RCC_IRQn = 5,

  EXTI0_IRQn = 6,

  EXTI1_IRQn = 7,

  EXTI2_IRQn = 8,

  EXTI3_IRQn = 9,

  EXTI4_IRQn = 10,

  DMA1_Channel1_IRQn = 11,

  DMA1_Channel2_IRQn = 12,

  DMA1_Channel3_IRQn = 13,

  DMA1_Channel4_IRQn = 14,

  DMA1_Channel5_IRQn = 15,

  DMA1_Channel6_IRQn = 16,

  DMA1_Channel7_IRQn = 17,

  ADC1_2_IRQn = 18,

  USB_HP_CAN1_TX_IRQn = 19,

  USB_LP_CAN1_RX0_IRQn = 20,

  CAN1_RX1_IRQn = 21,

  CAN1_SCE_IRQn = 22,

  EXTI9_5_IRQn = 23,

  TIM1_BRK_IRQn = 24,

  TIM1_UP_IRQn = 25,

  TIM1_TRG_COM_IRQn = 26,

  TIM1_CC_IRQn = 27,

  TIM2_IRQn = 28,

  TIM3_IRQn = 29,

  TIM4_IRQn = 30,

  I2C1_EV_IRQn = 31,

  I2C1_ER_IRQn = 32,

  I2C2_EV_IRQn = 33,

  I2C2_ER_IRQn = 34,

  SPI1_IRQn = 35,

  SPI2_IRQn = 36,

  USART1_IRQn = 37,

  USART2_IRQn = 38,

  USART3_IRQn = 39,

  EXTI15_10_IRQn = 40,

  RTC_Alarm_IRQn = 41,

  USBWakeUp_IRQn = 42,

} IRQn_Type;











typedef struct

{

  volatile uint32_t SR;

  volatile uint32_t CR1;

  volatile uint32_t CR2;

  volatile uint32_t SMPR1;

  volatile uint32_t SMPR2;

  volatile uint32_t JOFR1;

  volatile uint32_t JOFR2;

  volatile uint32_t JOFR3;

  volatile uint32_t JOFR4;

  volatile uint32_t HTR;

  volatile uint32_t LTR;

  volatile uint32_t SQR1;

  volatile uint32_t SQR2;

  volatile uint32_t SQR3;

  volatile uint32_t JSQR;

  volatile uint32_t JDR1;

  volatile uint32_t JDR2;

  volatile uint32_t JDR3;

  volatile uint32_t JDR4;

  volatile uint32_t DR;

} ADC_TypeDef;



typedef struct

{

  volatile uint32_t SR;

  volatile uint32_t CR1;

  volatile uint32_t CR2;

  uint32_t RESERVED[16];

  volatile uint32_t DR;

} ADC_Common_TypeDef;











typedef struct

{

  uint32_t RESERVED0;

  volatile uint32_t DR1;

  volatile uint32_t DR2;

  volatile uint32_t DR3;

  volatile uint32_t DR4;

  volatile uint32_t DR5;

  volatile uint32_t DR6;

  volatile uint32_t DR7;

  volatile uint32_t DR8;

  volatile uint32_t DR9;

  volatile uint32_t DR10;

  volatile uint32_t RTCCR;

  volatile uint32_t CR;

  volatile uint32_t CSR;

} BKP_TypeDef;











typedef struct

{

  volatile uint32_t TIR;

  volatile uint32_t TDTR;

  volatile uint32_t TDLR;

  volatile uint32_t TDHR;

} CAN_TxMailBox_TypeDef;











typedef struct

{

  volatile uint32_t RIR;

  volatile uint32_t RDTR;

  volatile uint32_t RDLR;

  volatile uint32_t RDHR;

} CAN_FIFOMailBox_TypeDef;











typedef struct

{

  volatile uint32_t FR1;

  volatile uint32_t FR2;

} CAN_FilterRegister_TypeDef;











typedef struct

{

  volatile uint32_t MCR;

  volatile uint32_t MSR;

  volatile uint32_t TSR;

  volatile uint32_t RF0R;

  volatile uint32_t RF1R;

  volatile uint32_t IER;

  volatile uint32_t ESR;

  volatile uint32_t BTR;

  uint32_t RESERVED0[88];

  CAN_TxMailBox_TypeDef sTxMailBox[3];

  CAN_FIFOMailBox_TypeDef sFIFOMailBox[2];

  uint32_t RESERVED1[12];

  volatile uint32_t FMR;

  volatile uint32_t FM1R;

  uint32_t RESERVED2;

  volatile uint32_t FS1R;

  uint32_t RESERVED3;

  volatile uint32_t FFA1R;

  uint32_t RESERVED4;

  volatile uint32_t FA1R;

  uint32_t RESERVED5[8];

  CAN_FilterRegister_TypeDef sFilterRegister[14];

} CAN_TypeDef;











typedef struct

{

  volatile uint32_t DR;

  volatile uint8_t IDR;

  uint8_t RESERVED0;

  uint16_t RESERVED1;

  volatile uint32_t CR;

} CRC_TypeDef;













typedef struct

{

  volatile uint32_t IDCODE;

  volatile uint32_t CR;

}DBGMCU_TypeDef;











typedef struct

{

  volatile uint32_t CCR;

  volatile uint32_t CNDTR;

  volatile uint32_t CPAR;

  volatile uint32_t CMAR;

} DMA_Channel_TypeDef;



typedef struct

{

  volatile uint32_t ISR;

  volatile uint32_t IFCR;

} DMA_TypeDef;















typedef struct

{

  volatile uint32_t IMR;

  volatile uint32_t EMR;

  volatile uint32_t RTSR;

  volatile uint32_t FTSR;

  volatile uint32_t SWIER;

  volatile uint32_t PR;

} EXTI_TypeDef;











typedef struct

{

  volatile uint32_t ACR;

  volatile uint32_t KEYR;

  volatile uint32_t OPTKEYR;

  volatile uint32_t SR;

  volatile uint32_t CR;

  volatile uint32_t AR;

  volatile uint32_t RESERVED;

  volatile uint32_t OBR;

  volatile uint32_t WRPR;

} FLASH_TypeDef;











typedef struct

{

  volatile uint16_t RDP;

  volatile uint16_t USER;

  volatile uint16_t Data0;

  volatile uint16_t Data1;

  volatile uint16_t WRP0;

  volatile uint16_t WRP1;

  volatile uint16_t WRP2;

  volatile uint16_t WRP3;

} OB_TypeDef;











typedef struct

{

  volatile uint32_t CRL;

  volatile uint32_t CRH;

  volatile uint32_t IDR;

  volatile uint32_t ODR;

  volatile uint32_t BSRR;

  volatile uint32_t BRR;

  volatile uint32_t LCKR;

} GPIO_TypeDef;











typedef struct

{

  volatile uint32_t EVCR;

  volatile uint32_t MAPR;

  volatile uint32_t EXTICR[4];

  uint32_t RESERVED0;

  volatile uint32_t MAPR2;

} AFIO_TypeDef;









typedef struct

{

  volatile uint32_t CR1;

  volatile uint32_t CR2;

  volatile uint32_t OAR1;

  volatile uint32_t OAR2;

  volatile uint32_t DR;

  volatile uint32_t SR1;

  volatile uint32_t SR2;

  volatile uint32_t CCR;

  volatile uint32_t TRISE;

} I2C_TypeDef;











typedef struct

{

  volatile uint32_t KR;

  volatile uint32_t PR;

  volatile uint32_t RLR;

  volatile uint32_t SR;

} IWDG_TypeDef;











typedef struct

{

  volatile uint32_t CR;

  volatile uint32_t CSR;

} PWR_TypeDef;











typedef struct

{

  volatile uint32_t CR;

  volatile uint32_t CFGR;

  volatile uint32_t CIR;

  volatile uint32_t APB2RSTR;

  volatile uint32_t APB1RSTR;

  volatile uint32_t AHBENR;

  volatile uint32_t APB2ENR;

  volatile uint32_t APB1ENR;

  volatile uint32_t BDCR;

  volatile uint32_t CSR;





} RCC_TypeDef;











typedef struct

{

  volatile uint32_t CRH;

  volatile uint32_t CRL;

  volatile uint32_t PRLH;

  volatile uint32_t PRLL;

  volatile uint32_t DIVH;

  volatile uint32_t DIVL;

  volatile uint32_t CNTH;

  volatile uint32_t CNTL;

  volatile uint32_t ALRH;

  volatile uint32_t ALRL;

} RTC_TypeDef;











typedef struct

{

  volatile uint32_t CR1;

  volatile uint32_t CR2;

  volatile uint32_t SR;

  volatile uint32_t DR;

  volatile uint32_t CRCPR;

  volatile uint32_t RXCRCR;

  volatile uint32_t TXCRCR;

  volatile uint32_t I2SCFGR;

} SPI_TypeDef;









typedef struct

{

  volatile uint32_t CR1;

  volatile uint32_t CR2;

  volatile uint32_t SMCR;

  volatile uint32_t DIER;

  volatile uint32_t SR;

  volatile uint32_t EGR;

  volatile uint32_t CCMR1;

  volatile uint32_t CCMR2;

  volatile uint32_t CCER;

  volatile uint32_t CNT;

  volatile uint32_t PSC;

  volatile uint32_t ARR;

  volatile uint32_t RCR;

  volatile uint32_t CCR1;

  volatile uint32_t CCR2;

  volatile uint32_t CCR3;

  volatile uint32_t CCR4;

  volatile uint32_t BDTR;

  volatile uint32_t DCR;

  volatile uint32_t DMAR;

  volatile uint32_t OR;

}TIM_TypeDef;













typedef struct

{

  volatile uint32_t SR;

  volatile uint32_t DR;

  volatile uint32_t BRR;

  volatile uint32_t CR1;

  volatile uint32_t CR2;

  volatile uint32_t CR3;

  volatile uint32_t GTPR;

} USART_TypeDef;











typedef struct

{

  volatile uint16_t EP0R;

  volatile uint16_t RESERVED0;

  volatile uint16_t EP1R;

  volatile uint16_t RESERVED1;

  volatile uint16_t EP2R;

  volatile uint16_t RESERVED2;

  volatile uint16_t EP3R;

  volatile uint16_t RESERVED3;

  volatile uint16_t EP4R;

  volatile uint16_t RESERVED4;

  volatile uint16_t EP5R;

  volatile uint16_t RESERVED5;

  volatile uint16_t EP6R;

  volatile uint16_t RESERVED6;

  volatile uint16_t EP7R;

  volatile uint16_t RESERVED7[17];

  volatile uint16_t CNTR;

  volatile uint16_t RESERVED8;

  volatile uint16_t ISTR;

  volatile uint16_t RESERVED9;

  volatile uint16_t FNR;

  volatile uint16_t RESERVEDA;

  volatile uint16_t DADDR;

  volatile uint16_t RESERVEDB;

  volatile uint16_t BTABLE;

  volatile uint16_t RESERVEDC;

} USB_TypeDef;













typedef struct

{

  volatile uint32_t CR;

  volatile uint32_t CFR;

  volatile uint32_t SR;

} WWDG_TypeDef;
