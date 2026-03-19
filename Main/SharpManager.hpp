#pragma once

#include <Arduino.h>

struct ForwardDistanceResult
{
    float shortCm;
    float longCm;
    float fusedCm;
    bool shortValid;
    bool longValid;
    bool tooClose;
    bool outOfRange;
};

namespace SharpManager
{
    void init();
    ForwardDistanceResult readForward();
    float filterForwardDistance(float rawCm);
    void printForwardResult(const ForwardDistanceResult& r, float filteredCm);
}