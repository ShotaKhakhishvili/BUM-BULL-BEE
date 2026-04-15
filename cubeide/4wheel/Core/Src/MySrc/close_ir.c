#include "MySrc/close_ir.h"

#include "MySrc/platform_adapter.h"

#define CLOSE_IR_DETECT_THRESHOLD 500
#define CLOSE_IR_UPDATE_DELAY_MS 5U
#define CLOSE_IR_ALPHA 0.4

void CloseIR_Init(CloseIR *self)
{
    uint32_t now;

    if (self == 0)
    {
        return;
    }

    now = Platform_Millis();

    self->last_read = now - CLOSE_IR_UPDATE_DELAY_MS - 1U;
    self->raw_adc = (int)Platform_ReadIrCloseAdc();
    self->filtered_adc = (double)self->raw_adc;
    self->sees_object = (self->filtered_adc >= (double)CLOSE_IR_DETECT_THRESHOLD);
}

void CloseIR_Update(CloseIR *self)
{
    uint32_t now;

    if (self == 0)
    {
        return;
    }

    now = Platform_Millis();

    if ((now - self->last_read) < CLOSE_IR_UPDATE_DELAY_MS)
    {
        return;
    }

    self->raw_adc = (int)Platform_ReadIrCloseAdc();
    self->filtered_adc =
        (self->filtered_adc * (1.0 - CLOSE_IR_ALPHA)) +
        ((double)self->raw_adc * CLOSE_IR_ALPHA);
    self->sees_object = (self->filtered_adc >= (double)CLOSE_IR_DETECT_THRESHOLD);

    self->last_read = now;
}

int CloseIR_GetRawAdc(const CloseIR *self)
{
    return (self == 0) ? 0 : self->raw_adc;
}

bool CloseIR_SeesObject(const CloseIR *self)
{
    return (self == 0) ? false : self->sees_object;
}
