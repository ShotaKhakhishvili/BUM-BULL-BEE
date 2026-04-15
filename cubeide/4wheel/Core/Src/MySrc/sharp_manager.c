#include "MySrc/sharp_manager.h"

#include "MySrc/platform_adapter.h"
#include "MySrc/sharp_volt_to_distance.h"

#include <stdbool.h>

#define SHARP_SWITCH_TO_LONG_CM 13.5
#define SHARP_SWITCH_TO_SHORT_CM 11.5

#define SHARP_DELAY_SHORT_MS 16U
#define SHARP_DELAY_LONG_MS 40U

#define SHARP_ALPHA_LONG 0.3
#define SHARP_ALPHA_SHORT 0.3

#define SHARP_USE_DERIVATIVE_SUGGESTION false

static void SharpManager_RefreshSelectedDistance(SharpManager *self)
{
    if (self->current_mode == SHARP_MODE_LONG)
    {
        self->selected_distance_cm = self->distance_long_cm;
    }
    else
    {
        self->selected_distance_cm = self->distance_short_cm;
    }
}

static void SharpManager_RefreshModeByDistance(SharpManager *self)
{
    if (self->current_mode == SHARP_MODE_LONG)
    {
        if (self->selected_distance_cm < SHARP_SWITCH_TO_SHORT_CM)
        {
            self->current_mode = SHARP_MODE_SHORT;
        }
    }
    else if (self->selected_distance_cm > SHARP_SWITCH_TO_LONG_CM)
    {
        self->current_mode = SHARP_MODE_LONG;
    }
}

static void SharpManager_UpdateLongSensor(SharpManager *self, uint32_t now)
{
    if ((now - self->last_long_read) >= SHARP_DELAY_LONG_MS)
    {
        self->raw_long_adc = (int)Platform_ReadIrMAdc();

        {
            double new_distance = SharpManager_GetRawLongDistance(self);
            self->distance_long_cm =
                (self->distance_long_cm * (1.0 - SHARP_ALPHA_LONG)) +
                (new_distance * SHARP_ALPHA_LONG);
        }

        self->last_long_read = now;
    }
}

static void SharpManager_UpdateShortSensor(SharpManager *self, uint32_t now)
{
    if ((now - self->last_short_read) >= SHARP_DELAY_SHORT_MS)
    {
        self->raw_short_adc = (int)Platform_ReadIrSmlAdc();

        {
            double new_distance = SharpManager_GetRawShortDistance(self);
            self->distance_short_cm =
                (self->distance_short_cm * (1.0 - SHARP_ALPHA_SHORT)) +
                (new_distance * SHARP_ALPHA_SHORT);
        }

        self->last_short_read = now;
    }
}

void SharpManager_Init(SharpManager *self)
{
    uint32_t now;

    if (self == 0)
    {
        return;
    }

    now = Platform_Millis();

    self->last_short_read = now - SHARP_DELAY_SHORT_MS - 1U;
    self->last_long_read = now - SHARP_DELAY_LONG_MS - 1U;

    self->raw_long_adc = (int)Platform_ReadIrMAdc();
    self->raw_short_adc = (int)Platform_ReadIrSmlAdc();

    self->distance_long_cm = SharpManager_GetRawLongDistance(self);
    self->distance_short_cm = SharpManager_GetRawShortDistance(self);

    self->current_mode = SHARP_MODE_LONG;
    SharpManager_RefreshSelectedDistance(self);

    SharpSuggest_Init(&self->suggest);
}

void SharpManager_Update(SharpManager *self)
{
    uint32_t now;

    if (self == 0)
    {
        return;
    }

    now = Platform_Millis();

    SharpManager_UpdateLongSensor(self, now);
    SharpManager_UpdateShortSensor(self, now);

    SharpSuggest_Update(&self->suggest, self->raw_long_adc, self->raw_short_adc);

    SharpManager_RefreshSelectedDistance(self);
    SharpManager_RefreshModeByDistance(self);
    SharpManager_RefreshSelectedDistance(self);

    if (!SHARP_USE_DERIVATIVE_SUGGESTION)
    {
        return;
    }

    if (self->current_mode == SHARP_MODE_LONG)
    {
        if (SharpSuggest_ShouldSuggestShortFromLong(&self->suggest))
        {
            self->current_mode = SHARP_MODE_SHORT;
        }
    }
    else if (SharpSuggest_ShouldSuggestLongFromShort(&self->suggest))
    {
        self->current_mode = SHARP_MODE_LONG;
    }

    SharpManager_RefreshSelectedDistance(self);
}

double SharpManager_GetSelectedDistance(const SharpManager *self)
{
    return (self == 0) ? 0.0 : self->selected_distance_cm;
}

double SharpManager_GetLongDistance(const SharpManager *self)
{
    return (self == 0) ? 0.0 : self->distance_long_cm;
}

double SharpManager_GetShortDistance(const SharpManager *self)
{
    return (self == 0) ? 0.0 : self->distance_short_cm;
}

double SharpManager_GetRawLongDistance(const SharpManager *self)
{
    if (self == 0)
    {
        return 0.0;
    }

    return SharpManager_ConvertLongVoltageToDistance(
        SharpManager_AdcToVoltage(self->raw_long_adc, 3.3, 4095.0));
}

double SharpManager_GetRawShortDistance(const SharpManager *self)
{
    if (self == 0)
    {
        return 0.0;
    }

    return SharpManager_ConvertShortVoltageToDistance(
        SharpManager_AdcToVoltage(self->raw_short_adc, 3.3, 4095.0));
}

int SharpManager_GetLongRawAdc(const SharpManager *self)
{
    return (self == 0) ? 0 : self->raw_long_adc;
}

int SharpManager_GetShortRawAdc(const SharpManager *self)
{
    return (self == 0) ? 0 : self->raw_short_adc;
}

SharpMode SharpManager_GetMode(const SharpManager *self)
{
    return (self == 0) ? SHARP_MODE_LONG : self->current_mode;
}

int SharpManager_GetLongAnomalyScore(const SharpManager *self)
{
    if (self == 0)
    {
        return 0;
    }

    return SharpSuggest_GetLongAnomalyScore(&self->suggest);
}

int SharpManager_GetShortAnomalyScore(const SharpManager *self)
{
    if (self == 0)
    {
        return 0;
    }

    return SharpSuggest_GetShortAnomalyScore(&self->suggest);
}

double SharpManager_AdcToVoltage(int adc, double reference_voltage, double max_adc_value)
{
    return ((double)adc / max_adc_value) * reference_voltage;
}

double SharpManager_ConvertLongVoltageToDistance(double voltage)
{
    return SharpConverter_ConvertLongVoltageToDistance(voltage);
}

double SharpManager_ConvertShortVoltageToDistance(double voltage)
{
    return SharpConverter_ConvertShortVoltageToDistance(voltage);
}
