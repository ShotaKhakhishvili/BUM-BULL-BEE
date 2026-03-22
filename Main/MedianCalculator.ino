#include "MedianCalculator.hpp"
#include <Arduino.h>

namespace MedianCalculator
{
    int ComputeMedian(int* values, int count);
    void PrintMedians();

    int CURRENT_SAMPLE_INDEX = 0;

    int SAMPLES_SHORT[SAMPLE_SIZE] = {};
    int SAMPLES_LONG[SAMPLE_SIZE] = {};
    
    void Update(const int newShortValue, const int newLongValue)
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

    int ComputeMedian(int* values, int count)
    {
        for (int i = 1; i < count; ++i)
        {
            int key = values[i];
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
        int shortMedian = ComputeMedian(SAMPLES_SHORT, SAMPLE_SIZE);
        int longMedian = ComputeMedian(SAMPLES_LONG, SAMPLE_SIZE);

        Serial.print("Short Median: ");
        Serial.print(shortMedian);

        Serial.print(" | Long Median: ");
        Serial.println(longMedian);
        CURRENT_SAMPLE_INDEX = 0;
    }
}