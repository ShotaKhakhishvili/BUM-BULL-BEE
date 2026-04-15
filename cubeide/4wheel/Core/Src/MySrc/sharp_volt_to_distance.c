#include "MySrc/sharp_volt_to_distance.h"

static double SharpConverter_InterpolateDistanceFromVoltage(double voltage, const SharpSamplePoint *table, int count)
{
    int i;

    if ((table == 0) || (count < 2))
    {
        return -1.0;
    }

    if (voltage >= table[0].voltage)
    {
        return table[0].distance_cm;
    }

    if (voltage <= table[count - 1].voltage)
    {
        return table[count - 1].distance_cm;
    }

    for (i = 0; i < (count - 1); ++i)
    {
        double v_near = table[i].voltage;
        double v_far = table[i + 1].voltage;

        if ((voltage <= v_near) && (voltage >= v_far))
        {
            double d_near = table[i].distance_cm;
            double d_far = table[i + 1].distance_cm;
            double t = (voltage - v_near) / (v_far - v_near);
            return d_near + (d_far - d_near) * t;
        }
    }

    return table[count - 1].distance_cm;
}

double SharpConverter_ConvertLongVoltageToDistance(double voltage)
{
    static const SharpSamplePoint table[] = {
        {10.0, 2.53}, {11.0, 2.38}, {12.0, 2.22}, {13.0, 2.03}, {14.0, 1.93},
        {15.0, 1.79}, {16.0, 1.65}, {17.0, 1.55}, {18.0, 1.49}, {19.0, 1.41},
        {20.0, 1.35}, {21.0, 1.29}, {22.0, 1.23}, {23.0, 1.17}, {24.0, 1.13},
        {25.0, 1.05}, {26.0, 1.03}, {27.0, 0.99}, {28.0, 0.95}, {29.0, 0.93},
        {30.0, 0.91}, {31.0, 0.87}, {32.0, 0.85}, {33.0, 0.83}, {34.0, 0.80},
        {35.0, 0.78}, {36.0, 0.76}, {37.0, 0.72}, {38.0, 0.70}, {39.0, 0.68},
        {40.0, 0.66}, {41.0, 0.64}, {42.0, 0.63}, {43.0, 0.62}, {44.0, 0.0},
        {45.0, 0.67}, {46.0, 0.66}, {47.0, 0.65}, {48.0, 0.64}, {49.0, 0.63},
        {50.0, 0.62}, {51.0, 0.61}, {52.0, 0.60}, {53.0, 0.59}, {54.0, 0.58},
        {55.0, 0.57}, {56.0, 0.56}, {57.0, 0.55}, {58.0, 0.54}, {59.0, 0.53},
        {60.0, 0.52}, {61.0, 0.515}, {62.0, 0.510}, {63.0, 0.505}, {64.0, 0.500},
        {65.0, 0.495}, {66.0, 0.490}, {67.0, 0.0}, {68.0, 0.50}, {69.0, 0.48},
        {70.0, 0.46}, {71.0, 0.44}, {72.0, 0.44}, {73.0, 0.44}, {74.0, 0.44},
        {75.0, 0.42}, {76.0, 0.42}, {77.0, 0.42}, {78.0, 0.41}, {79.0, 0.40},
        {80.0, 0.40}
    };

    return SharpConverter_InterpolateDistanceFromVoltage(
        voltage,
        table,
        (int)(sizeof(table) / sizeof(table[0])));
}

double SharpConverter_ConvertShortVoltageToDistance(double voltage)
{
    static const SharpSamplePoint table[] = {
        {2.0, 2.32}, {3.0, 1.85}, {4.0, 1.48}, {5.0, 1.17}, {6.0, 1.03},
        {7.0, 0.92}, {8.0, 0.82}, {9.0, 0.69}, {10.0, 0.63}, {11.0, 0.57},
        {12.0, 0.51}, {13.0, 0.45}, {14.0, 0.43}, {15.0, 0.41}
    };

    return SharpConverter_InterpolateDistanceFromVoltage(
        voltage,
        table,
        (int)(sizeof(table) / sizeof(table[0])));
}
