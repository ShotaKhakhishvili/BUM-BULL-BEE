#include "SharpManager.hpp"
#include "SharpSuggest.hpp"
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

    static constexpr double alphaLong  = 0.3;
    static constexpr double alphaShort = 0.3;

    static unsigned long lastShortRead = 0;
    static unsigned long lastLongRead  = 0;

    static int rawLongAdc  = 0;
    static int rawShortAdc = 0;

    static double distanceLongCm = 1000.0;
    static double distanceShortCm = 1000.0;
    static double selectedDistanceCm = 1000.0;

    static SharpMode currentMode = SharpMode::LONG;

    static void RefreshSelectedDistance()
    {
        if (currentMode == SharpMode::LONG)
            selectedDistanceCm = distanceLongCm;
        else
            selectedDistanceCm = distanceShortCm;
    }

    static void RefreshModeByDistance()
    {
        if (currentMode == SharpMode::LONG)
        {
            if (selectedDistanceCm < switchToShort)
                currentMode = SharpMode::SHORT;
        }
        else
        {
            if (selectedDistanceCm > switchToLong)
                currentMode = SharpMode::LONG;
        }
    }

    static void UpdateLongSensor(const unsigned long now)
    {
        if (now - lastLongRead >= delayLongMs)
        {
            rawLongAdc = analogRead(IR_M);

            const double newDistance = sharpForwardLong.getDistance();
            distanceLongCm = distanceLongCm * (1.0 - alphaLong) + newDistance * alphaLong;

            lastLongRead = now;
        }
    }

    static void UpdateShortSensor(const unsigned long now)
    {
        if (now - lastShortRead >= delayShortMs)
        {
            rawShortAdc = analogRead(IR_SML);

            const double newDistance = sharpForwardShort.getDistance();
            distanceShortCm = distanceShortCm * (1.0 - alphaShort) + newDistance * alphaShort;

            lastShortRead = now;
        }
    }

    void Init()
    {
        lastShortRead = millis() - delayShortMs - 1;
        lastLongRead  = millis() - delayLongMs - 1;

        rawLongAdc  = analogRead(IR_M);
        rawShortAdc = analogRead(IR_SML);

        distanceLongCm  = sharpForwardLong.getDistance();
        distanceShortCm = sharpForwardShort.getDistance();

        currentMode = SharpMode::LONG;
        RefreshSelectedDistance();

        SharpSuggest::Init();
    }

    void Update()
    {
        const unsigned long now = millis();

        UpdateLongSensor(now);
        UpdateShortSensor(now);

        SharpSuggest::Update(currentMode, rawLongAdc, rawShortAdc);

        RefreshSelectedDistance();
        RefreshModeByDistance();
        RefreshSelectedDistance();

        if (currentMode == SharpMode::LONG)
        {
            if (SharpSuggest::ShouldSuggestShortFromLong())
                currentMode = SharpMode::SHORT;
        }
        else
        {
            if (SharpSuggest::ShouldSuggestLongFromShort())
                currentMode = SharpMode::LONG;
        }

        RefreshSelectedDistance();
    }

    double GetSelectedDistance()
    {
        return selectedDistanceCm;
    }

    double GetLongDistance()
    {
        return distanceLongCm;
    }

    double GetShortDistance()
    {
        return distanceShortCm;
    }

    int GetLongRawAdc()
    {
        return rawLongAdc;
    }

    int GetShortRawAdc()
    {
        return rawShortAdc;
    }

    SharpMode GetMode()
    {
        return currentMode;
    }

    int GetLongAnomalyScore()
    {
        return SharpSuggest::GetLongAnomalyScore();
    }

    int GetShortAnomalyScore()
    {
        return SharpSuggest::GetShortAnomalyScore();
    }
}