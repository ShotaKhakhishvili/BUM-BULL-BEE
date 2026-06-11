#pragma once

#include <VL53L0X.h>

class Tof
{
public:

void Init();
double Distance();
void DebugPrint();

private:

VL53L0X sensor;

bool ready;

unsigned long lastUpdateTime;

int rawMillimeters;
double distance;
};
