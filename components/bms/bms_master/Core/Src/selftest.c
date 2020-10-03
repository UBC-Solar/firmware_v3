/**
 * @file selftest.c
 * @brief BMS startup self tests
 *
 * @date 2020/10/03
 * @author
 */

#include "selftest.h"

/**
 * @brief Checks internal die temperature of LTC6813's for safe operating condition
 *
 * @return If at least one LTC6813 has a die temperature nearing thermal shutdown
 * threshold, returns an error with the device index of the first overheating IC
 */
BTM_Status_t ST_checkLTCtemp()
{
    BTM_Status_t status = {BTM_OK, 0};

    // ...
    // Maybe use CMD_ADSTAT_ITMP

    // Remember most things with the LTC6813 are accomplished in 2 steps:
    // a) Send a command to have it perform on operation
    // b) Read a register to get the results from that operation

    return status;
}
