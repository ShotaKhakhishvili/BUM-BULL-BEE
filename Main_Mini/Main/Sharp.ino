#include "Defines.hpp"
#include "Sharp.hpp"
#include "SharpMedian.hpp"

namespace SharpConvertions
{
    double InterpolateDistanceFromVoltage(
        double voltage,
        const SamplePoint* table,
        int count)
    {
        if (count < 2)
            return -1.0;

        if (voltage >= table[0].voltage)
            return table[0].distanceCm;

        if (voltage <= table[count - 1].voltage)
            return table[count - 1].distanceCm;

        for (int i = 0; i < count - 1; ++i)
        {
            const double vNear = table[i].voltage;
            const double vFar  = table[i + 1].voltage;

            if (voltage <= vNear && voltage >= vFar)
            {
                const double dNear = table[i].distanceCm;
                const double dFar  = table[i + 1].distanceCm;

                const double t = (voltage - vNear) / (vFar - vNear);
                return dNear + (dFar - dNear) * t;
            }
        }

        return table[count - 1].distanceCm;
    }

    double ConvertLongVoltageToDistance(double voltage)
    {
        static const SamplePoint table[] =
        {
            { 3.833, 10.0 },
            { 3.606, 11.0 },
            { 3.364, 12.0 },
            { 3.076, 13.0 },
            { 2.924, 14.0 },
            { 2.712, 15.0 },
            { 2.500, 16.0 },
            { 2.348, 17.0 },
            { 2.258, 18.0 },
            { 2.136, 19.0 },
            { 2.045, 20.0 },
            { 1.955, 21.0 },
            { 1.864, 22.0 },
            { 1.773, 23.0 },
            { 1.712, 24.0 },
            { 1.591, 25.0 },
            { 1.561, 26.0 },
            { 1.500, 27.0 },
            { 1.439, 28.0 },
            { 1.409, 29.0 },
            { 1.379, 30.0 },
            { 1.318, 31.0 },
            { 1.288, 32.0 },
            { 1.258, 33.0 },
            { 1.212, 34.0 },
            { 1.182, 35.0 },
            { 1.152, 36.0 },
            { 1.091, 37.0 },
            { 1.061, 38.0 },
            { 1.030, 39.0 },
            { 1.000, 40.0 },
            { 0.970, 41.0 },
            { 0.955, 42.0 },
            { 0.939, 43.0 },
            { 0.977, 44.0 },
            { 1.015, 45.0 },
            { 1.000, 46.0 },
            { 0.985, 47.0 },
            { 0.970, 48.0 },
            { 0.955, 49.0 },
            { 0.939, 50.0 },
            { 0.924, 51.0 },
            { 0.909, 52.0 },
            { 0.894, 53.0 },
            { 0.879, 54.0 },
            { 0.864, 55.0 },
            { 0.848, 56.0 },
            { 0.833, 57.0 },
            { 0.818, 58.0 },
            { 0.803, 59.0 },
            { 0.788, 60.0 },
            { 0.780, 61.0 },
            { 0.773, 62.0 },
            { 0.765, 63.0 },
            { 0.758, 64.0 },
            { 0.750, 65.0 },
            { 0.742, 66.0 },
            { 0.750, 67.0 },
            { 0.758, 68.0 },
            { 0.727, 69.0 },
            { 0.697, 70.0 },
            { 0.667, 71.0 },
            { 0.667, 72.0 },
            { 0.667, 73.0 },
            { 0.667, 74.0 },
            { 0.636, 75.0 },
            { 0.636, 76.0 },
            { 0.636, 77.0 },
            { 0.621, 78.0 },
            { 0.606, 79.0 },
            { 0.606, 80.0 }
        };

        return InterpolateDistanceFromVoltage(voltage, table, sizeof(table) / sizeof(table[0]));
    }
}

void Sharp::Init()
{
    lastUpdateTime = -SHARP_UPDATE_INTERVAL;
    seeded = false;

    Distance();
}

double Sharp::Distance()
{
    const unsigned long now = millis();

    if(now >= SHARP_UPDATE_INTERVAL + lastUpdateTime)
    {
        this->rawAdc = analogRead(IR);
        this->volt = AdcToVoltage(rawAdc);

        SharpMedian::Update(volt);

        const double reading = SharpConvertions::ConvertLongVoltageToDistance(volt);

        if (!seeded)
        {
            this->distance = reading;
            seeded = true;
        }
        else
        {
            this->distance = SHARP_SMOOTHING_ALPHA * reading
                           + (1.0 - SHARP_SMOOTHING_ALPHA) * this->distance;
        }

        lastUpdateTime = now;
    }

    return distance;
}

void Sharp::DebugPrint()
{
    this->Distance();
    
    Serial.print("Sharp Data : | Raw Adc: ");
    Serial.print(rawAdc);
    Serial.print(" | Voltage: ");
    Serial.print(volt, kPrintDigitsAfterDecimal);
    Serial.print(" | Distance: ");
    Serial.println(distance, kPrintDigitsAfterDecimal);
}
