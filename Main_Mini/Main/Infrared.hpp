#pragma once

// IR sensor. Reads either a digital HIGH/LOW module or an analog output,
// selected by readsAnalog at Init.
class Infrared
{
public:

void Init(int pin, bool readsAnalog = false);
bool Detected();   // digital: level == IR_DETECTED; analog: crosses threshold
int  Raw();        // last raw reading (0/1 digital, 0-1023 analog)
void DebugPrint();

private:

int pin;
int level;
bool readsAnalog;
};
