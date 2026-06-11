#pragma once

double AdcToVoltage(int adc)
{
    return (adc / 1023.0) * 5.0;
}