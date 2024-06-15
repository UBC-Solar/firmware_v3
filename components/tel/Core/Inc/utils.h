#include <stdint.h>

/**
 * @brief Union to convert float to bytes
*/
typedef union Utils_FloatBytes_t {
    float float_value;
    uint8_t bytes[4];
} Utils_FloatBytes_t;


/**
 * @brief Union to convert double to bytes
*/
typedef union Utils_DoubleBytes_t {
	double double_value;			/**< Double value member of the union. */
	uint64_t double_as_int;			/**< 64 bit in member of union. */
} Utils_DoubleBytes_t;

void UTILS_zeroArray(uint8_t* array, uint8_t size);

/**
 * @brief Gets the byte at the index i of the data
*/
#define UTILS_GET_BYTE_AT_INDEX(i, value) ((value >> (i * 8)) & 0xFF);   
