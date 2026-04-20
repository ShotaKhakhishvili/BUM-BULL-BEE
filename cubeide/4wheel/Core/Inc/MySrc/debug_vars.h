#ifndef MYSRC_DEBUG_VARS_H
#define MYSRC_DEBUG_VARS_H

#include <stdint.h>

extern volatile float g_debug_short_median;
extern volatile float g_debug_long_median;

extern volatile int32_t g_debug_short_raw_adc;
extern volatile int32_t g_debug_long_raw_adc;
extern volatile int32_t g_debug_left_raw_adc;
extern volatile int32_t g_debug_right_raw_adc;

extern volatile float g_debug_short_voltage;
extern volatile float g_debug_long_voltage;
extern volatile float g_debug_left_voltage;
extern volatile float g_debug_right_voltage;

#endif /* MYSRC_DEBUG_VARS_H */
