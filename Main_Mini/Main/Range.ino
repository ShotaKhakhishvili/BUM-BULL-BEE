#include "Defines.hpp"
#include "Range.hpp"

void Range::Init()
{
    sharp.Init();
    tof.Init();

    usingTof = false;
    distance = sharp.Distance();
}

double Range::Distance()
{
    const double sharpCm = sharp.Distance();
    const double tofCm   = tof.Distance();

    if (usingTof)
    {
        if (tofCm <= 0 || tofCm >= SHARP_SWITCH_DISTANCE)
            usingTof = false;
    }
    else
    {
        if (sharpCm < TOF_SWITCH_DISTANCE && tofCm > 0 && tofCm < SHARP_SWITCH_DISTANCE)
            usingTof = true;
    }

    distance = usingTof ? tofCm : sharpCm;
    return distance;
}

bool Range::UsingTof()
{
    return usingTof;
}

void Range::DebugPrint()
{
    this->Distance();

    Serial.print("Range | Source: ");
    Serial.print(usingTof ? "ToF  " : "Sharp");
    Serial.print(" | Distance: ");
    Serial.println(distance, kPrintDigitsAfterDecimal);
}
