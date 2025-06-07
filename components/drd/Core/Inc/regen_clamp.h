#ifndef REGEN_CLAMP_H
#define REGEN_CLAMP_H

#include <stdint.h>
#include <stdbool.h>

/* DEFINES */
#define REGEN_HARD_CAP_FACTOR     0.10f      // 30 % = hard-cap on entry
#define MAX_PACK_CURRENT          18.0f      // never exceed 18 A into pack
#define THROTTLE_REGEN_ZONE_DAC   300        // ≤ 300 == “pedal released”
#define SPIKE_SETTLE_PERIOD_MS    25         // one CAN cycle 
#define PACK_CURRENT_NEGATIVE_UNCERTAINTY       (0.2f)
#define FACTOR_MAX_REGEN                        (1.0f)
#define NUM_POINTS_TO_HARD_CAP                  (2)

/**
 * Calculates the next regen-DAC value.
 *
 * @param throttle_dac   Latest throttle reading (0–1023).
 * @param regen_switch   Physical switch state (true = driver wants regen).
 * @param pack_current   Latest pack current in amps  (negative = charging).
 * @param array_current  Latest array current in amps (positive = discharging).
 *
 * @return 10-bit DAC value to send to Mitsuba (0–1023).
 */
uint16_t RegenClamp_get_regen_dac(uint16_t  throttle_dac,
                                  uint32_t  velocity_kmh,
                                  float     pack_current,
                                  float     array_current);

/* Reset helper */
void RegenClamp_reset(void);

#endif /* REGEN_CLAMP_H */
