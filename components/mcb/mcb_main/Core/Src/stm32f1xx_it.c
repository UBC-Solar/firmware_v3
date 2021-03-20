/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32f1xx_it.c
  * @brief   Interrupt Service Routines.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f1xx_it.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "cmsis_os.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define INC_VALUE 1.0

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

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
void NMI_Handler(void) {
    /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

    /* USER CODE END NonMaskableInt_IRQn 0 */
    /* USER CODE BEGIN NonMaskableInt_IRQn 1 */

    /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void) {
    /* USER CODE BEGIN HardFault_IRQn 0 */

    /* USER CODE END HardFault_IRQn 0 */
    while (1) {
        /* USER CODE BEGIN W1_HardFault_IRQn 0 */
        /* USER CODE END W1_HardFault_IRQn 0 */
    }
}

/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void) {
    /* USER CODE BEGIN MemoryManagement_IRQn 0 */

    /* USER CODE END MemoryManagement_IRQn 0 */
    while (1) {
        /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
        /* USER CODE END W1_MemoryManagement_IRQn 0 */
    }
}

/**
  * @brief This function handles Prefetch fault, memory access fault.
  */
void BusFault_Handler(void) {
    /* USER CODE BEGIN BusFault_IRQn 0 */

    /* USER CODE END BusFault_IRQn 0 */
    while (1) {
        /* USER CODE BEGIN W1_BusFault_IRQn 0 */
        /* USER CODE END W1_BusFault_IRQn 0 */
    }
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void) {
    /* USER CODE BEGIN UsageFault_IRQn 0 */

    /* USER CODE END UsageFault_IRQn 0 */
    while (1) {
        /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
        /* USER CODE END W1_UsageFault_IRQn 0 */
    }
}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void) {
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
  * @brief This function handles EXTI line1 interrupt.
  */
void EXTI1_IRQHandler(void) {
    /* USER CODE BEGIN EXTI1_IRQn 0 */

    // EXTI1 corresponds to the REGEN_EN value
    event_Mem->regen_enable = HAL_GPIO_ReadPin(REGEN_EN_GPIO_Port, REGEN_EN_Pin);

    /* USER CODE END EXTI1_IRQn 0 */
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_1);
    /* USER CODE BEGIN EXTI1_IRQn 1 */

    // flag unblocks the updateFlagsTask
    osSemaphoreRelease(eventFlagsSemaphoreHandle);

    /* USER CODE END EXTI1_IRQn 1 */
}

/**
  * @brief This function handles EXTI line2 interrupt.
  */
void EXTI2_IRQHandler(void) {
    /* USER CODE BEGIN EXTI2_IRQn 0 */

    // EXTI2 corresponds to the BRK_IN value

    event_Mem->brake_in = HAL_GPIO_ReadPin(BRK_IN_GPIO_Port, BRK_IN_Pin);

    /* USER CODE END EXTI2_IRQn 0 */
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_2);
    /* USER CODE BEGIN EXTI2_IRQn 1 */

    // flag unblocks the updateFlagsTask
    // uint32_t flags = osThreadFlagsSet(updateEventFlagsTaskHandle, 0x0001U);
    osSemaphoreRelease(eventFlagsSemaphoreHandle);

    /* USER CODE END EXTI2_IRQn 1 */
}

/**
  * @brief This function handles EXTI line3 interrupt.
  */
void EXTI3_IRQHandler(void) {
    /* USER CODE BEGIN EXTI3_IRQn 0 */

    // EXTI3 corresponds to the RVRS_EN value

    event_Mem->reverse_enable = HAL_GPIO_ReadPin(RVRS_EN_GPIO_Port, RVRS_EN_Pin);

    /* USER CODE END EXTI3_IRQn 0 */
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_3);
    /* USER CODE BEGIN EXTI3_IRQn 1 */

    osSemaphoreRelease(eventFlagsSemaphoreHandle);

    /* USER CODE END EXTI3_IRQn 1 */
}

/**
  * @brief This function handles EXTI line3 interrupt.
  */
