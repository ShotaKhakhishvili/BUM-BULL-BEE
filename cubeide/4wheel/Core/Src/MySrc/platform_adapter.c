#include "MySrc/platform_adapter.h"

#include "main.h"
#include "tim.h"

#include <stdio.h>

#define PLATFORM_PWM_MAX_COMPARE 199U
#define PLATFORM_UART_TX_TIMEOUT_MS 50U
#define PLATFORM_MAX_PRINT_DIGITS 6U

extern volatile uint16_t adc_raw[];

static void Platform_Usart1Transmit(const uint8_t *data, uint16_t length, uint32_t timeout_ms)
{
    uint32_t i;
    uint32_t start;

    if ((data == 0) || (length == 0U))
    {
        return;
    }

    start = HAL_GetTick();

    for (i = 0U; i < length; ++i)
    {
        while ((USART1->SR & USART_SR_TXE) == 0U)
        {
            if ((HAL_GetTick() - start) >= timeout_ms)
            {
                return;
            }
        }

        USART1->DR = data[i];
    }

    while ((USART1->SR & USART_SR_TC) == 0U)
    {
        if ((HAL_GetTick() - start) >= timeout_ms)
        {
            return;
        }
    }
}

static uint32_t Platform_Pow10(uint8_t exponent)
{
    uint32_t value = 1U;
    uint8_t i;

    for (i = 0U; i < exponent; ++i)
    {
        value *= 10U;
    }

    return value;
}

static void Platform_FormatFixed(char *buffer, size_t buffer_len, double value, uint8_t digits_after_decimal)
{
    bool is_negative;
    uint32_t scale;
    uint32_t scaled_value;
    uint32_t integer_part;
    uint32_t fractional_part;

    if ((buffer == 0) || (buffer_len == 0U))
    {
        return;
    }

    if (digits_after_decimal > PLATFORM_MAX_PRINT_DIGITS)
    {
        digits_after_decimal = PLATFORM_MAX_PRINT_DIGITS;
    }

    is_negative = (value < 0.0);
    if (is_negative)
    {
        value = -value;
    }

    scale = Platform_Pow10(digits_after_decimal);
    scaled_value = (uint32_t)((value * (double)scale) + 0.5);
    integer_part = scaled_value / scale;
    fractional_part = scaled_value % scale;

    if (digits_after_decimal == 0U)
    {
        (void)snprintf(
            buffer,
            buffer_len,
            "%s%lu",
            is_negative ? "-" : "",
            (unsigned long)integer_part);
    }
    else
    {
        (void)snprintf(
            buffer,
            buffer_len,
            "%s%lu.%0*lu",
            is_negative ? "-" : "",
            (unsigned long)integer_part,
            (int)digits_after_decimal,
            (unsigned long)fractional_part);
    }
}

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
    char short_buffer[24];
    char long_buffer[24];
    char tx_line[96];
    int line_length;
    uint16_t tx_length;

    if (((RCC->APB2ENR & RCC_APB2ENR_USART1EN) == 0U) ||
        ((USART1->CR1 & USART_CR1_UE) == 0U))
    {
        return;
    }

    Platform_FormatFixed(short_buffer, sizeof(short_buffer), short_median, digits_after_decimal);
    Platform_FormatFixed(long_buffer, sizeof(long_buffer), long_median, digits_after_decimal);

    line_length = snprintf(
        tx_line,
        sizeof(tx_line),
        "Short Median: %s | Long Median: %s\r\n",
        short_buffer,
        long_buffer);

    if (line_length <= 0)
    {
        return;
    }

    if (line_length >= (int)sizeof(tx_line))
    {
        tx_length = (uint16_t)(sizeof(tx_line) - 1U);
    }
    else
    {
        tx_length = (uint16_t)line_length;
    }

    Platform_Usart1Transmit((const uint8_t *)tx_line, tx_length, PLATFORM_UART_TX_TIMEOUT_MS);
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

bool Platform_ReadDigitalInput(uint8_t input_id)
{
    GPIO_TypeDef *port;
    uint16_t pin;
    GPIO_PinState state;

    switch (input_id)
    {
    case PLATFORM_LIGHT_INPUT_COL1:
        port = GPIOA;
        pin = GPIO_PIN_15;
        break;
    case PLATFORM_LIGHT_INPUT_COL2:
        port = GPIOB;
        pin = GPIO_PIN_3;
        break;
    case PLATFORM_LIGHT_INPUT_COL3:
        port = GPIOB;
        pin = GPIO_PIN_5;
        break;
    case PLATFORM_LIGHT_INPUT_COL4:
        port = GPIOB;
        pin = GPIO_PIN_4;
        break;
    case PLATFORM_LIGHT_INPUT_RESET:
        port = GPIOB;
        pin = GPIO_PIN_13;
        break;
    default:
        return false;
    }

    state = HAL_GPIO_ReadPin(port, pin);
    return (state == GPIO_PIN_SET);
}
