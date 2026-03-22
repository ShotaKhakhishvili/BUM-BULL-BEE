#pragma once

enum class SharpMode;

namespace SharpSuggest
{
    void Init();
    void Reset();

    void Update(int rawLongAdc, int rawShortAdc);

    bool ShouldSuggestShortFromLong();
    bool ShouldSuggestLongFromShort();

    int GetLongAnomalyScore();
    int GetShortAnomalyScore();
}