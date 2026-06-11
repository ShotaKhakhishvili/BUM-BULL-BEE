#pragma once

double AdcToVoltage(int adc)
{
    return (adc / 2048.0) * 5.0;
}