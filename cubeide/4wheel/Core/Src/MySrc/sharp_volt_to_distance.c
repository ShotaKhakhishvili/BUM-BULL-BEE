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
        { 7.0, 3.035}, { 8.0, 2.726}, { 9.0, 2.526}, {10.0, 2.416},
        {11.0, 2.223}, {12.0, 2.052}, {13.0, 1.910}, {14.0, 1.771},
        {15.0, 1.681}, {16.0, 1.587}, {17.0, 1.506}, {18.0, 1.426},
        {19.0, 1.365}, {20.0, 1.281}, {21.0, 1.223}, {22.0, 1.161},
        {23.0, 1.129}, {24.0, 1.084}, {25.0, 1.029}, {26.0, 1.006},
        {27.0, 0.974}, {28.0, 0.939}, {29.0, 0.923}, {30.0, 0.884},
        {31.0, 0.845}, {32.0, 0.829}, {33.0, 0.806}, {34.0, 0.787},
        {35.0, 0.765}, {36.0, 0.745}, {37.0, 0.723}, {38.0, 0.703},
        {39.0, 0.684}, {40.0, 0.665}, {41.0, 0.645}, {42.0, 0.639},
        {43.0, 0.623}, {44.0, 0.616}, {45.0, 0.603}, {46.0, 0.597},
        {47.0, 0.584}, {48.0, 0.565}, {49.0, 0.558}, {50.0, 0.545},
        {51.0, 0.523}, {52.0, 0.519}, {53.0, 0.510}, {54.0, 0.500},
        {55.0, 0.497}, {56.0, 0.495}, {57.0, 0.484}, {58.0, 0.468},
        {59.0, 0.457}, {60.0, 0.442}, {70.0, 0.397}, {80.0, 0.345},
        {90.0, 0.290}
    };

    return SharpConverter_InterpolateDistanceFromVoltage(
        voltage,
        table,
        (int)(sizeof(table) / sizeof(table[0])));
}

double SharpConverter_ConvertShortVoltageToDistance(double voltage)
{
    static const SharpSamplePoint table[] = {
        { 1.0, 2.519}, { 2.0, 1.977}, { 3.0, 1.406}, { 4.0, 1.229},
        { 5.0, 1.006}, { 6.0, 0.890}, { 7.0, 0.777}, { 8.0, 0.694},
        { 9.0, 0.632}, {10.0, 0.571}, {11.0, 0.516}, {12.0, 0.477},
        {13.0, 0.455}, {14.0, 0.413}, {15.0, 0.394}, {16.0, 0.374},
        {17.0, 0.332}, {18.0, 0.332}, {19.0, 0.313}, {20.0, 0.294}
    };

    return SharpConverter_InterpolateDistanceFromVoltage(
        voltage,
        table,
        (int)(sizeof(table) / sizeof(table[0])));
}