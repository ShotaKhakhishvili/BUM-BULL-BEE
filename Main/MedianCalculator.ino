#include "MedianCalculator.hpp"
#include <Arduino.h>

namespace MedianCalculator
{
    int ComputeMedian(int* values, int count);
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

    double ComputeMedian(double* values, int count)
    {
        for (int i = 1; i < count; ++i)
        {
            double key = values[i];
            int j = i - 1;

            while (j >= 0 && values[j] > key)
            {
                values[j + 1] = values[j];
                --j;
            }
            values[j + 1] = key;
        }

        return values[count / 2];
    }

    void PrintMedians()
    {
        double shortMedian = ComputeMedian(SAMPLES_SHORT, SAMPLE_SIZE);
        double longMedian = ComputeMedian(SAMPLES_LONG, SAMPLE_SIZE);

        Serial.print("Short Median: ");
        Serial.print(shortMedian);

        Serial.print(" | Long Median: ");
        Serial.println(longMedian);
        CURRENT_SAMPLE_INDEX = 0;
    }
}