#include "MySrc/median_calculator.h"

#include "MySrc/defines.h"
#include "MySrc/platform_adapter.h"

#include <string.h>

void MedianCalculator_Init(MedianCalculator *self)
{
    if (self == 0)
    {
        return;
    }

    memset(self->samples_short, 0, sizeof(self->samples_short));
    memset(self->samples_long, 0, sizeof(self->samples_long));
    self->current_sample_index = 0;
}

double MedianCalculator_ComputeMedian(const double *values, int count)
{
    int i;
    int j;
    double key;
    double sorted[MEDIAN_CALCULATOR_SAMPLE_SIZE];

    if ((values == 0) || (count <= 0))
    {
        return 0.0;
    }

    if (count > MEDIAN_CALCULATOR_SAMPLE_SIZE)
    {
        count = MEDIAN_CALCULATOR_SAMPLE_SIZE;
    }

    for (i = 0; i < count; ++i)
    {
        sorted[i] = values[i];
    }

    for (i = 1; i < count; ++i)
    {
        key = sorted[i];
        j = i - 1;

        while ((j >= 0) && (sorted[j] > key))
        {
            sorted[j + 1] = sorted[j];
            --j;
        }

        sorted[j + 1] = key;
    }

    if ((count % 2) == 1)
    {
        return sorted[count / 2];
    }

    return (sorted[(count / 2) - 1] + sorted[count / 2]) / 2.0;
}

void MedianCalculator_Update(MedianCalculator *self, double new_short_value, double new_long_value)
{
    double short_median;
    double long_median;

    if (self == 0)
    {
        return;
    }

    if (self->current_sample_index < MEDIAN_CALCULATOR_SAMPLE_SIZE)
    {
        self->samples_short[self->current_sample_index] = new_short_value;
        self->samples_long[self->current_sample_index] = new_long_value;
        ++self->current_sample_index;
    }

    if (self->current_sample_index < MEDIAN_CALCULATOR_SAMPLE_SIZE)
    {
        return;
    }

    short_median = MedianCalculator_ComputeMedian(self->samples_short, MEDIAN_CALCULATOR_SAMPLE_SIZE);
    long_median = MedianCalculator_ComputeMedian(self->samples_long, MEDIAN_CALCULATOR_SAMPLE_SIZE);

    Platform_DebugPrintMedians(short_median, long_median, (uint8_t)BBB_PRINT_DIGITS_AFTER_DECIMAL);
    self->current_sample_index = 0;
}
