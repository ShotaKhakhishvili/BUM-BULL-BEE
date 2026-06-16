#include "Defines.hpp"
#include "SharpMedian.hpp"
#include "Range.hpp"
#include "Infrared.hpp"

#include "Move.hpp"
#include "Behavior.hpp"

Range range;
Infrared ir1, ir2;
unsigned long lastDebugPrint = 0;

void setup()
{
    Serial.begin(115200);

    pinMode(LED, OUTPUT);

    Move::Init();           // TB6612FNG motor pins

    range.Init();           // Sharp (A3) + ToF (I2C: A4/A5)
    ir1.Init(IR1);          // D7
    ir2.Init(IR2);          // D8
}

void loop()
{
    double front = range.Distance();

    Behavior::Update(front);

    if(millis() - lastDebugPrint >= 1000)
    {
        Behavior::DebugPrint();
        range.DebugPrint();
        ir1.DebugPrint();
        ir2.DebugPrint();

        lastDebugPrint = millis();
    }
}
