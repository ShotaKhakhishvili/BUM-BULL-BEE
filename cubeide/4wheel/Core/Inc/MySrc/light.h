#ifndef MYSRC_LIGHT_H
#define MYSRC_LIGHT_H

#include <stdbool.h>
#include <stdint.h>

typedef struct
{
    uint8_t input_id;
    bool last_col;
    uint32_t last_refresh;
    bool rev;
    uint32_t last_white;
} Light;

void Light_Init(Light *self, uint8_t input_id, bool rev);
bool Light_GetCol(Light *self);

#endif /* MYSRC_LIGHT_H */
