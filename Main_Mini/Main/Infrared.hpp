#pragma once

// Digital IR sensor (e.g. obstacle/edge module) with a HIGH/LOW output.
class Infrared
{
public:

void Init(int pin);
bool Detected();   // true when the sensor sees a target (see IR_DETECTED)
int  Raw();        // last raw digital level
void DebugPrint();

private:

int pin;
int level;
};
