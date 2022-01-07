/**
 * @file soc.c
 * @brief This file contains functions for calculating the state of charge for UBC Solar's
 *        V3 solar car, Brightside using interpolation and coloumb counting
 * 
 * 
 * @date 2021/11/09
 * @author Edward Ma, Forbes Choy 
 */


#include "soc.h"
#include <math.h>

/*============================================================================*/

//last current readings from CAN bus
float SOC_last_CAN_currentReading; 

//last recorded elasped time since the start of the FSM (finite state machine)
int SOC_last_FSM_time;

//Lookup table (LUT) for state of charge initialization
//first row is cell voltage [V] and second row is the corresponding SOC [%]
//condition: 4A charge/discharge, 25^oC
//derive from the discharge curve in page 3 of this document: https://www.orbtronic.com/content/Datasheet-specs-Sanyo-Panasonic-NCR18650GA-3500mah.pdf
const float cellVoltage_SOC_table[2][21] =
{
  {4.50, 4.45, 4.40, 4.35, 4.30, 4.25, 4.20, 4.15, 4.10, 4.05, 4.00, 3.95,
  3.90, 3.85, 3.80, 3.75, 3.70, 3.65, 3.60, 3.55, 3.50},
  {100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0,
  97.971, 88.406, 81.884, 76.522, 72.464, 67.391, 62.319, 56.522, 52.464}
};

/*============================================================================*/

/**
 * @brief initialize an SOC estimation of a module based on its cell voltage (our module is 13 cells in parallel)
 *
 * @param cell_voltage the cell voltage of a module [V]
 */

float SOC_moduleInit(float cell_voltage)
{
  //find the cell voltage value closest to cell_voltage in the cellVoltage_SOC_table lookup table
  int LUT_index = LUT_indexOfNearestCellVoltage(cell_voltage);

  //return SOC from the cellVoltage_SOC_table lookup table, which are stored in the 2nd row
  return cellVoltage_SOC_table[2][LUT_index];
}

/**
 * @brief initializes the SOC of all modules in our battery pack
 *
 * @param pack the battery pack data structure
 */
void SOC_allModulesInit(BTM_PackData_t * pack)
{
  //first set last CAN current reading and FSM time to 0

  SOC_last_CAN_currentReading = 0; 
  SOC_last_FSM_time = 0;

  //initialize SOC for all modules
  
  float cellVoltage_reading = 0.0;

  for(int stack_num = 0; stack_num < BTM_NUM_DEVICES; stack_num++) 
  {
    for(int module_num = 0; module_num < BTM_NUM_MODULES; module_num++) 
    {
      cellVoltage_reading = pack->stack[stack_num].module[module_num].voltage;
      pack->stack[stack_num].module[module_num].soc = SOC_moduleInit(cellVoltage_reading);
    }
  }
}

/**
 * @brief calculates/update an SOC estimation of a module
 *        the algorithm is based on the flowchart (figure 1, page 3) of this document: 
 *        https://www.analog.com/media/en/technical-documentation/technical-articles/a-closer-look-at-state-of-charge-and-state-health-estimation-techniques.pdf
 *
 * @param last_SOC last SOC estimation of the module [%]
 * @param cell_voltage100uV the cell voltage of the module in 100uV [100uV]
 * @param CAN_currentReading current readings of our hall-effect sensors transmitted through CAN [A]
 * @param FSM_totalTimeElapsed total time elapsed since the start of our finite state machine (FSM) [ms]
 */

