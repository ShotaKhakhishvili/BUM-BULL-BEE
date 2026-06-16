#pragma once

#include "Sharp.hpp"
#include "Tof.hpp"

class Range
{
public:
    void Init();
    double Distance();
    bool UsingTof();           
    void DebugPrint();

private:
    Sharp sharp;
    Tof tof;

    bool usingTof;
    double distance;
};
