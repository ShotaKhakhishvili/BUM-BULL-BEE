#include "Defines.hpp"
#include "Move.hpp"
#include "Light.hpp"
#include "SharpManager.hpp"

#include <Arduino.h>

// Color Sensors
Light FR(COL1, false), FL(COL2, false), BR(COL3, false), BL(COL4, false);
Light* lights[] = { &FR, &FL, &BR, &BL };

// Moves
unsigned long lastPrint = 0;

// Tuning
static constexpr float STOP_DISTANCE_CM = 15.0f;
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

    SharpManager::init();

    lastPrint = millis();
}

void loop()
{
    SharpManager::Update();

}














