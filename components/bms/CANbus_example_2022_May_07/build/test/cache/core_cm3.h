#include "Drivers/CMSIS/Include/cmsis_compiler.h"
#include "Drivers/CMSIS/Include/cmsis_version.h"
typedef union

{

  struct

  {

    uint32_t _reserved0:27;

    uint32_t Q:1;

    uint32_t V:1;

    uint32_t C:1;

    uint32_t Z:1;

    uint32_t N:1;

  } b;

  uint32_t w;

} APSR_Type;

typedef union

{

  struct

  {

    uint32_t ISR:9;

    uint32_t _reserved0:23;

  } b;

  uint32_t w;

} IPSR_Type;

typedef union

{

  struct

  {

    uint32_t ISR:9;

    uint32_t _reserved0:1;

    uint32_t ICI_IT_1:6;

    uint32_t _reserved1:8;

    uint32_t T:1;

    uint32_t ICI_IT_2:2;

    uint32_t Q:1;

    uint32_t V:1;

    uint32_t C:1;

    uint32_t Z:1;

    uint32_t N:1;

  } b;

  uint32_t w;

} xPSR_Type;

typedef union

{

  struct

  {

    uint32_t nPRIV:1;

    uint32_t SPSEL:1;

    uint32_t _reserved1:30;

  } b;

  uint32_t w;

} CONTROL_Type;

typedef struct

{

  volatile uint32_t ISER[8U];

        uint32_t RESERVED0[24U];

  volatile uint32_t ICER[8U];

        uint32_t RSERVED1[24U];

  volatile uint32_t ISPR[8U];

        uint32_t RESERVED2[24U];

  volatile uint32_t ICPR[8U];

        uint32_t RESERVED3[24U];

  volatile uint32_t IABR[8U];

        uint32_t RESERVED4[56U];

  volatile uint8_t IP[240U];

        uint32_t RESERVED5[644U];

  volatile uint32_t STIR;

} NVIC_Type;

typedef struct

{

  volatile const uint32_t CPUID;

  volatile uint32_t ICSR;

  volatile uint32_t VTOR;

  volatile uint32_t AIRCR;

  volatile uint32_t SCR;

  volatile uint32_t CCR;

  volatile uint8_t SHP[12U];

  volatile uint32_t SHCSR;

  volatile uint32_t CFSR;

  volatile uint32_t HFSR;

  volatile uint32_t DFSR;

  volatile uint32_t MMFAR;

  volatile uint32_t BFAR;

  volatile uint32_t AFSR;

  volatile const uint32_t PFR[2U];

  volatile const uint32_t DFR;

  volatile const uint32_t ADR;

  volatile const uint32_t MMFR[4U];

  volatile const uint32_t ISAR[5U];

        uint32_t RESERVED0[5U];

  volatile uint32_t CPACR;

} SCB_Type;

typedef struct

{

        uint32_t RESERVED0[1U];

  volatile const uint32_t ICTR;







        uint32_t RESERVED1[1U];



} SCnSCB_Type;

typedef struct

{

  volatile uint32_t CTRL;

  volatile uint32_t LOAD;

  volatile uint32_t VAL;

  volatile const uint32_t CALIB;

} SysTick_Type;

typedef struct

{

  volatile union

  {

    volatile uint8_t u8;

    volatile uint16_t u16;

    volatile uint32_t u32;

  } PORT [32U];

        uint32_t RESERVED0[864U];

  volatile uint32_t TER;

        uint32_t RESERVED1[15U];

  volatile uint32_t TPR;

        uint32_t RESERVED2[15U];

  volatile uint32_t TCR;

        uint32_t RESERVED3[29U];

  volatile uint32_t IWR;

  volatile const uint32_t IRR;

  volatile uint32_t IMCR;

        uint32_t RESERVED4[43U];

  volatile uint32_t LAR;

  volatile const uint32_t LSR;

        uint32_t RESERVED5[6U];

  volatile const uint32_t PID4;

  volatile const uint32_t PID5;

  volatile const uint32_t PID6;

  volatile const uint32_t PID7;

  volatile const uint32_t PID0;

  volatile const uint32_t PID1;

  volatile const uint32_t PID2;

  volatile const uint32_t PID3;

  volatile const uint32_t CID0;

  volatile const uint32_t CID1;

  volatile const uint32_t CID2;

  volatile const uint32_t CID3;

} ITM_Type;

