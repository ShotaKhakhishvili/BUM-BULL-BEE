#ifndef MYSRC_MOVE_H
#define MYSRC_MOVE_H

#define MOVE_FORWARD true
#define MOVE_BACKWARD false
#define ROT_RIGHT true
#define ROT_LEFT false

#include <stdbool.h>

#include "MySrc/wheel.h"

typedef struct
{
    Wheel left;
    Wheel right;
    uint32_t moveStartTime;
    uint32_t moveEndTime;
} Move;

void Move_Init(Move *self);

void Move_Update(Move *self);
void Move_RotateOnPoint(Move *self, bool direction, int strength);
void Move_RotateOnSide(Move *self, bool direction, int strength);
void Move_Walk(Move *self, bool direction, int strength);
void Move_WalkForTime(Move *self, bool direction, int strength, int duration_millis);
void Move_SlideFwd(Move *self, bool direction, int strength, float coef);
void Move_SlideBwd(Move *self, bool direction, int strength, float coef);

#endif /* MYSRC_MOVE_H */
