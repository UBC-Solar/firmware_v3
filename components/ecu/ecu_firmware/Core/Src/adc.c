/**
 * @file adc.c
 * @brief ADC interface for supplemental battery and current sensor read on ECU
 * 
 * These functions need to average out the ADC readings for accuracy.
 * 
 * @date 2021/10/30
 * @author Forbes Choy 
 */

#include "adc.h"

/*============================================================================*/
/* PRIVATE DEFINITIONS */

#define ADC3_BUF_LENGTH (ADC3_BUF_LENGTH_PER_CHANNEL * ADC3_NUM_ANALOG_CHANNELS)

#define ADC_MAX_VOLT_READING 3.3
#define ADC_RESOLUTION 4095

/*============================================================================*/
/* GLOBAL VARIABLES */

// Peripheral handles
static ADC_HandleTypeDef *ADC3_hadc;
static TIM_HandleTypeDef *ADC3_htim;

//global variables to store ADC readings
static int ADC3_supp_batt_volt;               //stores supplemental battery voltage readings in ADC3
static int ADC3_motor_current;                //stores current readings, flowing form the motor to the battery, in ADC3
static int ADC3_array_current;                //stores current readings, flowing from solar arrays to the battery, in ADC3
static volatile int ADC3_DMA_in_process_flag; //flag that indicates the DMA interrupt if ADC3 has been called and is in process
static volatile int ADC3_DMA_fault_flag;      //flag that indicates the DMA interrupt if ADC3 has been called and is at fault

static volatile uint16_t adc3_buf[ADC3_BUF_LENGTH] = {0};


/*============================================================================*/
/* PRIVATE FUNCTIONS */

/**
 * @brief Provides the voltage of the supplemental battery (mV)
 * 
 * The supplemental battery voltage input is provided as an float, which is mainly processed and averaged
 * by the ADC3_processRawReadings function (see below), with units of volts (V).
 * Function then uses the input (note that it comes from a voltage divider) to output the voltage of the supplemental battery.
 * From the ADC3_processRawReadings function (see below), the voltage is averaged over 1 second in 100 readings (100Hz).
 * Supp battery only for startup not really in driving
 * 
 * @param supp_voltage float variable
 * @retval sets the global variable ADC3_supp_batt_volt (int datatype) to its input (supp_voltage)
 */
static void ADC3_setSuppBattVoltage(float supp_voltage)
{
    //assign the average ADC voltage to supp_voltage
    //adc voltage comes from a voltage divider made of R1=10k and R2=1k (v_adc = 1/11 * v_supp)
    ADC3_supp_batt_volt = (supp_voltage * ADC_MAX_VOLT_READING / ADC_RESOLUTION) * 11 * 1000; 
}

/**
 * @brief Provides the current flowing between the motor controller and battery pack (cA)
 * 
 * The motor current input is provided as an float, which is mainly processed and averaged
 * by the ADC3_processRawReadings function (see below), with units of volts (V).
 * Function then uses the input  to output the current according to the datasheet of our
 * hall effect sensor (HASS 50 600-S).
 * From the ADC3_processRawReadings function (see below), the current is averaged over 1 second in 100 readings (100Hz).
 * 
 * Current flowing **OUT** of the pack is positive.
 * Hardware exists to detect overcurrent
 * Output pins are wired to comparators to a set voltage
 * 
 * @param motor_current float variable
 * @retval sets the global variable ADC3_motor_current (int datatype) to its input (motor_current)
 */
static void ADC3_setMotorCurrent(float motor_current)
{
    //convert volt readings into current
    //good references: https://www.lem.com/sites/default/files/products_datasheets/hass_50_600-s.pdf 

    motor_current = motor_current * ADC_MAX_VOLT_READING / ADC_RESOLUTION;

    ADC3_motor_current = (motor_current - 2.5) * 80 * 100; //output in cA
}

/**
 * @brief Provides current flowing between the solar array and battery pack (cA)
 * 
 * The array current input is provided as an float, which is mainly processed and averaged
 * by the ADC3_processRawReadings function (see below), with units of volts (V).
 * Function then uses the input  to output the current according to the datasheet of our
 * hall effect sensor (HASS 50 600-S).
 * From the ADC3_processRawReadings function (see below), the current is averaged over 1 second in 100 readings (100Hz).
 * 
 * Current is averaged over 1 second. The result is provided as a
 * signed integer with units of 100ths of an amp (cA).
 * Current flowing **INTO** the pack is positive.
 * 
 * @param array_current float variable
 * @retval sets the global variable ADC3_array_current (int datatype) to its input (array_current)
 */
