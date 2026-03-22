#pragma once

// #include "SharpMode.hpp"

enum class SharpMode
{
    LONG,
    SHORT
};

namespace SharpSuggest
{
    void Init();
    void Reset();

    void Update(SharpMode currentMode, int rawLongAdc, int rawShortAdc);

    bool ShouldSuggestShortFromLong();
    bool ShouldSuggestLongFromShort();

    int GetLongAnomalyScore();
    int GetShortAnomalyScore();
}