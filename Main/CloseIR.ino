#include "CloseIR.hpp"
#include "Defines.hpp"

#include <Arduino.h>

namespace CloseIR
{
    static constexpr int detectThreshold = 500; // this should be tuned
    
    static constexpr unsigned long updateDelayMs = 5;

    static constexpr double alpha = 0.4;

    static unsigned long lastRead = 0;

    static int rawAdc = 0;
    static double filteredAdc = 0.0;
    static bool seesObject = false;

    void Init()
    {
        lastRead = millis() - updateDelayMs - 1;
        rawAdc = analogRead(IR_CLOSE_A);
        filteredAdc = rawAdc;
        seesObject = (filteredAdc >= detectThreshold);
    }

    void Update()
    {
        const unsigned long now = millis();

        if (now - lastRead < updateDelayMs)
            return;

        rawAdc = analogRead(IR_CLOSE_A);
        filteredAdc = filteredAdc * (1.0 - alpha) + rawAdc * alpha;
        seesObject = (filteredAdc >= detectThreshold);

        lastRead = now;
    }

    int GetRawAdc()
    {
        return rawAdc;
    }

    bool SeesObject()
    {
        return seesObject;
    }
}