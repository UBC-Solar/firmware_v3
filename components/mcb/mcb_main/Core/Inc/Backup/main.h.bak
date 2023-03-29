/* USER CODE BEGIN Header */

/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  */

/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "cmsis_os.h"

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

extern TIM_HandleTypeDef htim3;

/** Helps write float values into the CAN message data array.
 * 	Instead of converting 32-bit floating point values into 4 byte arrays manually,
 * 	the union automatically takes care of that conversion. This conversion is necessary
 * 	since you cannot write float values directly into the CAN message data field. The float
 * 	value must first be converted to an array of 4 bytes (uint8_t).
 *
 */
typedef union FloatBytes {
	float float_value;			/**< Float value member of the union. */
	uint8_t bytes[4];			/**< Array of 4 bytes member of union. */
} FloatBytes;

/** Collection of boolean flags that are set/reset by driver inputs on the steering wheel
 * 	and/or dashboard. The main control board uses the values inside this struct along with
 * 	other variables to decide what drive state it should be in. The struct contains strictly
 * 	boolean values and is (for the most part) written to by interrupt service routines
 * 	and read from by FreeRTOS tasks. Please note that writing to this struct is **not** protected
 * 	by any locking mechanism such as a semaphore or mutex.
 */
typedef struct InputFlags {
  volatile uint8_t regen_enable;			/**< Records the position of the regen switch. A value of 0x01 indicates regenerative braking is
  	  	  	  	  	  	  	  	  	  	  	 	 enabled and a value of 0x00 indicates that regen is disabled. */

  volatile uint8_t reverse_enable;			/**< Records the position of the reverse switch. A value of 0x01 indicates "reverse" is enabled
   	   	   	   	   	   	   	   	   	   	   	 	 and a value of 0x00 indicates that "reverse" is disabled. */

  volatile uint8_t cruise_status;			/**< Records the cruise control status. A value of 0x01 indicates that the CRUISE_EN button has
   	   	   	   	   	   	   	   	   	   	   	 	 pressed and a value of 0x00 means the CRUISE_DIS button has been pressed. */

  volatile uint8_t brake_in;				/**< Records if the brake has been pressed. A value of 0x01 indicates the brake pedal has been pressed
  	  	  	  	  	  	  	  	  	  	  	  	 and a value of 0x00 means the brake pedal has not been pressed. This value is used
  	  	  	  	  	  	  	  	  	  	  	  	 to ensure cruise control mode is exited when the brake pedal is pressed. */

  volatile uint8_t regen_value_is_zero;		/**< Flag that indicates if the regen value read from the ADC is zero or not. A value
  	  	  	  	  	  	  	  	  	  	  	  	 of 0x01 means the regen value is zero while a value of 0x00 means the regen value
  	  	  	  	  	  	  	  	  	  	  	  	 is not zero. */

  volatile uint8_t encoder_value_is_zero;	/**< Flag that indicates if the encoder value read from the hardware timer is zero
  	  	  	  	  	  	  	  	  	  	  	  	 or not. A value of 0x01 means the regen value is zero while a value of 0x00
  	  	  	  	  	  	  	  	  	  	  	  	 means the regen value is not zero. */

  volatile uint8_t motor_overheat;			/**< Flag that indicates if the motor is above its maximum temperature. A value of 0x01 means that
   	   	   	   	   	   	   	   	   	   	   	   	 the motor is over heating while 0x00 means that the motor condition is acceptable. */

} InputFlags;

extern union FloatBytes current;			/**< Stores the current value to send to motor controller over CAN */
extern union FloatBytes velocity;			/**< Stores the velocity value to send to motor controller over CAN */
extern union FloatBytes motor_temperature;	/**< Stores the motor temperature read from the CAN bus */

extern struct InputFlags event_flags;

extern uint32_t regen_value;
extern uint8_t cruise_value;
extern uint8_t battery_soc;

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define REGEN_VAL_Pin GPIO_PIN_0
#define REGEN_VAL_GPIO_Port GPIOA
#define REGEN_EN_Pin GPIO_PIN_1
#define REGEN_EN_GPIO_Port GPIOA
#define REGEN_EN_EXTI_IRQn EXTI1_IRQn
#define BRK_IN_Pin GPIO_PIN_2
#define BRK_IN_GPIO_Port GPIOA
#define BRK_IN_EXTI_IRQn EXTI2_IRQn
#define RVRS_EN_Pin GPIO_PIN_3
#define RVRS_EN_GPIO_Port GPIOA
#define RVRS_EN_EXTI_IRQn EXTI3_IRQn
#define NEXT_SCREEN_Pin GPIO_PIN_4
#define NEXT_SCREEN_GPIO_Port GPIOA
#define NEXT_SCREEN_EXTI_IRQn EXTI4_IRQn
#define CRUISE_DIS_Pin GPIO_PIN_5
#define CRUISE_DIS_GPIO_Port GPIOA
#define CRUISE_DIS_EXTI_IRQn EXTI9_5_IRQn
#define CRUISE_UP_Pin GPIO_PIN_6
#define CRUISE_UP_GPIO_Port GPIOA
#define CRUISE_UP_EXTI_IRQn EXTI9_5_IRQn
#define CRUISE_DOWN_Pin GPIO_PIN_7
#define CRUISE_DOWN_GPIO_Port GPIOA
#define CRUISE_DOWN_EXTI_IRQn EXTI9_5_IRQn
#define ENC_AS_Pin GPIO_PIN_8
#define ENC_AS_GPIO_Port GPIOA
#define ENC_BS_Pin GPIO_PIN_9
#define ENC_BS_GPIO_Port GPIOA
#define CRUISE_STAT_Pin GPIO_PIN_10
#define CRUISE_STAT_GPIO_Port GPIOA
/* USER CODE BEGIN Private defines */

#define ADC_MAX 0xFFF
#define ADC_MIN 0

#define ENCODER_TIMER_TICKS (uint32_t) 1

#define CRUISE_INCREMENT_VALUE 	1
#define CRUISE_MAX              100         /* value is in km/h */
#define CRUISE_MIN              0           /* value is in km/h */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
