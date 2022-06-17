#include <string.h>
#include <stdlib.h>
#include <setjmp.h>
#include "cmock.h"
#include "mock_stm32f1xx.h"


static struct mock_stm32f1xxInstance
{
  unsigned char placeHolder;
} Mock;

extern jmp_buf AbortFrame;
extern int GlobalExpectCount;
extern int GlobalVerifyOrder;

void mock_stm32f1xx_Verify(void)
{
}

void mock_stm32f1xx_Init(void)
{
  mock_stm32f1xx_Destroy();
}

void mock_stm32f1xx_Destroy(void)
{
  CMock_Guts_MemFreeAll();
  memset(&Mock, 0, sizeof(Mock));
  GlobalExpectCount = 0;
  GlobalVerifyOrder = 0;
}

