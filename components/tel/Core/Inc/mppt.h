#ifndef _INC_MPPT_H_
#define _INC_MPPT_H_

/* DEFINES */
#define MPPT_INPUT_CURRENT_MAX     (7.0f)     
#define MPPT_OUTPUT_VOLTAGE_MAX     (165.0f)     
#define MPPT_COMMAND_DELAY          (10 * 1000)     // 10 seconds       
#define MPPT_COMMAND_TASK_OFFSET    (200)  

/* FUNCTION DECLARATIONS */
void MPPT_output_voltage_max_command(float voltage_to_set);
void MPPT_input_current_max_command(float current_to_set);

#endif /* _INC_MPPT_H_ */