void ADC3_setArrayCurrent(float array_current)
{
    //convert raw readings to voltage readings
    array_current = array_current * ADC_MAX_VOLT_READING / ADC_RESOLUTION;
    
    //convert volt readings into current
    //good references: https://www.lem.com/sites/default/files/products_datasheets/hass_50_600-s.pdf 

    ADC3_array_current = (array_current - 2.5) * 80 * 100; //output in cA
}

/**
 * @brief sets the busy flag in the global variable ADC3_DMA_in_process_flag
 * depending if ADC3 attempts to read values in the middle of a DMA interrupt callback process
 * 
 * @param flag_value integer value: 1 is busy, 0 is not at busy
 * @retval sets ADC3_DMA_in_process_flag with flag_value
 */
void ADC3_setBusyStatus(int flag_value)
{
  ADC3_DMA_in_process_flag = flag_value;
}

/**
 * @brief sets the Fault flag in the global variable ADC3_DMA_fault_flag
 * depending if ADC3 attempts to read values in the middle of a DMA interrupt callback process
 * 
 * @param flag_value integer value: 1 is at fault, 0 is not at fault
 * @retval sets ADC3_DMA_fault_flag with flag_value
 */
void ADC3_setFaultStatus(int flag_value)
{
  ADC3_DMA_fault_flag = flag_value;
}

/**
 * @brief converts the raw readings of ADC3 into ADC voltage readings and
 * averages these values at half of its buffer size per channel, 
 * when the DMA interrupt for ADC3 is called when half of its scan 
 * conversion sequence is completed. These output values will then be converted
 * into supplemental battery voltage, current flowing between motor controller
 * and battery pack, and the current flowing between solar arrays and battery pack 
 * 
 * @param half integer (1 or 0) indicating if the averaging is from the first half (0)
 *             of its circular buffer or its second half (1)
 * @param adc3_buf volatile int array that represents ADC3's DMA circular buffer 
 *                 (200 elements per channel/measurement values; 600 in total where the
 *                 first 200 elments stores supplemental battery voltage, then the next
 *                 200 elements stores the motor current, and finally the last 200 for array current)
 * @param result float array of size 3, storing the averaged ADC voltages (float) for
 *               supplemental battery voltage, motor current, array current
 * @retval stores the averaged readings of ADC3 to result[]
 */
void ADC3_processRawReadings(int half, volatile uint16_t adc3_buf[], float result[])
{
  uint32_t sum[ADC3_NUM_ANALOG_CHANNELS] = {0}; //used for summing in the averaging process
  
  int sample_num = 0; //start index for averaging the ADC readings
  int limit = ADC3_BUF_LENGTH_PER_CHANNEL >> 1; // divide by 2; the end index of the averaging process

  if(half == 1) //start and end indices if averaging the second half of the ADC3's DMA circular buffer
  {
    sample_num = ADC3_BUF_LENGTH_PER_CHANNEL >> 1; 
    limit = ADC3_BUF_LENGTH_PER_CHANNEL;
  }

  // Average the samples
  for(; sample_num < limit; sample_num++) //summing the readings in the averaging process
  {
    for(int channel = 0; channel < ADC3_NUM_ANALOG_CHANNELS; channel++)
    {
      //adc3_buf is organized as [supp batt x 200 ... motor curr x 200 ... array curr x 200]
      //when sampling at 100Hz
      sum[channel] += adc3_buf[ADC3_NUM_ANALOG_CHANNELS * sample_num + channel];
    }
  }
  
  //the division process of the averaging
  for(int channel = 0; channel < ADC3_NUM_ANALOG_CHANNELS; channel++)
  {
    //averages for supplemental batery voltage, motor current, and array current
    result[channel] = ((float) sum[channel]) / (ADC3_BUF_LENGTH_PER_CHANNEL >> 1);
  }
}


/*============================================================================*/
/* PUBLIC FUNCTIONS */

void ADC3_Init(ADC_HandleTypeDef* adc_handle, TIM_HandleTypeDef *trigger_timer_handle)
{
  ADC3_hadc = adc_handle;
  ADC3_htim = trigger_timer_handle;

  ADC3_setFaultStatus(0); //ADC3 not in process yet
  ADC3_setBusyStatus(0);

  ADC3_setSuppBattVoltage(0);
  ADC3_setMotorCurrent(0);
  ADC3_setArrayCurrent(0);

  HAL_ADC_Start_DMA(ADC3_hadc, (uint32_t *) adc3_buf, ADC3_BUF_LENGTH);
  HAL_TIM_Base_Start(ADC3_htim);
}

/**
 * @brief Retrieves the voltage of the supplemental battery (mV)
 * 
 * The supplement battery voltage is stored in the global varialbe ADC3_supp_batt_volt (int datatype),
 * and the function returns that global variable
 * Supp battery only for startup not really in driving
 * 
 * @param -
 * @retval returns the global variable ADC3_supp_batt_volt (int datatype)
 */
