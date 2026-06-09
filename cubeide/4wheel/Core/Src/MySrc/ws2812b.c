#include "MySrc/ws2812b.h"
#include "tim.h"      /* htim2 from CubeMX */
#include "stm32f1xx_hal.h"

/*
 * WS2812B bit timing at 72 MHz, ARR = 89 (period = 90 ticks = 1.25 us):
 *   Logic 1 : ~0.8 us HIGH  -> CCR = 58  (64% duty)
 *   Logic 0 : ~0.4 us HIGH  -> CCR = 29  (32% duty)
 *   Reset   : CCR = 0  for >= 50 us -> at least 40 zero-words @ 1.25 us each
 */
#define WS2812B_HIGH    58U
#define WS2812B_LOW     29U
#define WS2812B_RESET   50U   /* number of 0-CCR words for reset pulse */

/* 24 bits per LED + reset words at the end */
#define WS2812B_BUF_SIZE  (WS2812B_NUM_LEDS * 24U + WS2812B_RESET)

static uint16_t s_dma_buf[WS2812B_BUF_SIZE];

/* Pixel storage in GRB order (WS2812B wire order) */
static uint8_t s_grb[WS2812B_NUM_LEDS][3]; /* [i][0]=G [i][1]=R [i][2]=B */

static volatile uint8_t s_tx_done = 1U;

/* ------------------------------------------------------------------ */
/*  HAL DMA transfer-complete callback                                  */
/* ------------------------------------------------------------------ */
void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2)
    {
        HAL_TIM_PWM_Stop_DMA(htim, TIM_CHANNEL_1);
        s_tx_done = 1U;
    }
}

/* ------------------------------------------------------------------ */

void WS2812B_Init(void)
{
    uint32_t i;
    for (i = 0U; i < WS2812B_BUF_SIZE; ++i)
        s_dma_buf[i] = 0U;
    /* default all pixels off */
    for (i = 0U; i < WS2812B_NUM_LEDS; ++i)
        s_grb[i][0] = s_grb[i][1] = s_grb[i][2] = 0U;
}

void WS2812B_SetColor(uint8_t led_index, uint8_t r, uint8_t g, uint8_t b)
{
    if (led_index >= WS2812B_NUM_LEDS) return;
    s_grb[led_index][0] = g;   /* WS2812B expects G first on the wire */
    s_grb[led_index][1] = r;
    s_grb[led_index][2] = b;
}

void WS2812B_Send(void)
{
    uint32_t led, byte, bit, buf_idx = 0U;

    /* Wait for any previous transfer to finish */
    while (!s_tx_done) {}
    s_tx_done = 0U;

    /* Build the DMA buffer */
    for (led = 0U; led < WS2812B_NUM_LEDS; ++led)
    {
        for (byte = 0U; byte < 3U; ++byte)
        {
            uint8_t val = s_grb[led][byte];
            for (bit = 0U; bit < 8U; ++bit)
            {
                /* MSB first */
                s_dma_buf[buf_idx++] =
                    (val & 0x80U) ? WS2812B_HIGH : WS2812B_LOW;
                val <<= 1U;
            }
        }
    }

    /* Reset words - CCR = 0 keeps the line LOW */
    for (bit = 0U; bit < WS2812B_RESET; ++bit)
        s_dma_buf[buf_idx++] = 0U;

    /* Fire DMA */
    HAL_TIM_PWM_Start_DMA(&htim2, TIM_CHANNEL_1,
                           (uint32_t *)s_dma_buf,
                           WS2812B_BUF_SIZE);
}
