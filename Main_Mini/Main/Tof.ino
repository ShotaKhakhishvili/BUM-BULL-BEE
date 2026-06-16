#include "Defines.hpp"
#include "Tof.hpp"

#include <Wire.h>

void Tof::ScanI2C()
{
    Serial.println("I2C scan...");

    int found = 0;
    for (byte addr = 1; addr < 127; ++addr)
    {
        Wire.beginTransmission(addr);
        if (Wire.endTransmission() == 0)
        {
            Serial.print("  device at 0x");
            Serial.println(addr, HEX);
            ++found;
        }
    }

    if (found == 0)
        Serial.println("  no I2C devices found (check SDA/SCL/VCC/GND wiring)");
}

void Tof::Init()
{
    Wire.begin();
    ScanI2C();   // report what's on the bus before boot

    // Adafruit begin() boots the sensor over I2C (default address 0x29).
    ready = lox.begin();

    lastUpdateTime = -TOF_UPDATE_INTERVAL;
    rawMillimeters = -1;
    inRange = false;
    distance = -1.0;

    Distance();
}

double Tof::Distance()
{
    if (!ready)
    {
        this->rawMillimeters = -1;
        this->inRange = false;
        this->distance = -1.0;
        return distance;
    }

    const unsigned long now = millis();

    if (now >= TOF_UPDATE_INTERVAL + lastUpdateTime)
    {
        VL53L0X_RangingMeasurementData_t measure;
        lox.rangingTest(&measure, false);

        // RangeStatus 4 = phase failure / out of range -> data is invalid.
        if (measure.RangeStatus != 4)
        {
            this->rawMillimeters = measure.RangeMilliMeter;
            this->inRange = true;
            this->distance = rawMillimeters / 10.0; // mm -> cm
        }
        else
        {
            this->rawMillimeters = -1;
            this->inRange = false;
            this->distance = -1.0; // out of range -> "no close target"
        }

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

    if (!inRange)
    {
        Serial.println("out of range");
        return;
    }

    Serial.print("Raw mm: ");
    Serial.print(rawMillimeters);
    Serial.print(" | Distance: ");
    Serial.println(distance, kPrintDigitsAfterDecimal);
}
