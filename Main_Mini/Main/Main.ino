#include "Defines.hpp"
#include "SharpMedian.hpp"
#include "Range.hpp"

#include "Move.hpp"
#include "Behavior.hpp"
#include "RunState.hpp"

Range range;
unsigned long lastDebugPrint = 0;

void setup()
{
    Serial.begin(115200);

    pinMode(LED, OUTPUT);

    Move::Init();           // TB6612FNG motor pins

    range.Init();           // Sharp (A3) + ToF (I2C: A4/A5)

    RunState::Init();       // start (D2) / stop (D1) interrupts
}

void loop()
{
    if(!RunState::IsRunning())
    {
        Move::Walk(FORWARD, 0);
        return;
    }

    double front = range.Distance();

    Behavior::Update(front);    // SEEK (spin) / CHASE (catch)

    if(millis() - lastDebugPrint >= 1000)
    {
        Serial.println("--------------------------------------------");

        RunState::DebugPrint();
        Behavior::DebugPrint();
        Move::DebugPrint();
        range.DebugPrint();

        lastDebugPrint = millis();
    }
}
