#pragma once

#include <VL53L0X.h>

class Tof
{
public:

void Init();
double Distance();   // cm; <= 0 means not ready or timeout
void DebugPrint();

private:

VL53L0X sensor;

bool ready;
bool timedOut;

unsigned long lastUpdateTime;

int rawMillimeters;
double distance;
};
