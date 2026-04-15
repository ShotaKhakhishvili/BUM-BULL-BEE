#ifndef MYSRC_MEDIAN_CALCULATOR_H
#define MYSRC_MEDIAN_CALCULATOR_H

#include <stdint.h>

#define MEDIAN_CALCULATOR_SAMPLE_SIZE 101

typedef struct
{
    double samples_short[MEDIAN_CALCULATOR_SAMPLE_SIZE];
    double samples_long[MEDIAN_CALCULATOR_SAMPLE_SIZE];
    int current_sample_index;
} MedianCalculator;

void MedianCalculator_Init(MedianCalculator *self);
void MedianCalculator_Update(MedianCalculator *self, double new_short_value, double new_long_value);

double MedianCalculator_ComputeMedian(const double *values, int count);

#endif /* MYSRC_MEDIAN_CALCULATOR_H */
