#include "debug_io.h"
#include <stdio.h>

// =============================================================================
// Configuration Parameters
// =============================================================================
//#define USE_UART
#define USE_SWO
#define NO_SYSCALL

static UART_HandleTypeDef *io_huart;

/**
 * Configure a UART peripheral assignment for printf debugging
 *
 * @param huart HAL handle for the UART to use for printf debugging
*/
void DebugIO_Init(UART_HandleTypeDef *huart)
{
    io_huart = huart;
}

// Configure where printf() and putchar() output goes
int __io_putchar(int ch)
{
#ifdef USE_UART
    // Output on UART
    HAL_UART_Transmit(io_huart, (uint8_t *)&ch, 1, 0xFFFF);
#endif // USE_UART

#ifdef USE_SWO
    // Output on Serial Wire Output (SWO)
    ITM_SendChar(ch);
#endif // USE_SWO

    return ch;
}

int __io_getchar()
{
    // Implement if needed
    return 0;
}

#ifdef NO_SYSCALL
int _read(int file, char *ptr, int len)
{
    int DataIdx;

    for (DataIdx = 0; DataIdx < len; DataIdx++)
    {
        *ptr++ = __io_getchar();
    }

    return len;
}

int _write(int file, char *ptr, int len)
{
    int DataIdx;

    for (DataIdx = 0; DataIdx < len; DataIdx++)
    {
        __io_putchar(*ptr++);
    }
    return len;
}
#endif // NO_SYSCALL
