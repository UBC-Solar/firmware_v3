#include <string.h>
#include <stdlib.h>
#include <setjmp.h>
#include "cmock.h"
#include "mock_stm32f103xb.h"


static struct mock_stm32f103xbInstance
{
  unsigned char placeHolder;
} Mock;

extern jmp_buf AbortFrame;
extern int GlobalExpectCount;
extern int GlobalVerifyOrder;

void mock_stm32f103xb_Verify(void)
{
}

void mock_stm32f103xb_Init(void)
{
  mock_stm32f103xb_Destroy();
}

void mock_stm32f103xb_Destroy(void)
{
  CMock_Guts_MemFreeAll();
  memset(&Mock, 0, sizeof(Mock));
  GlobalExpectCount = 0;
  GlobalVerifyOrder = 0;
}

