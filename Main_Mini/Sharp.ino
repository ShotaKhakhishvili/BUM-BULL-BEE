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
        // { voltage, distanceCm } for GP2Y0A21YK0F, sorted near -> far
        // (descending voltage). Approximate datasheet values — recalibrate
        // against your own ADC readings for best accuracy.
        static const SamplePoint table[] =
        {
            { 2.30, 10.0 },
            { 1.65, 15.0 },
            { 1.30, 20.0 },
            { 1.05, 25.0 },
            { 0.90, 30.0 },
            { 0.78, 35.0 },
            { 0.70, 40.0 },
            { 0.63, 45.0 },
            { 0.58, 50.0 },
            { 0.53, 55.0 },
            { 0.50, 60.0 },
            { 0.47, 65.0 },
            { 0.45, 70.0 },
            { 0.42, 75.0 },
            { 0.40, 80.0 }
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
