#include "Defines.hpp"
#include "Infrared.hpp"

void Infrared::Init(int pin, bool readsAnalog)
{
    this->pin = pin;
    this->readsAnalog = readsAnalog;

    // No pull-up in analog mode - it would skew the reading.
    pinMode(pin, readsAnalog ? INPUT : INPUT_PULLUP);

    Detected();
}

bool Infrared::Detected()
{
    if (readsAnalog)
    {
        level = analogRead(pin);
        return level >= IR_ANALOG_THRESHOLD;
    }

    level = digitalRead(pin);
    return level == IR_DETECTED;
}

int Infrared::Raw()
{
    return level;
}

void Infrared::DebugPrint()
{
    this->Detected();

    Serial.print("IR (pin ");
    Serial.print(pin);
    Serial.print(") : | Level: ");
    Serial.print(level);
    Serial.print(" | ");
    Serial.println(Detected() ? "LINE" : "NOLINE");
}
