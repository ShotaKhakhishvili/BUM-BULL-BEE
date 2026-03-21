#include "SharpManager.hpp"
#include "Defines.hpp"

#include <Arduino.h>
#include <SharpIR.h>

namespace SharpManager
{
    SharpIR sharpForwardShort(IR_SML, MODEL_SHORT);
    SharpIR sharpForwardLong(IR_M, MODEL_LONG);

    static constexpr double switchToLong  = 13.5;
    static constexpr double switchToShort = 11.5;

    static constexpr unsigned long delayShortMs = 16;
    static constexpr unsigned long delayLongMs  = 40;

    static unsigned long lastShortRead = 0;
    static unsigned long lastLongRead  = 0;

    static double distanceM = 1000.0;
    static SharpMode currentMode = SharpMode::LONG;

    void RefreshMode()
    {
        if (currentMode == SharpMode::LONG)
        {
            if (distanceM < switchToShort)
                currentMode = SharpMode::SHORT;
        }
        else
        {
            if (distanceM > switchToLong)
                currentMode = SharpMode::LONG;
        }
    }

    void Update()
    {
        const unsigned long now = millis();

        if (currentMode == SharpMode::LONG)
        {
            if (now - lastLongRead >= delayLongMs)
            {
                double newDistance = sharpForwardLong.getDistance();
                distanceM = distanceM * 0.7 + newDistance * 0.3;
                lastLongRead = now;
            }
        }
        else
        {
            if (now - lastShortRead >= delayShortMs)
            {
                double newDistance = sharpForwardShort.getDistance();
                distanceM = distanceM * 0.7 + newDistance * 0.3;
                lastShortRead = now;
            }
        }

        RefreshMode();
    }

    void Init()
    {
        lastShortRead = millis() - delayShortMs - 1;
        lastLongRead  = millis() - delayLongMs - 1;
    }

    double GetDistance()
    {
        return distanceM;
    }

    SharpMode GetMode()
    {
        return currentMode;
    }
}