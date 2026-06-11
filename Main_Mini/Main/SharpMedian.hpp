#pragma once

namespace SharpMedian
{
    constexpr int SAMPLE_SIZE = 101;

    void Update(double voltage);
    double GetMedianVoltage();
    int Count();
}
