#include "stm32f1xx_hal.h"

#define UBX_HEAD1             0xB5
#define UBX_HEAD2             0x62
#define UBX_CLASS_CFG         0x06
#define UBX_ID_VALSET         0x8A
#define UBX_KEY_CFG_RATE_MEAS 0x30210001u

#define UBX_LAYER_RAM   0x01
#define UBX_LAYER_BBR   0x02
#define UBX_LAYER_FLASH 0x04

static void ubx_cksum(const uint8_t *buf, uint16_t len, uint8_t *ckA, uint8_t *ckB)
{
    *ckA = 0;
    *ckB = 0;

    for (uint16_t i = 0; i < len; i++) {
        *ckA += buf[i];
        *ckB += *ckA;
    }
}

HAL_StatusTypeDef ubx_valset_set_meas(I2C_HandleTypeDef *hi2c, uint8_t addr7bit, uint16_t meas_ms) {

    uint8_t payload[10];
    uint32_t key = UBX_KEY_CFG_RATE_MEAS;

    payload[0] = 0x00;  // version (0x00 - regular, 0x01 - with transaction)

    payload[1] = UBX_LAYER_RAM; // Memory layer

    payload[2] = 0x00;  // reserved
    payload[3] = 0x00;  // reserved

    payload[4] = (uint8_t)(key & 0xFF);
    payload[5] = (uint8_t)((key >> 8) & 0xFF);
    payload[6] = (uint8_t)((key >> 16) & 0xFF);
    payload[7] = (uint8_t)((key >> 24) & 0xFF);

    payload[8] = (uint8_t)(meas_ms & 0xFF);
    payload[9] = (uint8_t)((meas_ms >> 8) & 0xFF);

    uint16_t len = sizeof(payload);

    uint8_t frame[8 + len];

    frame[0] = UBX_HEAD1;
    frame[1] = UBX_HEAD2;
    frame[2] = UBX_CLASS_CFG;
    frame[3] = UBX_ID_VALSET;
    frame[4] = (uint8_t)(len & 0xFF);
    frame[5] = (uint8_t)((len >> 8) & 0xFF);
    
    memcpy(&frame[6], payload, len);

    uint8_t ckA;
    uint8_t ckB;

    ubx_cksum(&frame[2], 4 + len, &ckA, &ckB);

    frame[6 + len] = ckA;
    frame[7 + len] = ckB;

    HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(hi2c, addr7bit << 1, frame, sizeof(frame), HAL_MAX_DELAY);
    return status;
}