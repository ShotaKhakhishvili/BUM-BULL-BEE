#pragma once

#include "Adafruit_VL53L0X.h"

class Tof
{
public:

void Init();
double Distance();   // cm; <= 0 means not ready or out of range
void DebugPrint();

private:

void ScanI2C();   // boot-time bus probe; VL53L0X should answer at 0x29

Adafruit_VL53L0X lox;

bool ready;
bool inRange;

unsigned long lastUpdateTime;

int rawMillimeters;
double distance;
};