int ADC3_getSuppBattVoltage()
{
    return ADC3_supp_batt_volt;
}

/**
 * @brief Retrieves the current flowing between the motor controller and battery pack (cA)
 * 
 * The motor current is stored in the global varialbe ADC3_motor_current (int datatype),
 * and the function returns that global variable
 * Current flowing **OUT** of the pack is positive.
 * 
 * @param -
 * @retval returns the global variable ADC3_motor_current (int datatype)
 */
int ADC3_getMotorCurrent()
{
    return ADC3_motor_current;
}

/**
 * @brief Retrieves the current flowing between the solar array and battery pack (cA)
 * 
 * The motor current is stored in the global varialbe ADC3_array_current (int datatype),
 * and the function returns that global variable
 * Current flowing **INTO** of the pack is positive.
 * 
 * @param -
 * @retval returns the global variable ADC3_array_current (int datatype)
 */
int ADC3_getArrayCurrent()
{
    return ADC3_array_current;
}

/**
 * @brief Calculates the net current flowing out of the battery
 * 
 * If the return value is positive, net current is flowing out of the
 * battery and it is discharging.
 * If the return value is negative, net current is flowing into the
 * battery and it is charging.
 * 
 * @param motor_current Current to motor (positive out of pack)
 * @param array_current Current from array (positive into pack)
 * @retval net current out of battery, in the same format as the inputs
 */
int ADC3_netCurrentOut(int motor_current, int array_current)
{
  return ADC3_motor_current - ADC3_array_current;
}

/**
 * @brief Retrieves the busy status of ADC3, stored in global variable
 * ADC3_DMA_in_process_flag
 * 
 * @param -
 * @retval returns the global variable ADC3_DMA_in_process_flag (int datatype).
 *         1 means at busy; 0 means not at busy
 */
int ADC3_getBusyStatus() 
{
  return ADC3_DMA_in_process_flag;
}

/**
 * @brief Retrieves the fault status of ADC3, stored in global variable
 * ADC3_DMA_fault_flag
 * 
 * @param -
 * @retval returns the global variable ADC3_DMA_fault_flag (int datatype).
 *         1 means at fault; 0 means not at fault
 */
int ADC3_getFaultStatus() 
{
  return ADC3_DMA_fault_flag;
}


/*============================================================================*/
/* PUBLIC CALLBACK FUNCTIONS */

void ADC3_ConversionCompleteCallback(ADC_BufferHalf_t bufferHalf)
{
  if (!ADC3_getBusyStatus()) // make sure DMA processing stops when processing ADC3 readings
  {
    ADC3_setBusyStatus(1); // indicates DMA right now is in process
    static float result[ADC3_NUM_ANALOG_CHANNELS] = {0.0}; //stores supplemental battery voltage, motor and array currents

    // Average 1st half of the buffer
    ADC3_processRawReadings(bufferHalf, adc3_buf, result);

    // convert averaged raw readings into corresponding voltage and current values
    ADC3_setSuppBattVoltage(result[0]);
    ADC3_setMotorCurrent(result[1]);
    ADC3_setArrayCurrent(result[2]);

    ADC3_setBusyStatus(0); //indicates now the DMA is not in process
  }
  else // fault status when previous DMA processing is not finished beforehand
  {
    ADC3_setFaultStatus(1); 
    HAL_TIM_Base_Stop(ADC3_htim); //stop TIM8: the trigger timer for ADC3
  }
}

void ADC3_ErrorCallback(void)
{
  //stop and reset DMA when there is an error
  //reference: http://www.disca.upv.es/aperles/arm_cortex_m3/llibre/st/STM32F439xx_User_Manual/stm32f4xx__hal__adc_8c_source.html#l01675
  if (ADC3_hadc->State == HAL_ADC_STATE_ERROR_DMA)
  {
    //stop and reset DMA
    //reference: http://www.disca.upv.es/aperles/arm_cortex_m3/llibre/st/STM32F439xx_User_Manual/group__adc__exported__functions__group2.html#gadea1a55c5199d5cb4cfc1fdcd32be1b2
    HAL_TIM_Base_Stop(ADC3_htim); //stop TIM8: the trigger timer for ADC3   
    HAL_ADC_Stop_DMA(ADC3_hadc);
    HAL_ADC_Start_DMA(ADC3_hadc, (uint32_t *) adc3_buf, ADC3_BUF_LENGTH);
    HAL_TIM_Base_Start(ADC3_htim);
  }
  else
  {
    /* In case of ADC error, call main error handler */
    Error_Handler();
  }
}
