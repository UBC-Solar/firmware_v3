#include "Drivers/CMSIS/Device/ST/STM32F1xx/Include/stm32f103xb.h"
typedef enum

{

  RESET = 0,

  SET = !RESET

} FlagStatus, ITStatus;



typedef enum

{

  DISABLE = 0,

  ENABLE = !DISABLE

} FunctionalState;





typedef enum

{

  SUCCESS = 0U,

  ERROR = !SUCCESS

} ErrorStatus;
