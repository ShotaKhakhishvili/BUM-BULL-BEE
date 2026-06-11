#include "SharpMedian.hpp"

namespace SharpMedian
{
    double samples[SAMPLE_SIZE] = {};
    int head = 0;    // next write position in the circular buffer
    int filled = 0;  // number of valid samples, capped at SAMPLE_SIZE

    void Update(double voltage)
    {
        samples[head] = voltage;
        head = (head + 1) % SAMPLE_SIZE;
        if (filled < SAMPLE_SIZE)
            ++filled;
    }

    int Count()
    {
        return filled;
    }

    void Reset()
    {
        head = 0;
        filled = 0;
    }

    double GetMedianVoltage()
    {
        if (filled == 0)
            return 0.0;

        double sorted[SAMPLE_SIZE];
        for (int i = 0; i < filled; ++i)
            sorted[i] = samples[i];

        for (int i = 1; i < filled; ++i)
        {
            double key = sorted[i];
            int j = i - 1;

            while (j >= 0 && sorted[j] > key)
            {
                sorted[j + 1] = sorted[j];
                --j;
            }
            sorted[j + 1] = key;
        }

        if ((filled % 2) == 1)
            return sorted[filled / 2];

        return (sorted[(filled / 2) - 1] + sorted[filled / 2]) / 2.0;
    }
}
