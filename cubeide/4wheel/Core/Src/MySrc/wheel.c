#include "MySrc/wheel.h"

#include "MySrc/platform_adapter.h"

static int Wheel_ClampStrength(int strength)
{
    if (strength < 0)
    {
        return 0;
    }

    if (strength > 255)
    {
        return 255;
    }

    return strength;
}

void Wheel_Init(Wheel *self, uint8_t forward_output, uint8_t backward_output)
{
    if (self == 0)
    {
        return;
    }

    self->direction = false;
    self->forward_output = forward_output;
    self->backward_output = backward_output;

    Platform_WheelSetPwm(self->forward_output, 0U);
    Platform_WheelSetPwm(self->backward_output, 0U);
}

void Wheel_SetRotation(Wheel *self, bool direction, int strength)
{
    uint16_t clamped_strength;

    if (self == 0)
    {
        return;
    }

    clamped_strength = (uint16_t)Wheel_ClampStrength(strength);

    if (direction != self->direction)
    {
        Platform_WheelSetPwm(self->forward_output, 0U);
        Platform_WheelSetPwm(self->backward_output, 0U);
        Platform_DelayMs(1U);
    }

    if (direction)
    {
        Platform_WheelSetPwm(self->forward_output, clamped_strength);
        Platform_WheelSetPwm(self->backward_output, 0U);
    }
    else
    {
        Platform_WheelSetPwm(self->forward_output, 0U);
        Platform_WheelSetPwm(self->backward_output, clamped_strength);
    }

    self->direction = direction;
}
