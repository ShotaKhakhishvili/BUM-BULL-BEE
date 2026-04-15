#ifndef MYSRC_MOVE_H
#define MYSRC_MOVE_H

#include <stdbool.h>

#include "MySrc/wheel.h"

typedef struct
{
    Wheel left;
    Wheel right;
} Move;

void Move_Init(Move *self);

void Move_RotateOnPoint(Move *self, bool direction, int strength);
void Move_RotateOnSide(Move *self, bool direction, int strength);
void Move_Walk(Move *self, bool direction, int strength);
void Move_SlideFwd(Move *self, bool direction, int strength, float coef);
void Move_SlideBwd(Move *self, bool direction, int strength, float coef);

#endif /* MYSRC_MOVE_H */
