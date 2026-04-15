#ifndef MYSRC_CLOSE_IR_H
#define MYSRC_CLOSE_IR_H

#include <stdbool.h>
#include <stdint.h>

typedef struct
{
    uint32_t last_read;
    int raw_adc;
    double filtered_adc;
    bool sees_object;
} CloseIR;

void CloseIR_Init(CloseIR *self);
void CloseIR_Update(CloseIR *self);

int CloseIR_GetRawAdc(const CloseIR *self);
bool CloseIR_SeesObject(const CloseIR *self);

#endif /* MYSRC_CLOSE_IR_H */
