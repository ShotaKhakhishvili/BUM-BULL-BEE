#ifndef MYSRC_SHARP_MANAGER_H
#define MYSRC_SHARP_MANAGER_H

#include <stdint.h>

#include "MySrc/sharp_suggest.h"

typedef enum
{
    SHARP_MODE_LONG = 0,
    SHARP_MODE_SHORT = 1
} SharpMode;

typedef struct
{
    uint32_t last_short_read;
    uint32_t last_long_read;

    int raw_long_adc;
    int raw_short_adc;

    double distance_long_cm;
    double distance_short_cm;
    double selected_distance_cm;

    SharpMode current_mode;
    SharpSuggest suggest;
} SharpManager;

void SharpManager_Init(SharpManager *self);
void SharpManager_Update(SharpManager *self);

double SharpManager_GetSelectedDistance(const SharpManager *self);
double SharpManager_GetLongDistance(const SharpManager *self);
double SharpManager_GetShortDistance(const SharpManager *self);

double SharpManager_GetRawLongDistance(const SharpManager *self);
double SharpManager_GetRawShortDistance(const SharpManager *self);

int SharpManager_GetLongRawAdc(const SharpManager *self);
int SharpManager_GetShortRawAdc(const SharpManager *self);

SharpMode SharpManager_GetMode(const SharpManager *self);

int SharpManager_GetLongAnomalyScore(const SharpManager *self);
int SharpManager_GetShortAnomalyScore(const SharpManager *self);

double SharpManager_AdcToVoltage(int adc, double reference_voltage, double max_adc_value);

double SharpManager_ConvertLongVoltageToDistance(double voltage);
double SharpManager_ConvertShortVoltageToDistance(double voltage);

#endif /* MYSRC_SHARP_MANAGER_H */
