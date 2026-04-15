#ifndef MYSRC_WHEEL_H
#define MYSRC_WHEEL_H

#include <stdbool.h>
#include <stdint.h>

typedef struct
{
    bool direction;
    uint8_t forward_output;
    uint8_t backward_output;
} Wheel;

void Wheel_Init(Wheel *self, uint8_t forward_output, uint8_t backward_output);
void Wheel_SetRotation(Wheel *self, bool direction, int strength);

#endif /* MYSRC_WHEEL_H */
