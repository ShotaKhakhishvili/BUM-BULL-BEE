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
    CloseIR::Update();

    static unsigned long lastPrint = 0;

    if (millis() - lastPrint >= 100)
    {
        lastPrint = millis();

        Serial.print("Mode: ");
        Serial.print(SharpManager::GetMode() == SharpMode::LONG ? "LONG" : "SHORT");

        Serial.print(" | Dist: ");
        Serial.print(SharpManager::GetSelectedDistance());

        Serial.print(" | Long: ");
        Serial.print(SharpManager::GetLongDistance());

        Serial.print(" | Short: ");
        Serial.print(SharpManager::GetShortDistance());

        Serial.print(" | RawLong: ");
        Serial.print(SharpManager::GetLongRawAdc());

        Serial.print(" | RawShort: ");
        Serial.print(SharpManager::GetShortRawAdc());

        Serial.print(" | IR_Close: ");
        Serial.print(CloseIR::GetRawAdc());

        Serial.print(" | Sees: ");
        Serial.print(CloseIR::SeesObject() ? "YES" : "NO");

        Serial.print(" | LongScore: ");
        Serial.print(SharpManager::GetLongAnomalyScore());

        Serial.print(" | ShortScore: ");
        Serial.print(SharpManager::GetShortAnomalyScore());

        Serial.println();
    }
}














