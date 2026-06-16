#pragma once

double AdcToVoltage(int adc)
{
    return (adc / 1024.0) * 5.0;
}