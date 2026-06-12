#include "MySrc/sharp_manager.h"

#include "MySrc/platform_adapter.h"
#include "MySrc/sharp_volt_to_distance.h"

#include <stdbool.h>

#define SHARP_DELAY_MS 40U

#define SHARP_ALPHA 0.3

static bool SharpManager_IsValidSensor(SharpSensor sensor)
{
    return (sensor >= SHARP_SENSOR_LEFT) && (sensor < SHARP_SENSOR_COUNT);
}

static uint16_t SharpManager_ReadSensorAdc(SharpSensor sensor)
{
    switch (sensor)
    {
        case SHARP_SENSOR_LEFT:
            return Platform_ReadIrLAdc();
        case SHARP_SENSOR_MIDDLE:
            return Platform_ReadIrMAdc();
        case SHARP_SENSOR_RIGHT:
            return Platform_ReadIrRAdc();
        default:
            return 0U;
    }
}

static void SharpManager_UpdateSensor(SharpManager *self, SharpSensor sensor, uint32_t now)
{
    if ((now - self->last_read[sensor]) >= SHARP_DELAY_MS)
    {
        double new_distance;

        self->raw_adc[sensor] = (int)SharpManager_ReadSensorAdc(sensor);

        new_distance = SharpManager_GetRawDistance(self, sensor);
        self->distance_cm[sensor] =
            (self->distance_cm[sensor] * (1.0 - SHARP_ALPHA)) +
            (new_distance * SHARP_ALPHA);

        self->last_read[sensor] = now;
    }
}

void SharpManager_Init(SharpManager *self)
{
    uint32_t now;
    SharpSensor sensor;

    if (self == 0)
    {
        return;
    }

    now = Platform_Millis();

    for (sensor = SHARP_SENSOR_LEFT; sensor < SHARP_SENSOR_COUNT; ++sensor)
    {
        self->last_read[sensor] = now - SHARP_DELAY_MS - 1U;
        self->raw_adc[sensor] = (int)SharpManager_ReadSensorAdc(sensor);
        self->distance_cm[sensor] = SharpManager_GetRawDistance(self, sensor);
    }
}

void SharpManager_Update(SharpManager *self)
{
    uint32_t now;
    SharpSensor sensor;

    if (self == 0)
    {
        return;
    }

    now = Platform_Millis();

    for (sensor = SHARP_SENSOR_LEFT; sensor < SHARP_SENSOR_COUNT; ++sensor)
    {
        SharpManager_UpdateSensor(self, sensor, now);
    }
}

double SharpManager_GetDistance(const SharpManager *self, SharpSensor sensor)
{
    if ((self == 0) || !SharpManager_IsValidSensor(sensor))
    {
        return 0.0;
    }

    return self->distance_cm[sensor];
}

double SharpManager_GetRawDistance(const SharpManager *self, SharpSensor sensor)
{
    if ((self == 0) || !SharpManager_IsValidSensor(sensor))
    {
        return 0.0;
    }

    return SharpManager_ConvertVoltageToDistance(
        SharpManager_AdcToVoltage(self->raw_adc[sensor], 3.3, 4095.0));
}

int SharpManager_GetRawAdc(const SharpManager *self, SharpSensor sensor)
{
    if ((self == 0) || !SharpManager_IsValidSensor(sensor))
    {
        return 0;
    }

    return self->raw_adc[sensor];
}

double SharpManager_AdcToVoltage(int adc, double reference_voltage, double max_adc_value)
{
    return ((double)adc / max_adc_value) * reference_voltage;
}

double SharpManager_ConvertVoltageToDistance(double voltage)
{
    return SharpConverter_ConvertLongVoltageToDistance(voltage);
}
