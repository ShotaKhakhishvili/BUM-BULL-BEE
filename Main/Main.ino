#include "Defines.hpp"
#include "Move.hpp"
#include "Light.hpp"
#include "SharpManager.hpp"
#include "CloseIR.hpp"

#include <Arduino.h>

// Color Sensors
Light FR(COL1, false), FL(COL2, false), BR(COL3, false), BL(COL4, false);
Light* lights[] = { &FR, &FL, &BR, &BL };

// Tuning
static constexpr float STOP_DISTANCE_CM = 5.0f;
static constexpr int SEARCH_SPEED = 100;

static constexpr unsigned long PRINT_INTERVAL_MS = 100;

void setup()
{
    Serial.begin(115200);

    pinMode(LED, OUTPUT);

    pinMode(COL1, INPUT);
    pinMode(COL2, INPUT);
    pinMode(COL3, INPUT);
    pinMode(COL4, INPUT);

    pinMode(WH_LF, OUTPUT);
    pinMode(WH_LB, OUTPUT);
    pinMode(WH_RF, OUTPUT);
    pinMode(WH_RB, OUTPUT);

    pinMode(RESET, INPUT);

    SharpManager::Init();
    CloseIR::Init();
}

void loop()
{
    SharpManager::Update();

    static unsigned long lastPrint = 0;

    if (millis() - lastPrint >= PRINT_INTERVAL_MS)
    {
        lastPrint = millis();

        const int rawLong  = SharpManager::GetLongRawAdc();
        const int rawShort = SharpManager::GetShortRawAdc();

        const double longVoltage  = SharpManager::AdcToVoltage(rawLong);
        const double shortVoltage = SharpManager::AdcToVoltage(rawShort);

        Serial.print("LONG_ADC: ");
        Serial.print(rawLong);
        Serial.print(" | LONG_V: ");
        Serial.print(longVoltage, 3);

        Serial.print(" || SHORT_ADC: ");
        Serial.print(rawShort);
        Serial.print(" | SHORT_V: ");
        Serial.print(shortVoltage, 3);

        Serial.println();
    }
}














