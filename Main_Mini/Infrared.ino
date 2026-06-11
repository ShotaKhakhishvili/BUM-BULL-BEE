#include "Defines.hpp"
#include "Infrared.hpp"

void Infrared::Init()
{
    lastUpdateTime = -INFRARED_UPDATE_INTERVAL;

    Distance();
}

double Infrared::Distance()
{
    const unsigned long now = millis();

    if (now >= INFRARED_UPDATE_INTERVAL + lastUpdateTime)
    {
        this->rawAdc = analogRead(INFRARED);
        this->volt = AdcToVoltage(rawAdc);
        this->distance = ((INFRARED_MAX_ADC - rawAdc) / INFRARED_MAX_ADC) * INFRARED_MAX_DISTANCE;

        lastUpdateTime = now;
    }

    return distance;
}

void Infrared::DebugPrint()
{
    this->Distance();
    
    Serial.print("Infrared Data : | Raw Adc: ");
    Serial.print(rawAdc);
    Serial.print(" | Voltage: ");
    Serial.print(volt, kPrintDigitsAfterDecimal);
    Serial.print(" | Distance: ");
    Serial.println(distance, kPrintDigitsAfterDecimal);
}
