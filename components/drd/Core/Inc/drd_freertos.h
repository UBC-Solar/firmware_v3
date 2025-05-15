#ifndef __DRD_FREERTOS___
#define __DRD_FREERTOS___

#define SOC_CALCULATE_ON        (0xFF)
#define SOC_CALCULATE_OFF       (0x00)
#define CALCULATE_SOC_DELAY     (50)

extern osEventFlagsId_t calculate_soc_flagHandle;

#endif /* __DRD_FREERTOS__ */