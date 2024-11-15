/**
 *  @file bitops.h
 *  @brief header file for bitops.c. Lots of Inline bit manipulation functionality
 */


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BITOPS_H__
#define __BITOPS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>


#define BITOPS_32BIT_REVERSE(num) \
    ((((num) & 0x000000FFU) << 24U) | \
     (((num) & 0x0000FF00U) << 8U)  | \
     (((num) & 0x00FF0000U) >> 8U)  | \
     (((num) & 0xFF000000U) >> 24U))

#define BITOPS_64BIT_REVERSE(num) \
    ((((num) & 0x00000000000000FFUL) << 56U) | \
     (((num) & 0x000000000000FF00UL) << 40U) | \
     (((num) & 0x0000000000FF0000UL) << 24U) | \
     (((num) & 0x00000000FF000000UL) << 8U)  | \
     (((num) & 0x000000FF00000000UL) >> 8U)  | \
     (((num) & 0x0000FF0000000000UL) >> 24U) | \
     (((num) & 0x00FF000000000000UL) >> 40U) | \
     (((num) & 0xFF00000000000000UL) >> 56U))

typedef union {                 // Union to interpret doubles to an integral
    double d;
    uint64_t u;
} DoubleAsUint64;


#endif /* __BITOPS_H__ */