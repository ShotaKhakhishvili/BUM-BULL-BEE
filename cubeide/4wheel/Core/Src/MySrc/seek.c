#include "MySrc/seek.h"

static int Seek_ClampSpeed(int speed)
{
    if (speed < 0)
    {
        return 0;
    }

    if (speed > 255)
    {
        return 255;
    }

    return speed;
}

static float Seek_ClampCoef(float coef)
{
    if (coef < 0.0f)
    {
        return 0.0f;
    }

    if (coef > 1.0f)
    {
        return 1.0f;
    }

    return coef;
}

static int Seek_ScaleSpeedByCoef(int speed, float coef)
{
    double scaled;

    speed = Seek_ClampSpeed(speed);
    coef = Seek_ClampCoef(coef);

    scaled = (double)speed * (double)coef;

    if (scaled < 0.0)
    {
        return 0;
    }

    if (scaled > 255.0)
    {
        return 255;
    }

    return (int)scaled;
}

static bool Seek_IsSeen(double distance_cm, double threshold_cm)
{
    return (distance_cm > 0.0) && (distance_cm <= threshold_cm);
}

static SeekSteerDirection Seek_BalanceDirection(
    double left_cm,
    double right_cm,
    double tolerance_cm)
{
    if ((left_cm + tolerance_cm) < right_cm)
    {
        return SEEK_STEER_LEFT;
    }

    if ((right_cm + tolerance_cm) < left_cm)
    {
        return SEEK_STEER_RIGHT;
    }

    return SEEK_STEER_STRAIGHT;
}

static void Seek_Stop(Move *move)
{
    Move_Walk(move, MOVE_FORWARD, 0);
}

static void Seek_LookRight(const Seek *self, Move *move)
{
    Move_RotateOnPoint(move, ROT_RIGHT, Seek_ClampSpeed(self->tuning.look_rotate_speed));
}

static void Seek_MoveForward(Move *move, int speed)
{
    Move_Walk(move, MOVE_FORWARD, Seek_ClampSpeed(speed));
}

static void Seek_MoveWithSteer(
    Move *move,
    int speed,
    SeekSteerDirection direction,
    float coef)
{
    int left_speed;
    int right_speed;

    speed = Seek_ClampSpeed(speed);
    coef = Seek_ClampCoef(coef);

    left_speed = speed;
    right_speed = speed;

    if (direction == SEEK_STEER_LEFT)
    {
        left_speed = Seek_ScaleSpeedByCoef(speed, coef);
    }
    else if (direction == SEEK_STEER_RIGHT)
    {
        right_speed = Seek_ScaleSpeedByCoef(speed, coef);
    }

    /* Keep forward motion consistent with Move_Walk wiring convention. */
    Wheel_SetRotation(&move->left, MOVE_FORWARD, left_speed);
    Wheel_SetRotation(&move->right, MOVE_BACKWARD, right_speed);
    move->moveEndTime = 0U;
}

static int Seek_GetBaseSpeed(const Seek *self, SeekMode mode)
{
    if (mode == SEEK_MODE_CATCH)
    {
        return self->tuning.catch_speed;
    }

    return self->tuning.chase_speed;
}

void Seek_Init(Seek *self)
{
    if (self == 0)
    {
        return;
    }

    self->tuning = (SeekTuning)SEEK_TUNING_DEFAULT_INITIALIZER;
    self->last_steer_direction = SEEK_STEER_STRAIGHT;
    self->target_visible = false;
}

void Seek_SetTuning(Seek *self, const SeekTuning *tuning)
{
    if ((self == 0) || (tuning == 0))
    {
        return;
    }

    self->tuning = *tuning;
}

