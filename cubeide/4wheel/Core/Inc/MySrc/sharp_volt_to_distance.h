#ifndef MYSRC_SHARP_VOLT_TO_DISTANCE_H
#define MYSRC_SHARP_VOLT_TO_DISTANCE_H

typedef struct
{
    double distance_cm;
    double voltage;
} SharpSamplePoint;

double SharpConverter_ConvertLongVoltageToDistance(double voltage);
double SharpConverter_ConvertShortVoltageToDistance(double voltage);

#endif /* MYSRC_SHARP_VOLT_TO_DISTANCE_H */