float SOC_moduleEst(float last_SOC, uint32_t cell_voltage100uV, int32_t CAN_currentReading, uint32_t FSM_totalTimeElapsed)
{
  //initialize constants
  float module_SOH = 100.0; //the battery SOH will be approximated as 100% until the module is fully charged or discharged
  float cell_voltage = cell_voltage100uV * 0.0001; //convert cell voltage readings into V
  float module_DOD = 100.0 - last_SOC; //current instantaneous DOD of a module
  float delta_DOD = 0.0; //change in depth of discharge (DOD)

  /*------------------------------------*/

  //When Module is DISCHARGING --> Ib < 0

  if (CAN_currentReading < 0) //discharge
  {
    if(cell_voltage > SOC_CELL_MIN_VOLTAGE) //when module is not fully discharged yet --> Vb > Vmin
    {
      //first compute the change in DOD
      delta_DOD = calculate_deltaDOD(CAN_currentReading, FSM_totalTimeElapsed,
                  SOC_last_CAN_currentReading, SOC_last_FSM_time);

      module_DOD = module_DOD + SOC_CELL_DISCHARGE_EFFICIENCY * delta_DOD;
      last_SOC = module_SOH - module_DOD;
    }
    else //when module is fully discharged --> Vb < Vmin
    {
      module_SOH = module_DOD;
      last_SOC = 100 - module_DOD;
    }
  }
  /*------------------------------------*/

  //when Module is CHARGING --> Ib > 0
     
  else if(CAN_currentReading >= 0) //charge
  {
    if(cell_voltage < SOC_CELL_MAX_VOLTAGE) //when module is not fully charged yet
    {
      delta_DOD = calculate_deltaDOD(CAN_currentReading, FSM_totalTimeElapsed,
                  SOC_last_CAN_currentReading, SOC_last_FSM_time);
      module_DOD = module_DOD + SOC_CELL_CHARGE_EFFICIENCY * delta_DOD;
      last_SOC = module_SOH - module_DOD;
    }
    else //when module is fully charged
    {
      module_SOH = module_DOD;
      last_SOC = 100 - module_DOD;
    }
  }

  return last_SOC;
}

/**
 * @brief calculates/update the SOC estimation of all modules of our battery pack
 *
 * @param pack the battery pack data structure
 * @param CAN_currentReading current readings of our hall-effect sensors transmitted through CAN [A]
 * @param FSM_totalTimeElapsed total time elapsed since the start of our finite state machine (FSM) [ms]
 */

void SOC_allModulesEst(BTM_PackData_t * pack, int32_t CAN_currentReading, uint32_t FSM_totalTimeElasped)
{
  float cellVoltage_reading = 0.0;
  float last_moduleSOC = 0.0;

  for(int stack_num = 0; stack_num < BTM_NUM_DEVICES; stack_num++) 
  {
    for(int module_num = 0; module_num < BTM_NUM_MODULES; module_num++) 
    {
      cellVoltage_reading = pack->stack[stack_num].module[module_num].voltage;
      last_moduleSOC = pack->stack[stack_num].module[module_num].soc;
      
      pack->stack[stack_num].module[module_num].soc = SOC_moduleEst(last_moduleSOC, cellVoltage_reading, CAN_currentReading, FSM_totalTimeElasped);
    }
  }

  SOC_last_CAN_currentReading = CAN_currentReading; //update current globally
  SOC_last_FSM_time = FSM_totalTimeElasped; //update time globally
}

/*============================================================================*/

/**
 * @brief helper function to calculate the change in depth of discharge (DOD) of a module
 *
 * @param presentCurrent current current reading [A]
 * @param presentTime current time [ms]
 * @param pastCurrent last current readings [A]
 * @param pastTime last recorded time [ms]
 */

float calculate_deltaDOD(float presentCurrent, float presentTime, float pastCurrent, float pastTime)
{
  float delta_DOD = 0.0; //signed value

  delta_DOD = - ( (presentCurrent + pastCurrent) / 2 * (presentTime - pastTime) / 1000 ) // divide time by 1, 000 to convert to s
              / SOC_MODULE_RATED_CAPACITY * 100; //equation 5 in Analog Devices' SoC estimation document

  return delta_DOD;
}

/**
 * @brief helper function to find the index of the closest cell voltage in the 
 *        cellVoltage_SOC_table lookup table from a given cell voltage input
 *
 * @param cell_voltage the cell voltage of a module [V]
 */

int LUT_indexOfNearestCellVoltage(float cell_voltage)
{
  int resultIndex = 0;
  float cellVoltageDifference = 100.0;
  float cellVoltageDifference_tmp;

  //loop through the cellVoltage_SOC_table lookup table (LUT) to find the index (in the first row) closest to cell_voltage
  //the for loop implementation works because the cellVoltage_SOC_table LUT is sorted in decreasing order
  
  for(int j = 0; j = 21; j ++)
  {
    cellVoltageDifference_tmp = fabs(cellVoltage_SOC_table[1][j] - cell_voltage);

    if(cellVoltageDifference_tmp > cellVoltageDifference)
    {
      break;
    } 
    else 
    {
      cellVoltageDifference = cellVoltageDifference_tmp;
      resultIndex = j;
    }
  }
  
  return resultIndex;
}
