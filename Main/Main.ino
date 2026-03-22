#include "Defines.hpp"
#include <Arduino.h>

static constexpr int SAMPLE_COUNT = 101;

static int longSamples[SAMPLE_COUNT];
static int shortSamples[SAMPLE_COUNT];
static int sampleIndex = 0;

double AdcToVoltage(int raw)
{
    return (static_cast<double>(raw) * 5.0) / 1023.0;
}

int ComputeMedian(const int* arr, int count)
{
    static int sorted[SAMPLE_COUNT];

    for (int i = 0; i < count; ++i)
        sorted[i] = arr[i];

    for (int i = 1; i < count; ++i)
    {
        int key = sorted[i];
        int j = i - 1;

        while (j >= 0 && sorted[j] > key)
        {
            sorted[j + 1] = sorted[j];
            --j;
        }

        sorted[j + 1] = key;
    }

    return sorted[count / 2];
}

void PrintMedian()
{
    const int medianLong  = ComputeMedian(longSamples, SAMPLE_COUNT);
    const int medianShort = ComputeMedian(shortSamples, SAMPLE_COUNT);

    const double longV  = AdcToVoltage(medianLong);
    const double shortV = AdcToVoltage(medianShort);

    Serial.print("MED(");
    Serial.print(SAMPLE_COUNT);
    Serial.print(") ");

    Serial.print("L_RAW:");
    Serial.print(medianLong);
    Serial.print(" L_V:");
    Serial.print(longV, 3);

    Serial.print(" || ");

    Serial.print("S_RAW:");
    Serial.print(medianShort);
    Serial.print(" S_V:");
    Serial.println(shortV, 3);
}

void setup()
{
    Serial.begin(115200);

    pinMode(LED, OUTPUT);
    pinMode(RESET, INPUT);

    pinMode(IR_M, INPUT);
    pinMode(IR_SML, INPUT);

    sampleIndex = 0;
}

void loop()
{
    const int rawLong  = analogRead(IR_M);
    const int rawShort = analogRead(IR_SML);

    if (sampleIndex < SAMPLE_COUNT)
    {
        longSamples[sampleIndex]  = rawLong;
        shortSamples[sampleIndex] = rawShort;
        ++sampleIndex;
    }

    if (sampleIndex >= SAMPLE_COUNT)
    {
        PrintMedian();
        sampleIndex = 0;
    }

    delay(5);
}