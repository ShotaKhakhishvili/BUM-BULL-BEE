#pragma once

class Infrared
{
public:

void Init();
double Distance();
void DebugPrint();

private:

unsigned long lastUpdateTime;

int rawAdc;
double volt;
double distance;
};
