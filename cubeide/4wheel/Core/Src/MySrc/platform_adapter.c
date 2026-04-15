#include "MySrc/platform_adapter.h"

#include "main.h"

extern volatile uint16_t adc_raw[];

static uint16_t Platform_ReadAdcByIndex(uint32_t index)
{
    if (index >= PLATFORM_ADC_CHANNEL_COUNT)
    {
        return 0U;
    }

    return adc_raw[index];
}

uint32_t Platform_Millis(void)
{
    return HAL_GetTick();
}

uint16_t Platform_ReadIrSmlAdc(void)
{
    return Platform_ReadAdcByIndex(PLATFORM_ADC_IDX_IR_SML);
}

uint16_t Platform_ReadIrMAdc(void)
{
    return Platform_ReadAdcByIndex(PLATFORM_ADC_IDX_IR_M);
}

uint16_t Platform_ReadIrCloseAdc(void)
{
    return Platform_ReadAdcByIndex(PLATFORM_ADC_IDX_IR_CLOSE_A);
}

void Platform_DebugPrintMedians(double short_median, double long_median, uint8_t digits_after_decimal)
{
    (void)short_median;
    (void)long_median;
    (void)digits_after_decimal;
}
