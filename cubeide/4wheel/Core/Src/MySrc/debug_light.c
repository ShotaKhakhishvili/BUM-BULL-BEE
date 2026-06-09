#include "MySrc/debug_light.h"
#include "main.h"

/* WS2812B bit timing in CPU cycles @ 72 MHz (1 cycle ~= 13.9 ns). */
#define WS_T1H  58u   /* '1' high ~805 ns */
#define WS_T1L  32u   /* '1' low  ~445 ns */
#define WS_T0H  22u   /* '0' high ~305 ns */
#define WS_T0L  58u   /* '0' low  ~805 ns */

static uint8_t ws_r;
static uint8_t ws_g;
static uint8_t ws_b;

static void DWT_Start(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

static inline void delay_cycles(uint32_t cycles)
{
    uint32_t start = DWT->CYCCNT;
    while ((DWT->CYCCNT - start) < cycles)
    {
    }
}

void DebugLight_Init(void)
{
    GPIO_InitTypeDef gpio = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();

    gpio.Pin = GPIO_PIN_0;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &gpio);

    GPIOA->BRR = GPIO_PIN_0;   /* idle low */

    DWT_Start();

    /* boot sanity flash: green */
    DebugLight_SetColor(0, 255, 0);
    DebugLight_Send();
}

void DebugLight_SetColor(uint8_t r, uint8_t g, uint8_t b)
{
    ws_r = r;
    ws_g = g;
    ws_b = b;
}

static void ws_send_byte(uint8_t val)
{
    uint8_t i;

    for (i = 0; i < 8u; i++)
    {
        if (val & 0x80u)
        {
            GPIOA->BSRR = GPIO_PIN_0;
            delay_cycles(WS_T1H);
            GPIOA->BRR = GPIO_PIN_0;
            delay_cycles(WS_T1L);
        }
        else
        {
            GPIOA->BSRR = GPIO_PIN_0;
            delay_cycles(WS_T0H);
            GPIOA->BRR = GPIO_PIN_0;
            delay_cycles(WS_T0L);
        }
        val <<= 1;
    }
}

void DebugLight_Send(void)
{
    uint32_t primask = __get_PRIMASK();

    __disable_irq();

    ws_send_byte(ws_g);
    ws_send_byte(ws_r);
    ws_send_byte(ws_b);

    __set_PRIMASK(primask);

    GPIOA->BRR = GPIO_PIN_0;   /* hold low for reset latch (>50 us) */
    delay_cycles(72u * 60u);
}

bool DebugLight_IsBusy(void)
{
    return false;
}