void Seek_Update(
    Seek *self,
    SeekMode mode,
    Move *move,
    const SharpManager *sharp_manager)
{
    double middle_cm;
    double left_cm;
    double right_cm;
    bool sees_middle;
    bool sees_left;
    bool sees_right;
    int base_speed;
    SeekSteerDirection direction;

    if ((self == 0) || (move == 0) || (sharp_manager == 0))
    {
        return;
    }

    middle_cm = SharpManager_GetMiddleDistance(sharp_manager);
    left_cm = SharpManager_GetLeftDistance(sharp_manager);
    right_cm = SharpManager_GetRightDistance(sharp_manager);

    sees_middle = Seek_IsSeen(middle_cm, self->tuning.see_threshold_cm);
    sees_left = Seek_IsSeen(left_cm, self->tuning.see_threshold_cm);
    sees_right = Seek_IsSeen(right_cm, self->tuning.see_threshold_cm);

    self->target_visible = sees_middle || sees_left || sees_right;

    if (mode == SEEK_MODE_NONE)
    {
        Seek_Stop(move);
        self->last_steer_direction = SEEK_STEER_STRAIGHT;
        return;
    }

    if (mode == SEEK_MODE_LOOK)
    {
        Seek_LookRight(self, move);
        self->last_steer_direction = SEEK_STEER_RIGHT;
        return;
    }

    if ((mode != SEEK_MODE_CHASE) && (mode != SEEK_MODE_CATCH))
    {
        Seek_Stop(move);
        self->last_steer_direction = SEEK_STEER_STRAIGHT;
        return;
    }

    base_speed = Seek_GetBaseSpeed(self, mode);

    if (!self->target_visible)
    {
        Seek_LookRight(self, move);
        self->last_steer_direction = SEEK_STEER_RIGHT;
        return;
    }

    if (sees_middle && !sees_left && !sees_right)
    {
        Seek_MoveForward(move, base_speed);
        self->last_steer_direction = SEEK_STEER_STRAIGHT;
        return;
    }

    if (sees_middle && sees_left && sees_right)
    {
        direction = Seek_BalanceDirection(left_cm, right_cm, self->tuning.balance_tolerance_cm);
        Seek_MoveWithSteer(move, base_speed, direction, self->tuning.small_turn_coef);
        self->last_steer_direction = direction;
        return;
    }

    if (sees_middle && sees_left && !sees_right)
    {
        Seek_MoveWithSteer(move, base_speed, SEEK_STEER_LEFT, self->tuning.medium_turn_coef);
        self->last_steer_direction = SEEK_STEER_LEFT;
        return;
    }

    if (sees_middle && !sees_left && sees_right)
    {
        Seek_MoveWithSteer(move, base_speed, SEEK_STEER_RIGHT, self->tuning.medium_turn_coef);
        self->last_steer_direction = SEEK_STEER_RIGHT;
        return;
    }

    if (!sees_middle && sees_left && !sees_right)
    {
        Seek_MoveWithSteer(move, base_speed, SEEK_STEER_LEFT, self->tuning.high_turn_coef);
        self->last_steer_direction = SEEK_STEER_LEFT;
        return;
    }

    if (!sees_middle && !sees_left && sees_right)
    {
        Seek_MoveWithSteer(move, base_speed, SEEK_STEER_RIGHT, self->tuning.high_turn_coef);
        self->last_steer_direction = SEEK_STEER_RIGHT;
        return;
    }

    if (!sees_middle && sees_left && sees_right)
    {
        direction = Seek_BalanceDirection(left_cm, right_cm, self->tuning.balance_tolerance_cm);
        Seek_MoveWithSteer(move, base_speed, direction, self->tuning.small_turn_coef);
        self->last_steer_direction = direction;
        return;
    }

    Seek_MoveForward(move, base_speed);
    self->last_steer_direction = SEEK_STEER_STRAIGHT;
}

SeekTuning Seek_GetTuning(const Seek *self)
{
    if (self == 0)
    {
        return (SeekTuning)SEEK_TUNING_DEFAULT_INITIALIZER;
    }

    return self->tuning;
}

SeekSteerDirection Seek_GetLastSteerDirection(const Seek *self)
{
    return (self == 0) ? SEEK_STEER_STRAIGHT : self->last_steer_direction;
}

bool Seek_IsTargetVisible(const Seek *self)
{
    return (self == 0) ? false : self->target_visible;
}
