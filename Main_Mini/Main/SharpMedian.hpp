#pragma once

namespace SharpMedian
{
    constexpr int SAMPLE_SIZE = 29;

    void Update(double voltage);
    double GetMedianVoltage();
    int Count();
    void Reset();
}
