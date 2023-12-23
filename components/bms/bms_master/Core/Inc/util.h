#ifndef INC_UTIL_H_
#define INC_UTIL_H_

#include "pack.h"

// Create a "testable" static type for private functions that are tested
#ifdef TEST
 #define STATIC_TESTABLE
#else
 #define STATIC_TESTABLE static
#endif // TEST

void UTIL_printModuleVoltages(Pack_t *pack);
void UTIL_printModuleTemps(Pack_t *pack);

#endif // INC_UTIL_H_