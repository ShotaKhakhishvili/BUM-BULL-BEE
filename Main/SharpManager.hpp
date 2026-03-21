#pragma once

#include <Arduino.h>

namespace SharpManager
{
    enum class SharpMode
    {
        SHORT, LONG
    }

    void SuggestUpdate();
    void RefreshMode();
    void Init();
}