#include "MySrc/platform_adapter.h"

#include "main.h"
#include "tim.h"

#define PLATFORM_PWM_MAX_COMPARE 199U

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

void Platform_DelayMs(uint32_t ms)
{
    HAL_Delay(ms);
}

void Platform_WheelSetPwm(uint8_t output, uint16_t strength_8bit)
{
    uint32_t compare_value;

    if (strength_8bit > 255U)
    {
        strength_8bit = 255U;
    }

    compare_value = (((uint32_t)strength_8bit * PLATFORM_PWM_MAX_COMPARE) + 127U) / 255U;

    switch (output)
    {
    case PLATFORM_WHEEL_OUTPUT_RB:
        __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, compare_value);
        break;
    case PLATFORM_WHEEL_OUTPUT_RF:
        __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, compare_value);
        break;
    case PLATFORM_WHEEL_OUTPUT_LF:
        __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_3, compare_value);
        break;
    case PLATFORM_WHEEL_OUTPUT_LB:
        __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_4, compare_value);
        break;
    default:
        break;
    }
}
