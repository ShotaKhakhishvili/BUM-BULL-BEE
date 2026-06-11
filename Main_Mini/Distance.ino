#include "Defines.hpp"
#include "Distance.hpp"

void Distance::Init()
{
    sharp.Init();
    infrared.Init();
}

double Distance::Get()
{
    if(infrared.Distance() > 3)
        return sharp.Distance();
    return infrared.Distance();
}

void Distance::DebugPrint(int whatToDebug)
{
    if(whatToDebug & 0b01)
        sharp.DebugPrint();
    if(whatToDebug & 0b10)
        infrared.DebugPrint();
}