typedef struct

{

  volatile uint32_t CTRL;

  volatile uint32_t CYCCNT;

  volatile uint32_t CPICNT;

  volatile uint32_t EXCCNT;

  volatile uint32_t SLEEPCNT;

  volatile uint32_t LSUCNT;

  volatile uint32_t FOLDCNT;

  volatile const uint32_t PCSR;

  volatile uint32_t COMP0;

  volatile uint32_t MASK0;

  volatile uint32_t FUNCTION0;

        uint32_t RESERVED0[1U];

  volatile uint32_t COMP1;

  volatile uint32_t MASK1;

  volatile uint32_t FUNCTION1;

        uint32_t RESERVED1[1U];

  volatile uint32_t COMP2;

  volatile uint32_t MASK2;

  volatile uint32_t FUNCTION2;

        uint32_t RESERVED2[1U];

  volatile uint32_t COMP3;

  volatile uint32_t MASK3;

  volatile uint32_t FUNCTION3;

} DWT_Type;

typedef struct

{

  volatile const uint32_t SSPSR;

  volatile uint32_t CSPSR;

        uint32_t RESERVED0[2U];

  volatile uint32_t ACPR;

        uint32_t RESERVED1[55U];

  volatile uint32_t SPPR;

        uint32_t RESERVED2[131U];

  volatile const uint32_t FFSR;

  volatile uint32_t FFCR;

  volatile const uint32_t FSCR;

        uint32_t RESERVED3[759U];

  volatile const uint32_t TRIGGER;

  volatile const uint32_t FIFO0;

  volatile const uint32_t ITATBCTR2;

        uint32_t RESERVED4[1U];

  volatile const uint32_t ITATBCTR0;

  volatile const uint32_t FIFO1;

  volatile uint32_t ITCTRL;

        uint32_t RESERVED5[39U];

  volatile uint32_t CLAIMSET;

  volatile uint32_t CLAIMCLR;

        uint32_t RESERVED7[8U];

  volatile const uint32_t DEVID;

  volatile const uint32_t DEVTYPE;

} TPI_Type;

typedef struct

{

  volatile uint32_t DHCSR;

  volatile uint32_t DCRSR;

  volatile uint32_t DCRDR;

  volatile uint32_t DEMCR;

} CoreDebug_Type;

static inline void __NVIC_SetPriorityGrouping(uint32_t PriorityGroup)

{

  uint32_t reg_value;

  uint32_t PriorityGroupTmp = (PriorityGroup & (uint32_t)0x07UL);



  reg_value = ((SCB_Type *) ((0xE000E000UL) + 0x0D00UL) )->AIRCR;

  reg_value &= ~((uint32_t)((0xFFFFUL << 16U) | (7UL << 8U)));

  reg_value = (reg_value |

                ((uint32_t)0x5FAUL << 16U) |

                (PriorityGroupTmp << 8U) );

  ((SCB_Type *) ((0xE000E000UL) + 0x0D00UL) )->AIRCR = reg_value;

}















static inline uint32_t __NVIC_GetPriorityGrouping(void)

{

  return ((uint32_t)((((SCB_Type *) ((0xE000E000UL) + 0x0D00UL) )->AIRCR & (7UL << 8U)) >> 8U));

}

static inline void __NVIC_EnableIRQ(IRQn_Type IRQn)

{

  if ((int32_t)(IRQn) >= 0)

  {

    ((NVIC_Type *) ((0xE000E000UL) + 0x0100UL) )->ISER[(((uint32_t)IRQn) >> 5UL)] = (uint32_t)(1UL << (((uint32_t)IRQn) & 0x1FUL));

  }

}

static inline uint32_t __NVIC_GetEnableIRQ(IRQn_Type IRQn)

{

  if ((int32_t)(IRQn) >= 0)

  {

    return((uint32_t)(((((NVIC_Type *) ((0xE000E000UL) + 0x0100UL) )->ISER[(((uint32_t)IRQn) >> 5UL)] & (1UL << (((uint32_t)IRQn) & 0x1FUL))) != 0UL) ? 1UL : 0UL));

  }

  else

  {

    return(0U);

  }

}

