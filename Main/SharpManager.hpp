#pragma once

#include "SharpSuggest.hpp"

namespace SharpManager
{
    void Init();
    void Update();

    double GetSelectedDistance();
    double GetLongDistance();
    double GetShortDistance();

    int GetLongRawAdc();
    int GetShortRawAdc();

    SharpMode GetMode();

    int GetLongAnomalyScore();
    int GetShortAnomalyScore();
}