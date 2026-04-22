#ifndef MYSRC_SEEK_H
#define MYSRC_SEEK_H

#include <stdbool.h>

#include "MySrc/move.h"
#include "MySrc/sharp_manager.h"

typedef enum
{
    SEEK_MODE_NONE = 0,
    SEEK_MODE_LOOK = 1,
    SEEK_MODE_CHASE = 2,
    SEEK_MODE_CATCH = 3
} SeekMode;

typedef enum
{
    SEEK_STEER_STRAIGHT = 0,
    SEEK_STEER_LEFT = -1,
    SEEK_STEER_RIGHT = 1
} SeekSteerDirection;

typedef struct
{
    double see_threshold_cm;
    double balance_tolerance_cm;

    int look_rotate_speed;
    int chase_speed;
    int catch_speed;

    float small_turn_coef;
    float medium_turn_coef;
    float high_turn_coef;
} SeekTuning;

#define SEEK_TUNING_DEFAULT_INITIALIZER \
    {                                   \
        65.0,                           \
        1.0,                            \
        50,                             \
        100,                            \
        150,                            \
        0.60f,                          \
        0.35f,                          \
        0.10f                           \
    }

typedef struct
{
    SeekTuning tuning;
    SeekSteerDirection last_steer_direction;
    bool target_visible;
} Seek;

void Seek_Init(Seek *self);
void Seek_SetTuning(Seek *self, const SeekTuning *tuning);

void Seek_Update(
    Seek *self,
    SeekMode mode,
    Move *move,
    const SharpManager *sharp_manager);

SeekTuning Seek_GetTuning(const Seek *self);
SeekSteerDirection Seek_GetLastSteerDirection(const Seek *self);
bool Seek_IsTargetVisible(const Seek *self);

#endif /* MYSRC_SEEK_H */
