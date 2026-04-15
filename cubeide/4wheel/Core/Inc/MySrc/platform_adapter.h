#ifndef MYSRC_PLATFORM_ADAPTER_H
#define MYSRC_PLATFORM_ADAPTER_H

#include <stdbool.h>
#include <stdint.h>

#ifndef PLATFORM_ADC_CHANNEL_COUNT
#define PLATFORM_ADC_CHANNEL_COUNT 5U
#endif

#ifndef PLATFORM_ADC_IDX_IR_SML
#define PLATFORM_ADC_IDX_IR_SML 1U /* PA2 / ADC1_IN2 */
#endif

#ifndef PLATFORM_ADC_IDX_IR_M
#define PLATFORM_ADC_IDX_IR_M 2U /* PA3 / ADC1_IN3 */
#endif

#ifndef PLATFORM_ADC_IDX_IR_R
#define PLATFORM_ADC_IDX_IR_R 3U /* PA6 / ADC1_IN6 */
#endif

#ifndef PLATFORM_ADC_IDX_IR_L
#define PLATFORM_ADC_IDX_IR_L 4U /* PA7 / ADC1_IN7 */
#endif

#ifndef PLATFORM_ADC_IDX_IR_CLOSE_A
#define PLATFORM_ADC_IDX_IR_CLOSE_A 0U /* PA1 / ADC1_IN1 */
#endif

uint32_t Platform_Millis(void);
uint16_t Platform_ReadIrSmlAdc(void);
uint16_t Platform_ReadIrMAdc(void);
uint16_t Platform_ReadIrCloseAdc(void);

void Platform_DebugPrintMedians(double short_median, double long_median, uint8_t digits_after_decimal);

typedef enum
{
	PLATFORM_WHEEL_OUTPUT_RB = 0,
	PLATFORM_WHEEL_OUTPUT_RF = 1,
	PLATFORM_WHEEL_OUTPUT_LF = 2,
	PLATFORM_WHEEL_OUTPUT_LB = 3
} PlatformWheelOutput;

typedef enum
{
	PLATFORM_LIGHT_INPUT_COL1 = 0,
	PLATFORM_LIGHT_INPUT_COL2 = 1,
	PLATFORM_LIGHT_INPUT_COL3 = 2,
	PLATFORM_LIGHT_INPUT_COL4 = 3,
	PLATFORM_LIGHT_INPUT_RESET = 4
} PlatformLightInput;

void Platform_DelayMs(uint32_t ms);
void Platform_WheelSetPwm(uint8_t output, uint16_t strength_8bit);
bool Platform_ReadDigitalInput(uint8_t input_id);

#endif /* MYSRC_PLATFORM_ADAPTER_H */
