#include <string.h>
#include <stdlib.h>
#include <setjmp.h>
#include "cmock.h"
#include "mock_stm32f1xx_hal_conf.h"


static struct mock_stm32f1xx_hal_confInstance
{
  unsigned char placeHolder;
} Mock;

extern jmp_buf AbortFrame;
extern int GlobalExpectCount;
extern int GlobalVerifyOrder;

void mock_stm32f1xx_hal_conf_Verify(void)
{
}

void mock_stm32f1xx_hal_conf_Init(void)
{
  mock_stm32f1xx_hal_conf_Destroy();
}

void mock_stm32f1xx_hal_conf_Destroy(void)
{
  CMock_Guts_MemFreeAll();
  memset(&Mock, 0, sizeof(Mock));
  GlobalExpectCount = 0;
  GlobalVerifyOrder = 0;
}

