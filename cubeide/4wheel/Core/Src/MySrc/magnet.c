#include "MySrc/magnet.h"

#include "MySrc/platform_adapter.h"

void Magnet_SetStrength(Magnet *self, uint16_t strength)
{
    if (self == 0)
    {
        return;
    }

    if (strength > MAGNET_PWM_PERIOD)
    {
        strength = MAGNET_PWM_PERIOD;
    }

    self->strength = strength;
    Platform_MagnetSetCompare(strength);
}

void Magnet_Init(Magnet *self)
{
    if (self == 0)
    {
        return;
    }

    self->strength = 0U;
    self->started = false;

    /* Bring the PWM up first, then apply the default holding force so the bot
     * stays planted on the floor even before the match start signal. */
    Platform_MagnetStartPwm();
    self->started = true;

    Magnet_SetStrength(self, MAGNET_STRENGTH_DEFAULT);
}

void Magnet_Default(Magnet *self)
{
    Magnet_SetStrength(self, MAGNET_STRENGTH_DEFAULT);
}

void Magnet_Close(Magnet *self)
{
    Magnet_SetStrength(self, MAGNET_STRENGTH_CLOSE);
}

void Magnet_Off(Magnet *self)
{
    Magnet_SetStrength(self, MAGNET_STRENGTH_OFF);
}

uint16_t Magnet_GetStrength(const Magnet *self)
{
    return (self == 0) ? 0U : self->strength;
}
