#include "MySrc/debug_light.h"
#include "tim.h"
#include "main.h"

#define WS2812B_BITS     24
#define WS2812B_RESET    50
#define WS2812B_BUF_LEN (WS2812B_BITS + WS2812B_RESET)

#define WS2812B_ARR      89
#define WS2812B_ONE      58
#define WS2812B_ZERO     29

extern DMA_HandleTypeDef hdma_tim2_ch1;

static uint16_t ws_buf[WS2812B_BUF_LEN];
static volatile bool ws_busy = false;

void DebugLight_Init(void)
{
    uint32_t i;
    GPIO_InitTypeDef gpio = {0};

    gpio.Pin = GPIO_PIN_0;
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &gpio);

    __HAL_TIM_SET_AUTORELOAD(&htim2, WS2812B_ARR);
    __HAL_TIM_SET_COUNTER(&htim2, 0);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 0);

    HAL_DMA_DeInit(&hdma_tim2_ch1);
    hdma_tim2_ch1.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_tim2_ch1.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_tim2_ch1.Init.MemInc = DMA_MINC_ENABLE;
    hdma_tim2_ch1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_tim2_ch1.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_tim2_ch1.Init.Mode = DMA_NORMAL;
    hdma_tim2_ch1.Init.Priority = DMA_PRIORITY_LOW;
    HAL_DMA_Init(&hdma_tim2_ch1);

    for (i = 0; i < WS2812B_BUF_LEN; i++)
    {
        ws_buf[i] = 0;
    }

    DebugLight_SetColor(0, 255, 0);
    DebugLight_Send();
    while (ws_busy) {}
}

void DebugLight_SetColor(uint8_t r, uint8_t g, uint8_t b)
{
    uint32_t grb;
    int i;

    grb = ((uint32_t)g << 16) | ((uint32_t)r << 8) | (uint32_t)b;

    for (i = 0; i < 24; i++)
    {
        ws_buf[i] = (grb & (1U << (23 - i))) ? WS2812B_ONE : WS2812B_ZERO;
    }
}

void DebugLight_Send(void)
{
    if (ws_busy)
    {
        return;
    }

    ws_busy = true;
    if (HAL_TIM_PWM_Start_DMA(&htim2, TIM_CHANNEL_1,
                               (uint32_t *)ws_buf, WS2812B_BUF_LEN) != HAL_OK)
    {
        ws_busy = false;
    }
}

bool DebugLight_IsBusy(void)
{
    return ws_busy;
}

void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2)
    {
        HAL_TIM_PWM_Stop_DMA(&htim2, TIM_CHANNEL_1);
        ws_busy = false;
    }
}
