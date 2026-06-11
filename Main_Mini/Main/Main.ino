#include "Defines.hpp"
#include "Distance.hpp"
#include "SharpMedian.hpp"

#include "Move.hpp"

Distance distance;
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

    distance.Init();
    infraL.Init(INFRARED_L);
    infraR.Init(INFRARED_R);
}

void loop()
{
    //distance.DebugPrint(0b11);
    //infraL.DebugPrint();
    //infraR.DebugPrint();

    distance.GetSharp();

    Serial.print("Sharp | samples: ");
    Serial.print(SharpMedian::Count());
    Serial.print("/");
    Serial.print(SharpMedian::SAMPLE_SIZE);
    Serial.print(" | Median Voltage: ");
    Serial.println(SharpMedian::GetMedianVoltage(), kPrintDigitsAfterDecimal);

    Serial.println();
    delay(500);
}
