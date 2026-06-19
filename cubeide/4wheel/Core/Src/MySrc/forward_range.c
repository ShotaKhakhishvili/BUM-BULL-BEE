#include "MySrc/forward_range.h"

/*
 * TEMP: forward range is ToF-ONLY. The Sharp fusion is disabled for now so we
 * can see how far the VL53L0X actually reaches on a real opponent. We report the
 * ToF distance whenever it is valid; otherwise the channel is invalid (no Sharp
 * fallback). A ToF reading below FR_TOF_MIN_CM (~2 cm) is point-blank and reported
 * as 0 ("too close to measure"). The ToF/Sharp crossover is in git history if we
 * want it back.
 */
#define FR_TOF_MIN_CM           2.0   /* below this the ToF can't be trusted -> report 0 */

void ForwardRange_Init(ForwardRange *self, Vl53l0x *tof, const SharpManager *sharp)
{
    if (self == 0)
    {
        return;
    }

    self->tof = tof;
    self->sharp = sharp;       /* stored but unused while ToF-only */
    self->fused_cm = 0.0;
    self->valid = false;
    self->using_tof = true;    /* ToF is the only source now */
}

void ForwardRange_Update(ForwardRange *self)
{
    double tof_cm;
    bool tof_ok;

    if (self == 0)
    {
        return;
    }

    tof_ok = Vl53l0x_IsValid(self->tof);
    tof_cm = Vl53l0x_GetDistanceCm(self->tof);

    self->using_tof = tof_ok;

    if (tof_ok)
    {
        /* Point-blank: too close to measure -> report 0. */
        self->fused_cm = (tof_cm < FR_TOF_MIN_CM) ? 0.0 : tof_cm;
        self->valid = true;
    }
    else
    {
        self->valid = false;
    }
}

double ForwardRange_GetDistanceCm(const ForwardRange *self)
{
    return (self == 0) ? 0.0 : self->fused_cm;
}

bool ForwardRange_IsValid(const ForwardRange *self)
{
    return (self == 0) ? false : self->valid;
}
