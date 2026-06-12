#ifndef MYSRC_SHARP_MANAGER_H
#define MYSRC_SHARP_MANAGER_H

#include <stdint.h>

/*
 * Manages the three forward/side Sharp IR sensors. They are all the same
 * "long" Sharp type, so they are treated uniformly and addressed by a single
 * SharpSensor selector instead of one set of functions per sensor.
 *
 * The middle sensor used to be the short forward Sharp; it has been physically
 * replaced by a VL53L0X time-of-flight sensor, and the middle Sharp here is now
 * simply the long forward sensor (consumed as the Sharp half of ForwardRange).
 */
typedef enum
{
    SHARP_SENSOR_LEFT = 0,
    SHARP_SENSOR_MIDDLE = 1,
    SHARP_SENSOR_RIGHT = 2,
    SHARP_SENSOR_COUNT = 3
} SharpSensor;

typedef struct
{
    uint32_t last_read[SHARP_SENSOR_COUNT];
    int raw_adc[SHARP_SENSOR_COUNT];
    double distance_cm[SHARP_SENSOR_COUNT];
} SharpManager;

void SharpManager_Init(SharpManager *self);
void SharpManager_Update(SharpManager *self);

double SharpManager_GetDistance(const SharpManager *self, SharpSensor sensor);
double SharpManager_GetRawDistance(const SharpManager *self, SharpSensor sensor);
int SharpManager_GetRawAdc(const SharpManager *self, SharpSensor sensor);

double SharpManager_AdcToVoltage(int adc, double reference_voltage, double max_adc_value);

double SharpManager_ConvertVoltageToDistance(double voltage);

#endif /* MYSRC_SHARP_MANAGER_H */
