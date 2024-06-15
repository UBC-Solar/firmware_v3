#include "utils.h"

/**
 * @brief Zeros out an array.
 * @param array The array to zero out.
 * @param size The size of the array.
 * @return void
 */
void UTILS_zeroArray(uint8_t* array, uint8_t size) {
    for (uint8_t i = 0; i < size; i++) {
        array[i] = 0;
    }
}
