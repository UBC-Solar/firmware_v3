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


/**
 * @brief Copies the source array to the buffer array from start_index to end_index
 * @param buffer: uint8_t pointer to the buffer array
 * @param startIndex: uint32_t start index of the buffer array
 * @param end_index: uint32_t end index of the buffer array
 * @param source: uint8_t pointer to the source array
 * @return void
*/
void UTILS_copy_to_buffer(uint8_t* buffer, uint32_t start_index, uint32_t end_index, uint8_t* source) {
  if (end_index >= start_index) {
    for (uint32_t i = start_index; i <= end_index; i++) {
      buffer[i] = source[i - start_index];
    }
  }
}
