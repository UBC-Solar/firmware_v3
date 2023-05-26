/**
 * @file soc.c
 * @brief Functions for calculating the state of charge for UBC Solar's
 * V3 solar car, Brightside using interpolation and coloumb counting
 * 
 * 
 * @date 2021/11/09
 * @author Edward Ma, Forbes Choy 
 */

#include "soc.h"
#include <math.h>

#ifdef TEST
#include <stdio.h> // for debugging
#endif // TEST

/*============================================================================*/

// Testable private functions

#ifndef TEST
STATIC_TESTABLE int indexOfNearestCellVoltage(float cell_votlage);
STATIC_TESTABLE float calculateDeltaDOD(float present_current, float present_time, float past_current, float past_time);
#endif // TEST

/*============================================================================*/

//last current readings from CAN bus
float SOC_last_CAN_current_reading;

//last recorded elasped time since the start of the FSM (finite state machine)
int SOC_last_FSM_time;

//Lookup table (LUT) for state of charge initialization
//first row is cell voltage [V] and second row is the corresponding SOC [%]
//condition: 4A charge/discharge, 25^oC
//derive from the discharge curve in page 3 of this document: https://www.orbtronic.com/content/Datasheet-specs-Sanyo-Panasonic-NCR18650GA-3500mah.pdf

const float cell_voltage_SOC_table[2][41] =
{
  {4.50, 4.45, 4.40, 4.35, 4.30, 4.25, 4.20, 4.15, 4.10, 4.05, 4.00, 3.95,
  3.90, 3.85, 3.80, 3.75, 3.70, 3.65, 3.60, 3.55, 3.50, 3.45, 
  3.40, 3.35, 3.30, 3.25, 3.20, 3.15, 3.10, 3.05, 3.00, 2.95, 
  2.90, 2.85, 2.80, 2.75, 2.70, 2.65, 2.60, 2.55, 2.50},
  {100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0,
  97.971, 88.406, 81.884, 76.522, 72.464, 67.391, 62.319, 56.522, 52.464, 47.536,
  42.029, 35.942, 30.435, 24.928, 21.739, 18.261, 15.362, 13.043, 11.594, 10.145, 
  9.565, 8.406, 7.536, 6.957, 6.087, 5.652, 4.638, 4.348, 4.058}
};

/*============================================================================*/

/**
 * @brief initialize an SOC estimation of a module from its cell voltage (our module is 13 cells in parallel)
 *        calculation based on linear intepolation/extrapolation on the lookup table, cell_voltage_SOC_table 
 *
 * @param cell_voltage the cell voltage of a module [V]
 */

float SOC_moduleInit(float cell_voltage)
{
  float module_SOC = 0.0;

  //determine two cell voltage values from the cell_voltage_SOC_table, so that our cell voltage input is between them

  //the first cell voltage value from the lookup table is the one closest to our cell voltage input
  int first_index = indexOfNearestCellVoltage(cell_voltage);

  //first, check if our input cell voltage is equal to one of the cell voltages in our lookup table; if so we can just return our SOC estimation
  if (cell_voltage == cell_voltage_SOC_table[0][first_index])
  {
    module_SOC = cell_voltage_SOC_table[1][first_index];
    return module_SOC;
  }
  
  //determine the second index for intepolation/extrapolation
  int second_index = 0;
  
  if (cell_voltage > cell_voltage_SOC_table[0][first_index])
  {
    if (cell_voltage > cell_voltage_SOC_table[0][0]) //when our input cell voltage exceeds the maximum cell voltage of our lookup table
      second_index = first_index + 1;
    else
      second_index = first_index - 1;  
  }
  else
  {
    if (cell_voltage < cell_voltage_SOC_table[0][41-1]) //when our input cell voltage is less than the minimum cell voltage of our lookup table
      second_index = first_index - 1;
    else
      second_index = first_index + 1;
  }

  //we can begin our intepolation/extrapolation

  //determine the slope (dSOC/dcell_voltage) of the line formed by our cell voltages from the lookup table; m = (y2-y1)/(x2-x1);
  float slope = (cell_voltage_SOC_table[1][second_index] - cell_voltage_SOC_table[1][first_index]) / (cell_voltage_SOC_table[0][second_index] - cell_voltage_SOC_table[0][first_index]);

  //determine the corresponding SOC for our input cell voltage using our slope and lookup table values; y = m*(x-x1) + y1
  module_SOC = slope * (cell_voltage - cell_voltage_SOC_table[0][first_index]) + cell_voltage_SOC_table[1][first_index];

  return module_SOC;
}

