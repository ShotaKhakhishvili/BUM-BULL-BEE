#include "Defines.hpp"
#include "Distance.hpp"
#include "SharpMedian.hpp"
#include "Tof.hpp"

#include "Move.hpp"

Distance distance;
Infrared infraL, infraR;
Tof tof;

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

    distance.Init();
    infraL.Init(INFRARED_L);
    infraR.Init(INFRARED_R);
    tof.Init();
}

void loop()
{
    distance.DebugPrint(0b11);
    infraL.DebugPrint();
    infraR.DebugPrint();
    tof.DebugPrint();

    /*

    distance.GetSharp();

    if (SharpMedian::Count() >= SharpMedian::SAMPLE_SIZE)
    {
        Serial.print("Sharp | Median Voltage: ");
        Serial.println(SharpMedian::GetMedianVoltage(), kPrintDigitsAfterDecimal);

        SharpMedian::Reset();
    }

    */

    delay(500);
}
