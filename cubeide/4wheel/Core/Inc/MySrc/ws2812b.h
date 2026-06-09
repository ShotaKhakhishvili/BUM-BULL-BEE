#ifndef MYSRC_WS2812B_H
#define MYSRC_WS2812B_H

#include <stdint.h>

/* Number of LEDs on your strip */
#define WS2812B_NUM_LEDS    1U

void WS2812B_Init(void);
void WS2812B_SetColor(uint8_t led_index, uint8_t r, uint8_t g, uint8_t b);
void WS2812B_Send(void);

#endif /* MYSRC_WS2812B_H */