static inline void __NVIC_DisableIRQ(IRQn_Type IRQn)

{

  if ((int32_t)(IRQn) >= 0)

  {

    ((NVIC_Type *) ((0xE000E000UL) + 0x0100UL) )->ICER[(((uint32_t)IRQn) >> 5UL)] = (uint32_t)(1UL << (((uint32_t)IRQn) & 0x1FUL));

    __DSB();

    __ISB();

  }

}

static inline uint32_t __NVIC_GetPendingIRQ(IRQn_Type IRQn)

{

  if ((int32_t)(IRQn) >= 0)

  {

    return((uint32_t)(((((NVIC_Type *) ((0xE000E000UL) + 0x0100UL) )->ISPR[(((uint32_t)IRQn) >> 5UL)] & (1UL << (((uint32_t)IRQn) & 0x1FUL))) != 0UL) ? 1UL : 0UL));

  }

  else

  {

    return(0U);

  }

}

static inline void __NVIC_SetPendingIRQ(IRQn_Type IRQn)

{

  if ((int32_t)(IRQn) >= 0)

  {

    ((NVIC_Type *) ((0xE000E000UL) + 0x0100UL) )->ISPR[(((uint32_t)IRQn) >> 5UL)] = (uint32_t)(1UL << (((uint32_t)IRQn) & 0x1FUL));

  }

}

static inline void __NVIC_ClearPendingIRQ(IRQn_Type IRQn)

{

  if ((int32_t)(IRQn) >= 0)

  {

    ((NVIC_Type *) ((0xE000E000UL) + 0x0100UL) )->ICPR[(((uint32_t)IRQn) >> 5UL)] = (uint32_t)(1UL << (((uint32_t)IRQn) & 0x1FUL));

  }

}

static inline uint32_t __NVIC_GetActive(IRQn_Type IRQn)

{

  if ((int32_t)(IRQn) >= 0)

  {

    return((uint32_t)(((((NVIC_Type *) ((0xE000E000UL) + 0x0100UL) )->IABR[(((uint32_t)IRQn) >> 5UL)] & (1UL << (((uint32_t)IRQn) & 0x1FUL))) != 0UL) ? 1UL : 0UL));

  }

  else

  {

    return(0U);

  }

}

static inline void __NVIC_SetPriority(IRQn_Type IRQn, uint32_t priority)

{

  if ((int32_t)(IRQn) >= 0)

  {

    ((NVIC_Type *) ((0xE000E000UL) + 0x0100UL) )->IP[((uint32_t)IRQn)] = (uint8_t)((priority << (8U - __NVIC_PRIO_BITS)) & (uint32_t)0xFFUL);

  }

  else

  {

    ((SCB_Type *) ((0xE000E000UL) + 0x0D00UL) )->SHP[(((uint32_t)IRQn) & 0xFUL)-4UL] = (uint8_t)((priority << (8U - __NVIC_PRIO_BITS)) & (uint32_t)0xFFUL);

  }

}

static inline uint32_t __NVIC_GetPriority(IRQn_Type IRQn)

{



  if ((int32_t)(IRQn) >= 0)

  {

    return(((uint32_t)((NVIC_Type *) ((0xE000E000UL) + 0x0100UL) )->IP[((uint32_t)IRQn)] >> (8U - __NVIC_PRIO_BITS)));

  }

  else

  {

    return(((uint32_t)((SCB_Type *) ((0xE000E000UL) + 0x0D00UL) )->SHP[(((uint32_t)IRQn) & 0xFUL)-4UL] >> (8U - __NVIC_PRIO_BITS)));

  }

}

static inline uint32_t NVIC_EncodePriority (uint32_t PriorityGroup, uint32_t PreemptPriority, uint32_t SubPriority)

