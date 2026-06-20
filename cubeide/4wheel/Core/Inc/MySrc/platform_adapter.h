#ifndef MYSRC_PLATFORM_ADAPTER_H
#define MYSRC_PLATFORM_ADAPTER_H

#include <stdbool.h>
#include <stdint.h>

#ifndef PLATFORM_ADC_CHANNEL_COUNT
#define PLATFORM_ADC_CHANNEL_COUNT 5U
#endif

/* These indices are slots in adc_raw[], which mirrors the ADC scan order
 * rank1..5 = IN1,IN2,IN3,IN6,IN7 = PA1,PA2,PA3,PA6,PA7. Slot 0 (PA1, magnet)
 * and 2 (PA3, unused) are skipped; the Sharps live at 1, 3, 4. */

#ifndef PLATFORM_ADC_IDX_IR_M
#define PLATFORM_ADC_IDX_IR_M 1U /* PA2 / ADC1_IN2 */
#endif

#ifndef PLATFORM_ADC_IDX_IR_R
#define PLATFORM_ADC_IDX_IR_R 4U /* PA7 / ADC1_IN7 */
#endif

#ifndef PLATFORM_ADC_IDX_IR_L
#define PLATFORM_ADC_IDX_IR_L 3U /* PA6 / ADC1_IN6 */
#endif

uint32_t Platform_Millis(void);
uint16_t Platform_ReadIrMAdc(void);
uint16_t Platform_ReadIrRAdc(void);
uint16_t Platform_ReadIrLAdc(void);

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

/* Magnet attachment as on/off GPIO on PA1 (was PWM on PA11 / TIM1_CH4). The
 * compare argument is treated as a threshold: non-zero energizes, zero releases.
 * Function names kept for API compatibility with the Magnet module. */
void Platform_MagnetStartPwm(void);
void Platform_MagnetSetCompare(uint16_t compare);

#endif /* MYSRC_PLATFORM_ADAPTER_H */
