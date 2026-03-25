#include "Defines.hpp"
#include "Move.hpp"
#include "Light.hpp"
#include "SharpManager.hpp"
#include "CloseIR.hpp"
#include "MedianCalculator.hpp"

#include <Arduino.h>

// Color Sensors
Light FR(COL1, false), FL(COL2, false), BR(COL3, false), BL(COL4, false);
Light* lights[] = { &FR, &FL, &BR, &BL };

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

    pinMode(IR_CLOSE_A, INPUT);
    pinMode(IR_M, INPUT);
    pinMode(IR_SML, INPUT);
    pinMode(IR_L, INPUT);
    pinMode(IR_R, INPUT);

    SharpManager::Init();
    CloseIR::Init();
}

void loop()
{
    SharpManager::Update();
    CloseIR::Update();

    MedianCalculator::Update(SharpManager::AdcToVoltage(SharpManager::GetShortRawAdc()), 
                            SharpManager::AdcToVoltage(SharpManager::GetLongRawAdc()));

    delay(5);
}