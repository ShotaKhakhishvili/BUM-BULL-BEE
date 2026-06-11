#pragma once

#include "Sharp.hpp"
#include "Infrared.hpp"

class Distance
{
public:

void Init();
double Get();
void DebugPrint(int whatToDebug); // 1 -> sharp, 2 -> infrared, 3-> both


private:

Sharp sharp;
Infrared infrared;

};