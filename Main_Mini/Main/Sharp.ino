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
            { 3.022, 7.0 },
            { 2.910, 8.0 },
            { 2.666, 9.0 },
            { 2.510, 10.0 },
            { 2.324, 11.0 },
            { 2.158, 12.0 },
            { 2.016, 13.0 },
            { 1.958, 14.0 },
            { 1.884, 15.0 },
            { 1.802, 16.0 },
            { 1.704, 17.0 },
            { 1.636, 18.0 },
            { 1.630, 19.0 },
            { 1.578, 20.0 },
            { 1.430, 25.0 },
            { 1.280, 30.0 },
            { 1.186, 35.0 },
            { 1.108, 40.0 },
            { 1.088, 45.0 },
            { 1.052, 50.0 },
            { 0.996, 55.0 },
            { 0.976, 60.0 },
            { 0.960, 65.0 }
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
        this->rawAdc = analogRead(SHARP_PIN);
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