void EXTI4_IRQHandler(void) {
    /* USER CODE BEGIN EXTI4_IRQn 0 */

    // EXTI3 corresponds to the CRUISE_EN value

    event_Mem->cruise_enable = HAL_GPIO_ReadPin(CRUISE_EN_GPIO_Port, CRUISE_EN_Pin);

    /* USER CODE END EXTI4_IRQn 0 */
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_4);
    /* USER CODE BEGIN EXTI4_IRQn 1 */

    // flag unblocks the updateFlagsTask
    uint32_t flags = osThreadFlagsSet(updateEventFlagsTaskHandle, 0x0001U);

    /* USER CODE END EXTI4_IRQn 1 */
}

/**
  * @brief This function handles EXTI line5 interrupt.
  */
void EXTI5_IRQHandler(void) {
    /* USER CODE BEGIN EXTI5_IRQn 0 */

    // EXTI3 corresponds to the CRUISE_EN value

    event_Mem->cruise_enable = !(HAL_GPIO_ReadPin(CRUISE_DIS_GPIO_Port, CRUISE_DIS_Pin));

    /* USER CODE END EXTI5_IRQn 0 */
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_5);
    /* USER CODE BEGIN EXTI5_IRQn 1 */

    // flag unblocks the updateFlagsTask
    // do we need to update flags here ??
    
    uint32_t flags = osThreadFlagsSet(updateEventFlagsTaskHandle, 0x0001U);

    /* USER CODE END EXTI5_IRQn 1 */
}

/**
  * @brief This function handles EXTI line6 interrupt.
  */
void EXTI6_IRQHandler(void) {
    /* USER CODE BEGIN EXTI6_IRQn 0 */

    // EXTI3 corresponds to the CRUISE_EN value

    if(HAL_GPIO_ReadPin(CRUISE_UP_GPIO_Port, CRUISE_UP_Pin)){
      event_Mem->cruise_value += INC_VALUE;
    }

    /* USER CODE END EXTI6_IRQn 0 */
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_6);
    /* USER CODE BEGIN EXTI6_IRQn 1 */

    // flag unblocks the updateFlagsTask
    // do we need to update flags here ??
    
    uint32_t flags = osThreadFlagsSet(updateEventFlagsTaskHandle, 0x0001U);

    /* USER CODE END EXTI6_IRQn 1 */
}


/**
  * @brief This function handles EXTI line7 interrupt.
  */
void EXTI7_IRQHandler(void) {
    /* USER CODE BEGIN EXTI7_IRQn 0 */

    // EXTI3 corresponds to the CRUISE_EN value

    if(HAL_GPIO_ReadPin(CRUISE_DOWN_GPIO_Port, CRUISE_DOWN_Pin)){
      event_Mem->cruise_value -= INC_VALUE;
    }

    /* USER CODE END EXTI7_IRQn 0 */
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_7);
    /* USER CODE BEGIN EXTI7_IRQn 1 */

    // flag unblocks the updateFlagsTask
    // do we need to update flags here ??
    
    uint32_t flags = osThreadFlagsSet(updateEventFlagsTaskHandle, 0x0001U);

    /* USER CODE END EXTI7_IRQn 1 */
}


/**
  * @brief This function handles DMA1 channel1 global interrupt.
  */
void DMA1_Channel1_IRQHandler(void) {
    /* USER CODE BEGIN DMA1_Channel1_IRQn 0 */

    /* USER CODE END DMA1_Channel1_IRQn 0 */
    HAL_DMA_IRQHandler(&hdma_adc1);
    /* USER CODE BEGIN DMA1_Channel1_IRQn 1 */

    /* USER CODE END DMA1_Channel1_IRQn 1 */
}

/**
  * @brief This function handles TIM8 update interrupt.
  */
void TIM8_UP_IRQHandler(void) {
    /* USER CODE BEGIN TIM8_UP_IRQn 0 */

    /* USER CODE END TIM8_UP_IRQn 0 */
    HAL_TIM_IRQHandler(&htim8);
    /* USER CODE BEGIN TIM8_UP_IRQn 1 */

    /* USER CODE END TIM8_UP_IRQn 1 */
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
