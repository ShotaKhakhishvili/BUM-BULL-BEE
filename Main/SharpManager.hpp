#pragma once

enum class SharpMode
{
    LONG,
    SHORT
};

struct SamplePoint
{
    double distanceCm;
    double voltage;
};

namespace SharpManager
{
    void Init();
    void Update();

    double GetSelectedDistance();
    double GetLongDistance();
    double GetShortDistance();

    double GetRawLongDistance();
    double GetRawShortDistance();

    int GetLongRawAdc();
    int GetShortRawAdc();

    SharpMode GetMode();

    int GetLongAnomalyScore();
    int GetShortAnomalyScore();

    double ConvertLongVoltageToDistance(double voltage);
    double ConvertShortVoltageToDistance(double voltage);

    double AdcToVoltage(int adc);
}