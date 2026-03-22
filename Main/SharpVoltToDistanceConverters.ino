#include "SharpManager.hpp"

namespace SharpManager
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
            { 10.0, 2.30 },
            { 11.0, 2.15 },
            { 12.0, 2.02 },
            { 13.0, 1.90 },
            { 14.0, 1.79 },
            { 15.0, 1.69 },
            { 16.0, 1.60 },
            { 17.0, 1.52 },
            { 18.0, 1.45 },
            { 19.0, 1.38 },
            { 20.0, 1.32 },
            { 21.0, 1.27 },
            { 22.0, 1.22 },
            { 23.0, 1.17 },
            { 24.0, 1.13 },
            { 25.0, 1.09 },
            { 26.0, 1.05 },
            { 27.0, 1.02 },
            { 28.0, 0.99 },
            { 29.0, 0.96 },
            { 30.0, 0.93 },
            { 31.0, 0.90 },
            { 32.0, 0.88 },
            { 33.0, 0.86 },
            { 34.0, 0.84 },
            { 35.0, 0.82 },
            { 36.0, 0.80 },
            { 37.0, 0.78 },
            { 38.0, 0.76 },
            { 39.0, 0.74 },
            { 40.0, 0.72 },
            { 41.0, 0.71 },
            { 42.0, 0.70 },
            { 43.0, 0.69 },
            { 44.0, 0.68 },
            { 45.0, 0.67 },
            { 46.0, 0.66 },
            { 47.0, 0.65 },
            { 48.0, 0.64 },
            { 49.0, 0.63 },
            { 50.0, 0.62 },
            { 51.0, 0.61 },
            { 52.0, 0.60 },
            { 53.0, 0.59 },
            { 54.0, 0.58 },
            { 55.0, 0.57 },
            { 56.0, 0.56 },
            { 57.0, 0.55 },
            { 58.0, 0.54 },
            { 59.0, 0.53 },
            { 60.0, 0.52 },
            { 61.0, 0.515 },
            { 62.0, 0.510 },
            { 63.0, 0.505 },
            { 64.0, 0.500 },
            { 65.0, 0.495 },
            { 66.0, 0.490 },
            { 67.0, 0.485 },
            { 68.0, 0.480 },
            { 69.0, 0.475 },
            { 70.0, 0.470 },
            { 71.0, 0.465 },
            { 72.0, 0.460 },
            { 73.0, 0.455 },
            { 74.0, 0.450 },
            { 75.0, 0.445 },
            { 76.0, 0.440 },
            { 77.0, 0.435 },
            { 78.0, 0.430 },
            { 79.0, 0.425 },
            { 80.0, 0.420 }
        };

        return InterpolateDistanceFromVoltage(voltage, table, sizeof(table) / sizeof(table[0]));
    }
    
    double ConvertShortVoltageToDistance(double voltage)
    {
        static const SamplePoint table[] =
        {
            {  2.0, 2.20 },
            {  3.0, 1.95 },
            {  4.0, 1.72 },
            {  5.0, 1.52 },
            {  6.0, 1.36 },
            {  7.0, 1.22 },
            {  8.0, 1.10 },
            {  9.0, 1.00 },
            { 10.0, 0.91 },
            { 11.0, 0.83 },
            { 12.0, 0.76 },
            { 13.0, 0.69 },
            { 14.0, 0.63 },
            { 15.0, 0.58 }
        };

        return InterpolateDistanceFromVoltage(voltage, table, sizeof(table) / sizeof(table[0]));
    }
}