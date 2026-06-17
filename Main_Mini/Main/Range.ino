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

    // ToF drives the choice: trust it while it reads within range and closer
    // than the limit; otherwise (far or dropped out) fall back to the Sharp.
    usingTof = (tofCm > 0 && tofCm < TOF_RANGE_LIMIT);

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

    sharp.DebugPrint();
    tof.DebugPrint();

    Serial.print("Range | Source: ");
    Serial.print(usingTof ? "ToF  " : "Sharp");
    Serial.print(" | Distance: ");
    Serial.println(distance, kPrintDigitsAfterDecimal);
}
