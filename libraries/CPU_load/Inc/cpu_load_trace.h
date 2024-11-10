/*
 * cpu_load_trace.h
 *
 *  Created on: Nov 9, 2024
 *      Author: diego
 */

#ifndef CPU_LOAD_INC_CPU_LOAD_TRACE_H_
#define CPU_LOAD_INC_CPU_LOAD_TRACE_H_



#endif /* CPU_LOAD_INC_CPU_LOAD_TRACE_H_ */

extern void taskSwitchedIn(void);
extern void taskSwitchedOut(void);

#ifndef traceTASK_SWITCHED_IN
#define traceTASK_SWITCHED_IN() taskSwitchedIn()
#endif

#ifndef traceTASK_SWITCHED_OUT
#define traceTASK_SWITCHED_OUT() taskSwitchedOut()
#endif
