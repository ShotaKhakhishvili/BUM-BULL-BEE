#include "Defines.hpp"
#include <Arduino.h>

static constexpr int LONG_PIN  = A0;
static constexpr int SHORT_PIN = A1;

static constexpr int LED_PIN   = 13;

static constexpr int SAMPLE_COUNT = 21; // luwi dasvi aq


static int longSamples[SAMPLE_COUNT];
static int shortSamples[SAMPLE_COUNT];

static int sampleIndex = 0;


double AdcToVoltage(int raw)
{
    return raw * (5.0 / 1023.0); 
}

int ComputeMedian(int* arr, int count)
{
    static int sorted[SAMPLE_COUNT];

    // copy
    for (int i = 0; i < count; ++i)
        sorted[i] = arr[i];

    // insertion sort
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

    return sorted[count / 2]; // odd count assumed
}

// ------------------- PRINT -------------------

void PrintMedian()
{
    int medianLong  = ComputeMedian(longSamples, SAMPLE_COUNT);
    int medianShort = ComputeMedian(shortSamples, SAMPLE_COUNT);

    double longV  = AdcToVoltage(medianLong);
    double shortV = AdcToVoltage(medianShort);

    Serial.print("MED(");
    Serial.print(SAMPLE_COUNT);
    Serial.print(") ");

    Serial.print("L_ADC:");
    Serial.print(medianLong);
    Serial.print(" L_V:");
    Serial.print(longV, 3);

    Serial.print(" || ");

    Serial.print("S_ADC:");
    Serial.print(medianShort);
    Serial.print(" S_V:");
    Serial.println(shortV, 3);
}

void setup()
{
    Serial.begin(115200);

    pinMode(LED, OUTPUT);
    pinMode(RESET, INPUT);

    pinMode(LONG_PIN, INPUT);
    pinMode(SHORT_PIN, INPUT);
    pinMode(LED_PIN, OUTPUT);

    pinMode(COL1, INPUT);
    pinMode(COL2, INPUT);
    pinMode(COL3, INPUT);
    pinMode(COL4, INPUT);

    pinMode(WH_LF, OUTPUT);
    pinMode(WH_LB, OUTPUT);
    pinMode(WH_RF, OUTPUT);
    pinMode(WH_RB, OUTPUT);

    sampleIndex = 0;
}

void loop()
{
    // read sensors
    int rawLong  = analogRead(LONG_PIN);
    int rawShort = analogRead(SHORT_PIN);

    if (sampleIndex < SAMPLE_COUNT)
    {
        longSamples[sampleIndex]  = rawLong;
        shortSamples[sampleIndex] = rawShort;
        sampleIndex++;
    }

    if (sampleIndex >= SAMPLE_COUNT)
    {
        PrintMedian();
        sampleIndex = 0;
    }

    delay(2);
}