#include "Defines.hpp"
#include "Sharp.hpp"

Distance distance;

void setup()
{
    Serial.begin(115200);

    pinMode(LED, OUTPUT);

    pinMode(WH_LF, OUTPUT);
    pinMode(WH_LB, OUTPUT);
    pinMode(WH_RF, OUTPUT);
    pinMode(WH_RB, OUTPUT);

    pinMode(RESET, INPUT);

    pinMode(IR, INPUT);

    distance.Init();
}

void loop()
{
    distance.DebugPrint(0b11);

    delay(50);
}