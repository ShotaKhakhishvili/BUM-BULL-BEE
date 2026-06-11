#pragma once

class Sharp
{
public:

void Init();
double Distance();
void DebugPrint();

private:

unsigned long lastUpdateTime;
bool seeded;

int rawAdc;
double volt;
double distance;
};