#include "Defines.hpp"
#include "Distance.hpp"
#include "SharpMedian.hpp"
#include "Range.hpp"

#include "Move.hpp"
#include "Behavior.hpp"

Range range;
Infrared infraL, infraR;

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
    pinMode(INFRARED_L, INPUT);
    pinMode(INFRARED_M, INPUT);
    pinMode(INFRARED_R, INPUT);

    range.Init();
    infraL.Init(INFRARED_L);
    infraR.Init(INFRARED_R);
}

void loop()
{
    double front = range.Distance();

    Behavior::Update(front);

    Behavior::DebugPrint();
    range.DebugPrint();
}