/**
 * @brief initializes the SOC of all modules in our battery pack
 *
 * @param pack the battery pack data structure
 */
void SOC_allModulesInit(BTM_PackData_t * pack)
{
  //first set last CAN current reading and FSM time to 0
  SOC_last_CAN_current_reading = 0; 
  SOC_last_FSM_time = 0;

  //initialize SOC for all modules
  float cell_voltage_reading = 0.0;

  for(int stack_num = 0; stack_num < BTM_NUM_DEVICES; stack_num++) 
  {
    for(int module_num = 0; module_num < BTM_NUM_MODULES; module_num++) 
    {
      cell_voltage_reading = pack->stack[stack_num].module[module_num].voltage;
      pack->stack[stack_num].module[module_num].soc = SOC_moduleInit(cell_voltage_reading);
    }
  }
}

/**
 * @brief calculates/update an SOC estimation of a module
 *        the algorithm is based on the flowchart (figure 1, page 3) of this document: 
 *        https://www.analog.com/media/en/technical-documentation/technical-articles/a-closer-look-at-state-of-charge-and-state-health-estimation-techniques.pdf
 *
 * @param last_SOC last SOC estimation of the module [%]
 * @param cell_voltage_100uV the cell voltage of the module in 100uV [100uV]
 * @param current_reading current readings of our hall-effect sensors for the battery pack [A]; positive (+) current means the battery pack is charging and negative (-) current means the battery pack is discharging
 * @param total_time_elasped total time elapsed [ms]
 */

float SOC_moduleEst(float last_SOC, uint32_t cell_voltage_100uV, int32_t current_reading, uint32_t total_time_elasped)
{
  //initialize constants
  float cell_voltage = cell_voltage_100uV * 0.0001; //convert cell voltage readings into V
  float last_DOD = 100.0 - last_SOC; //instantaneous DOD of the module
  float delta_DOD = 0.0; //change in depth of discharge (DOD)

  /*------------------------------------*/

  //When Module is DISCHARGING --> Ib < 0

  if (current_reading < 0) //discharge
  {
    if(cell_voltage > SOC_CELL_MIN_VOLTAGE) //when module is not fully discharged yet --> Vb > Vmin
    {
      //first compute the change in DOD
      delta_DOD = calculateDeltaDOD(current_reading, total_time_elasped,
                  SOC_last_CAN_current_reading, SOC_last_FSM_time);

      last_DOD = last_DOD + SOC_CELL_DISCHARGE_EFFICIENCY * delta_DOD;
      
      //update last_SOC
      last_SOC = 100.0 - last_DOD;
    }
    else //when module is fully discharged --> Vb < Vmin
    {
      last_SOC = SOC_moduleInit(cell_voltage); //TODO: change the function name of SOC_moduleInit?
    }
  }
  /*------------------------------------*/

  //when Module is CHARGING --> Ib > 0
     
  else if(current_reading >= 0) //charge
  {
    if(cell_voltage < SOC_CELL_MAX_VOLTAGE) //when module is not fully charged yet
    {
      delta_DOD = calculateDeltaDOD(current_reading, total_time_elasped,
                  SOC_last_CAN_current_reading, SOC_last_FSM_time);
      last_DOD = last_DOD + SOC_CELL_CHARGE_EFFICIENCY * delta_DOD;
      last_SOC = 100.0 - last_DOD;
    }
    else //when module is fully charged
    {
      last_SOC = SOC_moduleInit(cell_voltage); //TODO: change the function name of SOC_moduleInit?
    }
  }

  return last_SOC;
}

