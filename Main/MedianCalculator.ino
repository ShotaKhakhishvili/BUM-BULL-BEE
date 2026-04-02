#include "MedianCalculator.hpp"
#include "Defines.hpp"

#include <Arduino.h>

namespace MedianCalculator
{
    double ComputeMedian(const double* values, int count);
    void PrintMedians();

    int CURRENT_SAMPLE_INDEX = 0;

    double SAMPLES_SHORT[SAMPLE_SIZE] = {};
    double SAMPLES_LONG[SAMPLE_SIZE] = {};
    
    void Update(const double newShortValue, const double newLongValue)
    {
        if (CURRENT_SAMPLE_INDEX < SAMPLE_SIZE)
        {
            SAMPLES_SHORT[CURRENT_SAMPLE_INDEX] = newShortValue;
            SAMPLES_LONG[CURRENT_SAMPLE_INDEX] = newLongValue;
            ++CURRENT_SAMPLE_INDEX;
        }
        if(CURRENT_SAMPLE_INDEX >= SAMPLE_SIZE)
        {
            PrintMedians();
            CURRENT_SAMPLE_INDEX = 0;
        }
    }

    double ComputeMedian(const double* values, int count)
    {
        if (count <= 0)
            return 0.0;

        if (count > SAMPLE_SIZE)
            count = SAMPLE_SIZE;

        double sorted[SAMPLE_SIZE];
        for (int i = 0; i < count; ++i)
            sorted[i] = values[i];

        for (int i = 1; i < count; ++i)
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

        if ((count % 2) == 1)
            return sorted[count / 2];

        return (sorted[(count / 2) - 1] + sorted[count / 2]) / 2.0;
    }

void PrintMedians()
{
    double shortMedian = ComputeMedian(SAMPLES_SHORT, SAMPLE_SIZE);
    double longMedian = ComputeMedian(SAMPLES_LONG, SAMPLE_SIZE);

    Serial.print("Short Median: ");
    Serial.print(shortMedian, kPrintDigitsAfterDecimal);

    Serial.print(" | Long Median: ");
    Serial.println(longMedian, kPrintDigitsAfterDecimal);
}
}