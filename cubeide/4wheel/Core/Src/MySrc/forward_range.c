#include "MySrc/forward_range.h"

/*
 * Fusion policy: ToF-primary with Sharp fallback / cross-check.
 *
 *   - In the ToF's trustworthy band (<= TOF_TRUST_MAX_CM) the ToF leads. If the
 *     long Sharp also has a usable reading and the two agree within AGREE_TOL_CM,
 *     they are averaged for a smoother, less noisy estimate. If they disagree,
 *     the ToF wins (it is more accurate at close/mid range).
 *   - Outside that band (or when the ToF is blocked / saturated / invalid) the
 *     long Sharp takes over, covering the far range.
 *   - If neither source is usable, the last value is held and valid is cleared.
 */
#define FR_TOF_TRUST_MAX_CM   120.0   /* ToF gets noisy / saturates beyond this */
#define FR_AGREE_TOL_CM        15.0   /* ToF & Sharp "agree" within this many cm */
#define FR_SHARP_VALID_VOLT     0.5   /* mirrors Seek_IsSeen's long-voltage gate */
#define FR_SHARP_MIN_CM         0.0
#define FR_SHARP_MAX_CM       150.0   /* long Sharp practical reach */

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

    return (sharp_cm > FR_SHARP_MIN_CM) && (sharp_cm <= FR_SHARP_MAX_CM);
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

    /* Primary: ToF when valid and within its trustworthy band. */
    if (tof_ok && (tof_cm > 0.0) && (tof_cm <= FR_TOF_TRUST_MAX_CM))
    {
        if (sharp_ok)
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
                /* Disagreement near range -> trust the ToF. */
                self->fused_cm = tof_cm;
            }
        }
        else
        {
            self->fused_cm = tof_cm;
        }

        self->valid = true;
        return;
    }

    /* Fallback: long Sharp (far range / ToF saturated or blocked). */
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
