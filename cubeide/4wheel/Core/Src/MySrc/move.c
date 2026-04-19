#include "MySrc/move.h"

#include "MySrc/platform_adapter.h"

static int Move_ScaleStrength(int strength, float coef)
{
    float scaled;

    if (coef < 0.0f)
    {
        coef = 0.0f;
    }

    scaled = ((float)strength) * coef;

    if (scaled < 0.0f)
    {
        return 0;
    }

    if (scaled > 255.0f)
    {
        return 255;
    }

    return (int)scaled;
}

void Move_Update(Move *self)
{
    if(self == 0)
    {
        return;
    }

    if(self->moveEndTime > 0 && Platform_Millis() >= self->moveEndTime)
    {
        Move_Walk(self, MOVE_FORWARD, 0);
    }
}

void Move_Init(Move *self)
{
    if (self == 0)
    {
        return;
    }

    Wheel_Init(
        &self->left,
        PLATFORM_WHEEL_OUTPUT_LF,
        PLATFORM_WHEEL_OUTPUT_LB);

    Wheel_Init(
        &self->right,
        PLATFORM_WHEEL_OUTPUT_RF,
        PLATFORM_WHEEL_OUTPUT_RB);
}

void Move_RotateOnPoint(Move *self, bool direction, int strength)
{
    if (self == 0)
    {
        return;
    }

    Wheel_SetRotation(&self->left, direction, strength);
    Wheel_SetRotation(&self->right, !direction, strength);
}

void Move_RotateOnSide(Move *self, bool direction, int strength)
{
    if (self == 0)
    {
        return;
    }

    if (direction)
    {
        Wheel_SetRotation(&self->left, MOVE_BACKWARD, strength);
        Wheel_SetRotation(&self->right, MOVE_BACKWARD, 0);
    }
    else
    {
        Wheel_SetRotation(&self->left, MOVE_FORWARD, 0);
        Wheel_SetRotation(&self->right, MOVE_FORWARD, strength);
    }
}

void Move_Walk(Move *self, bool direction, int strength)
{
    if (self == 0)
    {
        return;
    }

    Wheel_SetRotation(&self->left, direction, strength);
    Wheel_SetRotation(&self->right, !direction, strength);

    self->moveEndTime = 0;
}

void Move_WalkForTime(Move *self, bool direction, int strength, int duration_millis)
{
    if(self == 0)
    {
        return;
    }

    Move_Walk(self, direction, strength);

    self->moveStartTime = Platform_Millis();
    self->moveEndTime = self->moveStartTime + duration_millis;
}

void Move_SlideFwd(Move *self, bool direction, int strength, float coef)
{
    if (self == 0)
    {
        return;
    }

    if (!direction)
    {
        Wheel_SetRotation(&self->left, MOVE_FORWARD, strength);
        Wheel_SetRotation(&self->right, MOVE_FORWARD, Move_ScaleStrength(strength, coef));
    }
    else
    {
        Wheel_SetRotation(&self->left, MOVE_FORWARD, Move_ScaleStrength(strength, coef));
        Wheel_SetRotation(&self->right, MOVE_FORWARD, strength);
    }
}

void Move_SlideBwd(Move *self, bool direction, int strength, float coef)
{
    if (self == 0)
    {
        return;
    }

    if (direction)
    {
        Wheel_SetRotation(&self->left, MOVE_BACKWARD, strength);
        Wheel_SetRotation(&self->right, MOVE_BACKWARD, Move_ScaleStrength(strength, coef));
    }
    else
    {
        Wheel_SetRotation(&self->left, MOVE_BACKWARD, Move_ScaleStrength(strength, coef));
        Wheel_SetRotation(&self->right, MOVE_BACKWARD, strength);
    }
}
