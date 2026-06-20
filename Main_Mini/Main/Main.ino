#include "Defines.hpp"
#include "SharpMedian.hpp"
#include "Range.hpp"
#include "Infrared.hpp"

#include "Move.hpp"
#include "Behavior.hpp"
#include "RunState.hpp"

Range range;
Infrared ir1, ir2;
unsigned long lastDebugPrint = 0;

void setup()
{
    Serial.begin(115200);

    pinMode(LED, OUTPUT);

    Move::Init();           // TB6612FNG motor pins

    range.Init();           // Sharp (A3) + ToF (I2C: A4/A5)
    ir1.Init(IR_READS_ANALOG ? IR1_ANALOG : IR1, IR_READS_ANALOG);
    ir2.Init(IR_READS_ANALOG ? IR2_ANALOG : IR2, IR_READS_ANALOG);

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

    Behavior::Update(front);

    if(millis() - lastDebugPrint >= 1000)
    {
        Serial.println("--------------------------------------------");

        RunState::DebugPrint();
        Behavior::DebugPrint();
        Move::DebugPrint();
        range.DebugPrint();
        ir1.DebugPrint();
        ir2.DebugPrint();

        lastDebugPrint = millis();
    }
}
