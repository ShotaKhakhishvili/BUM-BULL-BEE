#include "Defines.hpp"
#include "Tof.hpp"

#include <Wire.h>

void Tof::Init()
{
    Wire.begin();

    // Pololu VL53L0X connection sequence that works on this sensor.
    ready = sensor.init();
    if (ready)
    {
        sensor.setTimeout(50);
        sensor.startContinuous(20);   // new reading every 20 ms
    }

    lastUpdateTime = -TOF_UPDATE_INTERVAL;
    rawMillimeters = -1;
    timedOut = false;
    distance = -1.0;

    Distance();
}

double Tof::Distance()
{
    if (!ready)
    {
        this->rawMillimeters = -1;
        this->timedOut = false;
        this->distance = -1.0;
        return distance;
    }

    const unsigned long now = millis();

    if (now >= TOF_UPDATE_INTERVAL + lastUpdateTime)
    {
        this->rawMillimeters = sensor.readRangeContinuousMillimeters();
        this->timedOut = sensor.timeoutOccurred();

        // On timeout report invalid; otherwise mm -> cm. (Out-of-range comes
        // back as a large value, which Range treats as "far" automatically.)
        this->distance = timedOut ? -1.0 : (rawMillimeters / 10.0);

        lastUpdateTime = now;
    }

    return distance;
}

void Tof::DebugPrint()
{
    this->Distance();

    Serial.print("ToF Data : | ");

    if (!ready)
    {
        Serial.println("sensor not found (check wiring/I2C)");
        return;
    }

    if (timedOut)
    {
        Serial.println("TIMEOUT");
        return;
    }

    Serial.print("Raw mm: ");
    Serial.print(rawMillimeters);
    Serial.print(" | Distance: ");
    Serial.println(distance, kPrintDigitsAfterDecimal);
}
