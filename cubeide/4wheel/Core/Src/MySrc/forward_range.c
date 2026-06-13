#include "MySrc/forward_range.h"

/*
 * Fusion policy: the ToF is the primary validator, and the fused distance is
 * decided in three bands keyed off the ToF reading:
 *
 *   - tof_cm <  FR_TOF_MIN_CM (~4 cm): point-blank. Neither sensor can range
 *     this reliably (the ToF is below its usable floor and the Sharp folds back
 *     in the near field), so we report 0 - "too close to measure" - rather than
 *     a misleading number.
 *   - FR_TOF_MIN_CM <= tof_cm < FR_SHARP_MIN_CM (~8 cm): the ToF is trusted on
 *     its own; the Sharp is still in its non-monotonic near zone, so it is not
 *     considered yet.
 *   - FR_SHARP_MIN_CM <= tof_cm <= FR_TOF_TRUST_MAX_CM: both are in their good
 *     ranges. If the Sharp agrees within AGREE_TOL_CM the two are averaged
 *     (smoother), otherwise the ToF wins.
 *   - tof_cm > FR_TOF_TRUST_MAX_CM, or the ToF is invalid/blocked: fall back to
 *     the Sharp (far range). If the Sharp is also unusable, hold the last value
 *     and clear valid.
 */
#define FR_TOF_MIN_CM           4.0   /* below this the ToF can't be trusted -> report 0 */
#define FR_SHARP_MIN_CM         8.0   /* Sharp joins the decision at/above this (monotonic floor) */
#define FR_TOF_TRUST_MAX_CM   120.0   /* ToF gets noisy / saturates beyond this */
#define FR_AGREE_TOL_CM        15.0   /* ToF & Sharp "agree" within this many cm */
#define FR_SHARP_VALID_VOLT     0.5   /* mirrors Seek_IsSeen's middle-voltage gate */
#define FR_SHARP_MAX_CM       150.0   /* Sharp practical reach */

static bool ForwardRange_SharpValid(const SharpManager *sharp, double sharp_cm)
{
    int raw_adc;
    double voltage;

    if (sharp == 0)
    {
        return false;
    }

    raw_adc = SharpManager_GetRawAdc(sharp, SHARP_SENSOR_MIDDLE);
    voltage = SharpManager_AdcToVoltage(raw_adc, 3.3, 4095.0);

    if (voltage <= FR_SHARP_VALID_VOLT)
    {
        return false;
    }

    /* Only trust the Sharp inside its monotonic band; below FR_SHARP_MIN_CM the
     * GP2-style curve folds back and reports a near object as a mid-range one. */
    return (sharp_cm >= FR_SHARP_MIN_CM) && (sharp_cm <= FR_SHARP_MAX_CM);
}

void ForwardRange_Init(ForwardRange *self, Vl53l0x *tof, const SharpManager *sharp)
{
    if (self == 0)
    {
        return;
    }

    self->tof = tof;
    self->sharp = sharp;
    self->fused_cm = 0.0;
    self->valid = false;
}

void ForwardRange_Update(ForwardRange *self)
{
    double tof_cm;
    double sharp_cm;
    bool tof_ok;
    bool sharp_ok;

    if (self == 0)
    {
        return;
    }

    tof_ok = Vl53l0x_IsValid(self->tof);
    tof_cm = Vl53l0x_GetDistanceCm(self->tof);

    sharp_cm = SharpManager_GetDistance(self->sharp, SHARP_SENSOR_MIDDLE);
    sharp_ok = ForwardRange_SharpValid(self->sharp, sharp_cm);

    /* The ToF is the primary validator. */
    if (tof_ok)
    {
        /* Point-blank: too close for either sensor -> report 0. */
        if (tof_cm < FR_TOF_MIN_CM)
        {
            self->fused_cm = 0.0;
            self->valid = true;
            return;
        }

        /* ToF inside its trusted band. */
        if (tof_cm <= FR_TOF_TRUST_MAX_CM)
        {
            /* Bring the Sharp in only once we're above its near fold-back zone. */
            if (sharp_ok && (tof_cm >= FR_SHARP_MIN_CM))
            {
                double diff = tof_cm - sharp_cm;
                if (diff < 0.0)
                {
                    diff = -diff;
                }

                if (diff <= FR_AGREE_TOL_CM)
                {
                    /* Both agree -> average for a smoother estimate. */
                    self->fused_cm = (tof_cm + sharp_cm) * 0.5;
                }
                else
                {
                    /* Disagreement -> trust the ToF. */
                    self->fused_cm = tof_cm;
                }
            }
            else
            {
                /* ~4-8 cm, or no usable Sharp: ToF alone. */
                self->fused_cm = tof_cm;
            }

            self->valid = true;
            return;
        }

        /* tof_cm > trust max -> ToF reads "far"; let the Sharp take over below. */
    }

    /* Fallback: Sharp (far range / ToF saturated or blocked). */
    if (sharp_ok)
    {
        self->fused_cm = sharp_cm;
        self->valid = true;
        return;
    }

    /* Neither source usable -> hold last value, flag invalid. */
    self->valid = false;
}

double ForwardRange_GetDistanceCm(const ForwardRange *self)
{
    return (self == 0) ? 0.0 : self->fused_cm;
}

bool ForwardRange_IsValid(const ForwardRange *self)
{
    return (self == 0) ? false : self->valid;
}
