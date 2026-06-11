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
            { 1.511, 7.0 },
            { 1.455, 8.0 },
            { 1.333, 9.0 },
            { 1.255, 10.0 },
            { 1.162, 11.0 },
            { 1.079, 12.0 },
            { 1.008, 13.0 },
            { 0.979, 14.0 },
            { 0.942, 15.0 },
            { 0.901, 16.0 },
            { 0.852, 17.0 },
            { 0.818, 18.0 },
            { 0.815, 19.0 },
            { 0.789, 20.0 },
            { 0.715, 25.0 },
            { 0.640, 30.0 },
            { 0.593, 35.0 },
            { 0.554, 40.0 },
            { 0.544, 45.0 },
            { 0.526, 50.0 },
            { 0.498, 55.0 },
            { 0.488, 60.0 },
            { 0.480, 65.0 },
            { 0.470, 70.0 }
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
