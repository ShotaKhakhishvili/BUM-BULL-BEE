#include "Defines.hpp"
#include "Tof.hpp"

#include <Wire.h>

void Tof::Init()
{
    Wire.begin();

    sensor.setTimeout(500);
    ready = sensor.init();

    if (ready)
        sensor.startContinuous();

    lastUpdateTime = -TOF_UPDATE_INTERVAL;

    Distance();
}

double Tof::Distance()
{
    if (!ready)
    {
        this->rawMillimeters = -1;
        this->distance = -1.0;
        return distance;
    }

    const unsigned long now = millis();

    if (now >= TOF_UPDATE_INTERVAL + lastUpdateTime)
    {
        this->rawMillimeters = sensor.readRangeContinuousMillimeters();
        this->distance = rawMillimeters / 10.0; // mm -> cm

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

    Serial.print("Raw mm: ");
    Serial.print(rawMillimeters);
    Serial.print(" | Distance: ");
    Serial.print(distance, kPrintDigitsAfterDecimal);
    Serial.print(" | ");
    Serial.println(sensor.timeoutOccurred() ? "TIMEOUT" : "OK");
}
