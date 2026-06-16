#include "Defines.hpp"
#include "Infrared.hpp"

void Infrared::Init(int pin)
{
    this->pin = pin;
    pinMode(pin, INPUT);

    Detected();
}

bool Infrared::Detected()
{
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
    Serial.print(" | Detected: ");
    Serial.println(Detected() ? "YES" : "no");
}
