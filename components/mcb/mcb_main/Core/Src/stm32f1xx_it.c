/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    stm32f1xx_it.c
 * @brief   Contains interrupt service routine definitions
 ******************************************************************************
 */
/* USER CODE END Header */

#include "main.h"
#include "stm32f1xx_it.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "cmsis_os.h"

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

extern osSemaphoreId_t eventFlagsSemaphoreHandle;
extern osSemaphoreId_t nextScreenSemaphoreHandle;

/* USER CODE END PV */

/* External variables --------------------------------------------------------*/
extern DMA_HandleTypeDef hdma_adc1;
extern TIM_HandleTypeDef htim8;

/* USER CODE BEGIN EV */

extern osThreadId_t updateEventFlagsTaskHandle;

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M3 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
 * @brief This function handles Non maskable interrupt.
 */
void NMI_Handler(void)
{
    /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

    /* USER CODE END NonMaskableInt_IRQn 0 */
    /* USER CODE BEGIN NonMaskableInt_IRQn 1 */

    /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
 * @brief This function handles Hard fault interrupt.
 */
void HardFault_Handler(void)
{
    /* USER CODE BEGIN HardFault_IRQn 0 */

    /* USER CODE END HardFault_IRQn 0 */
    while (1)
    {
        /* USER CODE BEGIN W1_HardFault_IRQn 0 */
        /* USER CODE END W1_HardFault_IRQn 0 */
    }
}

/**
 * @brief This function handles Memory management fault.
 */
void MemManage_Handler(void)
{
    /* USER CODE BEGIN MemoryManagement_IRQn 0 */

    /* USER CODE END MemoryManagement_IRQn 0 */
    while (1)
    {
        /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
        /* USER CODE END W1_MemoryManagement_IRQn 0 */
    }
}

/**
 * @brief This function handles Prefetch fault, memory access fault.
 */
void BusFault_Handler(void)
{
    /* USER CODE BEGIN BusFault_IRQn 0 */

    /* USER CODE END BusFault_IRQn 0 */
    while (1)
    {
        /* USER CODE BEGIN W1_BusFault_IRQn 0 */
        /* USER CODE END W1_BusFault_IRQn 0 */
    }
}

/**
 * @brief This function handles Undefined instruction or illegal state.
 */
void UsageFault_Handler(void)
{
    /* USER CODE BEGIN UsageFault_IRQn 0 */

    /* USER CODE END UsageFault_IRQn 0 */
    while (1)
    {
        /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
        /* USER CODE END W1_UsageFault_IRQn 0 */
    }
}

/**
 * @brief This function handles Debug monitor.
 */
void DebugMon_Handler(void)
{
    /* USER CODE BEGIN DebugMonitor_IRQn 0 */

    /* USER CODE END DebugMonitor_IRQn 0 */
    /* USER CODE BEGIN DebugMonitor_IRQn 1 */

    /* USER CODE END DebugMonitor_IRQn 1 */
}

/******************************************************************************/
/* STM32F1xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f1xx.s).                    */
/******************************************************************************/

/**
 * @brief This function handles the REGEN_EN interrupt.
 */
void EXTI1_IRQHandler(void)
{
    /* USER CODE BEGIN EXTI1_IRQn 0 */

    // EXTI1 corresponds to the REGEN_EN pin
    event_flags.regen_enable = HAL_GPIO_ReadPin(REGEN_EN_GPIO_Port, REGEN_EN_Pin);

    /* USER CODE END EXTI1_IRQn 0 */
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_1);
}


/**
 * @brief This function handles the BRK_IN interrupt.
 */
void EXTI2_IRQHandler(void)
{
    /* USER CODE BEGIN EXTI2_IRQn 0 */

    // EXTI2 corresponds to the BRK_IN pin

    event_flags.brake_in = HAL_GPIO_ReadPin(BRK_IN_GPIO_Port, BRK_IN_Pin);

    // when brake_in goes high, cruise control should be disengaged
    event_flags.cruise_status = DISABLE;

    /* USER CODE END EXTI2_IRQn 0 */

    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_2);
}

/**
 * @brief This function handles the RVRS_EN interrupt.
 */
void EXTI3_IRQHandler(void)
{
    /* USER CODE BEGIN EXTI3_IRQn 0 */

    // EXTI3 corresponds to the RVRS_EN pin

    event_flags.reverse_enable = HAL_GPIO_ReadPin(RVRS_EN_GPIO_Port, RVRS_EN_Pin);

    /* USER CODE END EXTI3_IRQn 0 */
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_3);
}

/**
 * @brief This function handles the NEXT_SCREEN interrupt.
 */
void EXTI4_IRQHandler(void)
{
    /* USER CODE BEGIN EXTI4_IRQn 0 */

    // EXTI4 corresponds to the NEXT_SCREEN pin

    osSemaphoreRelease(nextScreenSemaphoreHandle);

    /* USER CODE END EXTI4_IRQn 0 */
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_4);
}

/**
 * @brief This function handles DMA1 channel1 global interrupt.
 */
void DMA1_Channel1_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&hdma_adc1);
}

/**
 * @brief This function handles EXTI line[9:5] interrupts.
 */
void EXTI9_5_IRQHandler(void)
{
    /* USER CODE BEGIN EXTI9_5_IRQn 0 */

    // checks if the CRUISE_DIS external interrupt is triggered
    if (__HAL_GPIO_EXTI_GET_FLAG(CRUISE_DIS_Pin))
    {
        event_flags.cruise_status = DISABLE;
    }

    // checks if the CRUISE_UP external interrupt is triggered
    else if (__HAL_GPIO_EXTI_GET_FLAG(CRUISE_UP_Pin))
    {
        if ((cruise_value + CRUISE_INCREMENT_VALUE) > CRUISE_MAX)
        {
            cruise_value = CRUISE_MAX;
        }
        else
        {
            cruise_value += CRUISE_INCREMENT_VALUE;
        }
    }

    // checks if the CRUISE_DOWN external interrupt is triggered
    else if (__HAL_GPIO_EXTI_GET_FLAG(CRUISE_DOWN_Pin))
    {
        if ((cruise_value - CRUISE_INCREMENT_VALUE) < CRUISE_MIN)
        {
            cruise_value = CRUISE_MIN;
        }
        else
        {
            cruise_value -= CRUISE_INCREMENT_VALUE;
        }
    }

    /* USER CODE END EXTI9_5_IRQn 0 */
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_5);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_6);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_7);
}

/**
 * @brief This function handles TIM8 update interrupt.
 */
void TIM8_UP_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim8);
}

