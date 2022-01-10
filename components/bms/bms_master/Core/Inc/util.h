#ifndef INC_UTIL_H_
#define INC_UTIL_H_

// Create a "testable" static type for private functions that are tested
#ifdef TEST
 #define STATIC_TESTABLE
#else
 #define STATIC_TESTABLE static
#endif // TEST

#endif // INC_UTIL_H_
