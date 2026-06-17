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

    lastSharp = sharpCm;
    lastTof   = tofCm;

    usingTof = (tofCm > 0 && tofCm < TOF_BLIND_CM);

    distance = usingTof ? tofCm : sharpCm;
    return distance;
}

bool Range::UsingTof()
{
    return usingTof;
}

void Range::DebugPrint()
{
    // NOTE: do NOT call Distance() here -- print the values the last real
    // (loop-time) decision used, so they reflect what drove the behavior.
    // The Sharp/ToF lines below re-sample, so comparing them to the Range
    // line reveals any loop-time vs debug-time discrepancy.
    sharp.DebugPrint();
    tof.DebugPrint();

    Serial.print("Range | sharpCm: ");
    Serial.print(lastSharp, kPrintDigitsAfterDecimal);
    Serial.print(" | tofCm: ");
    Serial.print(lastTof, kPrintDigitsAfterDecimal);
    Serial.print(" | Source: ");
    Serial.print(usingTof ? "ToF  " : "Sharp");
    Serial.print(" | Distance: ");
    Serial.println(distance, kPrintDigitsAfterDecimal);
}
