#pragma once

class Infrared
{
public:

void Init(int pin);
double Distance();
void DebugPrint();

private:

int pin;

unsigned long lastUpdateTime;

int rawAdc;
double volt;
double distance;
};
