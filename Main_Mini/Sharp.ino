#include "Defines.hpp"
#include "Sharp.hpp"

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
            { 10.0, 3.833 },
            { 11.0, 3.606 },
            { 12.0, 3.364 },
            { 13.0, 3.076 },
            { 14.0, 2.924 },
            { 15.0, 2.712 },
            { 16.0, 2.500 },
            { 17.0, 2.348 },
            { 18.0, 2.258 },
            { 19.0, 2.136 },
            { 20.0, 2.045 },
            { 21.0, 1.955 },
            { 22.0, 1.864 },
            { 23.0, 1.773 },
            { 24.0, 1.712 },
            { 25.0, 1.591 },
            { 26.0, 1.561 },
            { 27.0, 1.500 },
            { 28.0, 1.439 },
            { 29.0, 1.409 },
            { 30.0, 1.379 },
            { 31.0, 1.318},
            { 32.0, 1.288 },
            { 33.0, 1.258 },
            { 34.0, 1.212 },
            { 35.0, 1.182 },
            { 36.0, 1.152 },
            { 37.0, 1.091 },
            { 38.0, 1.061 },
            { 39.0, 1.030 },
            { 40.0, 1.000 },
            { 41.0, 0.970 },
            { 42.0, 0.955 },
            { 43.0, 0.939 },
            { 44.0, 0.977 },
            { 45.0, 1.015 },//aqedan mere tavidan
            { 46.0, 1.000 },
            { 47.0, 0.985 },
            { 48.0, 0.970 },
            { 49.0, 0.955 },
            { 50.0, 0.939 },
            { 51.0, 0.924 },
            { 52.0, 0.909 },
            { 53.0, 0.894 },
            { 54.0, 0.879 },
            { 55.0, 0.864 },
            { 56.0, 0.848 },
            { 57.0, 0.833 },
            { 58.0, 0.818 },
            { 59.0, 0.803 },
            { 60.0, 0.788 },
            { 61.0, 0.780 },
            { 62.0, 0.773 },
            { 63.0, 0.765 },
            { 64.0, 0.758 },
            { 65.0, 0.750 },
            { 66.0, 0.742 },
            { 67.0, 0.750 },
            { 68.0, 0.758 },
            { 69.0, 0.727 },
            { 70.0, 0.697 },
            { 71.0, 0.667 },
            { 72.0, 0.667 },
            { 73.0, 0.667 },
            { 74.0, 0.667 },
            { 75.0, 0.636 },
            { 76.0, 0.636 },
            { 77.0, 0.636 },
            { 78.0, 0.621 },
            { 79.0, 0.606 },
            { 80.0, 0.606 }
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
