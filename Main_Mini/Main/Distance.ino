#include "Defines.hpp"
#include "Distance.hpp"

void Distance::Init()
{
    sharp.Init();
    infrared.Init(INFRARED_M);
}

double Distance::Get()
{
    if(infrared.Distance() > 3)
        return sharp.Distance();
    return infrared.Distance();
}

double Distance::GetSharp()
{
    return sharp.Distance();
}

void Distance::DebugPrint(int whatToDebug)
{
    if(whatToDebug & 0b01)
        sharp.DebugPrint();
    if(whatToDebug & 0b10)
        infrared.DebugPrint();
}