/**
 * @brief calculates/update the SOC estimation of all modules of our battery pack
 *
 * @param pack the battery pack data structure
 * @param current_reading current readings of our hall-effect sensors for the battery pack [A]; positive (+) current means the battery pack is charging and negative (-) current means the battery pack is discharging
 * @param total_time_elasped total time elapsed [ms]
 */

void SOC_allModulesEst(BTM_PackData_t * pack, int32_t current_reading, uint32_t total_time_elasped)
{
  float cell_voltage_reading = 0.0;
  float last_module_SOC = 0.0;
  float min_module_SOC = 100.0;
  float module_wise_SOC = 0.0;

  for(int stack_num = 0; stack_num < BTM_NUM_DEVICES; stack_num++) 
  {
    for(int module_num = 0; module_num < BTM_NUM_MODULES; module_num++) 
    {
      cell_voltage_reading = pack->stack[stack_num].module[module_num].voltage;
      last_module_SOC = pack->stack[stack_num].module[module_num].soc;
      module_wise_SOC = SOC_moduleEst(last_module_SOC, cell_voltage_reading, current_reading, total_time_elasped);
      if(module_wise_SOC < min_module_SOC){
        min_module_SOC = module_wise_SOC; //find minimum SOC of all modules
      }
      
      pack->stack[stack_num].module[module_num].soc = module_wise_SOC;
    }
  }

  pack->PH_SOC_LOCATION = (uint8_t) min_module_SOC; //pack SOC is the lowest SOC of all modules
  //note casting

  SOC_last_CAN_current_reading = current_reading; //update current globally
  SOC_last_FSM_time = total_time_elasped; //update time globally
}

/*============================================================================*/

/**
 * @brief helper function to calculate the change in depth of discharge (DOD) of a module
 *
 * @param present_current instantaneous current reading [A]; positive (+) current means the battery pack is charging and negative (-) current means the battery pack is discharging
 * @param present_time current time [ms]
 * @param past_current last current readings [A]; positive (+) current means the battery pack is charging and negative (-) current means the battery pack is discharging
 * @param past_time last recorded time [ms]
 */

STATIC_TESTABLE float calculateDeltaDOD(float present_current, float present_time, float past_current, float past_time)
{
  float delta_DOD = 0.0; //signed value

  delta_DOD = - ( (present_current + past_current) / 2.0 * (present_time - past_time) / 1000.0 ) // divide time by 1, 000 to convert to s
              / SOC_MODULE_RATED_CAPACITY * 100.0; //equation 5 in Analog Devices' SoC estimation document

  return delta_DOD;
}

/**
 * @brief helper function to find the index of the closest cell voltage in the 
 *        cellVoltage_SOC_table lookup table from a given cell voltage input
 *
 * @param cell_voltage the cell voltage of a module [V]
 */

STATIC_TESTABLE int indexOfNearestCellVoltage(float cell_voltage)
{
  int result_index = 0;
  float cell_voltage_difference = 100.0; //first assign it to a large arbitrary value
  float cell_voltage_difference_tmp;

  //loop through the cellVoltage_SOC_table lookup table (LUT) to find the index (in the first row) closest to cell_voltage
  //the for loop implementation works because the cellVoltage_SOC_table LUT is sorted in decreasing order

  for(int j = 0; j < 41; j ++)
  {
    cell_voltage_difference_tmp = fabs(cell_voltage_SOC_table[0][j] - cell_voltage);
    
    if(cell_voltage_difference_tmp > cell_voltage_difference)
    {
      break;
    } 
    else 
    {
      cell_voltage_difference = cell_voltage_difference_tmp;
      result_index = j;
    }
  }
  
  return result_index;
}

/**
 * @brief get the DoD (depth of discharge) of the pack
 *
 * @param pack battery pack data structure
 */

uint8_t getDOD(BTM_PackData_t * pack){
  return 100 - (pack->PH_SOC_LOCATION);
}

/**
 * @brief helper function to get the Capacity (mAh) of pack
 *
 * @param pack battery pack data structure
 */

uint8_t getCapacity(BTM_PackData_t * pack){
  return SOC_MODULE_RATED_CAPACITY * (pack->PH_SOC_LOCATION / 100);
}