{

  uint32_t PriorityGroupTmp = (PriorityGroup & (uint32_t)0x07UL);

  uint32_t PreemptPriorityBits;

  uint32_t SubPriorityBits;



  PreemptPriorityBits = ((7UL - PriorityGroupTmp) > (uint32_t)(__NVIC_PRIO_BITS)) ? (uint32_t)(__NVIC_PRIO_BITS) : (uint32_t)(7UL - PriorityGroupTmp);

  SubPriorityBits = ((PriorityGroupTmp + (uint32_t)(__NVIC_PRIO_BITS)) < (uint32_t)7UL) ? (uint32_t)0UL : (uint32_t)((PriorityGroupTmp - 7UL) + (uint32_t)(__NVIC_PRIO_BITS));



  return (

           ((PreemptPriority & (uint32_t)((1UL << (PreemptPriorityBits)) - 1UL)) << SubPriorityBits) |

           ((SubPriority & (uint32_t)((1UL << (SubPriorityBits )) - 1UL)))

         );

}

static inline void NVIC_DecodePriority (uint32_t Priority, uint32_t PriorityGroup, uint32_t* const pPreemptPriority, uint32_t* const pSubPriority)

{

  uint32_t PriorityGroupTmp = (PriorityGroup & (uint32_t)0x07UL);

  uint32_t PreemptPriorityBits;

  uint32_t SubPriorityBits;



  PreemptPriorityBits = ((7UL - PriorityGroupTmp) > (uint32_t)(__NVIC_PRIO_BITS)) ? (uint32_t)(__NVIC_PRIO_BITS) : (uint32_t)(7UL - PriorityGroupTmp);

  SubPriorityBits = ((PriorityGroupTmp + (uint32_t)(__NVIC_PRIO_BITS)) < (uint32_t)7UL) ? (uint32_t)0UL : (uint32_t)((PriorityGroupTmp - 7UL) + (uint32_t)(__NVIC_PRIO_BITS));



  *pPreemptPriority = (Priority >> SubPriorityBits) & (uint32_t)((1UL << (PreemptPriorityBits)) - 1UL);

  *pSubPriority = (Priority ) & (uint32_t)((1UL << (SubPriorityBits )) - 1UL);

}

static inline void __NVIC_SetVector(IRQn_Type IRQn, uint32_t vector)

{

  uint32_t *vectors = (uint32_t *)((SCB_Type *) ((0xE000E000UL) + 0x0D00UL) )->VTOR;

  vectors[(int32_t)IRQn + 16] = vector;

}

static inline uint32_t __NVIC_GetVector(IRQn_Type IRQn)

{

  uint32_t *vectors = (uint32_t *)((SCB_Type *) ((0xE000E000UL) + 0x0D00UL) )->VTOR;

  return vectors[(int32_t)IRQn + 16];

}













__attribute__((__noreturn__)) static inline void __NVIC_SystemReset(void)

{

  __DSB();



  ((SCB_Type *) ((0xE000E000UL) + 0x0D00UL) )->AIRCR = (uint32_t)((0x5FAUL << 16U) |

                           (((SCB_Type *) ((0xE000E000UL) + 0x0D00UL) )->AIRCR & (7UL << 8U)) |

                            (1UL << 2U) );

  __DSB();



  for(;;)

  {

    __asm volatile ("nop");

  }

}

static inline uint32_t SCB_GetFPUType(void)

{

    return 0U;

}

extern volatile int32_t ITM_RxBuffer;

static inline uint32_t ITM_SendChar (uint32_t ch)

{

  if (((((ITM_Type *) (0xE0000000UL) )->TCR & (1UL )) != 0UL) &&

      ((((ITM_Type *) (0xE0000000UL) )->TER & 1UL ) != 0UL) )

  {

    while (((ITM_Type *) (0xE0000000UL) )->PORT[0U].u32 == 0UL)

    {

      __asm volatile ("nop");

    }

    ((ITM_Type *) (0xE0000000UL) )->PORT[0U].u8 = (uint8_t)ch;

  }

  return (ch);

}

static inline int32_t ITM_ReceiveChar (void)

{

  int32_t ch = -1;



  if (ITM_RxBuffer != ((int32_t)0x5AA55AA5U))

  {

    ch = ITM_RxBuffer;

    ITM_RxBuffer = ((int32_t)0x5AA55AA5U);

  }



  return (ch);

}

static inline int32_t ITM_CheckChar (void)

{



  if (ITM_RxBuffer == ((int32_t)0x5AA55AA5U))

  {

    return (0);

  }

  else

  {

    return (1);

  }

}
