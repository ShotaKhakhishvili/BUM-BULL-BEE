#include "MySrc/forward_range.h"

/*
 * Switching policy (no blending): the ToF only ranges to ~20 cm, so it owns the
 * near field and the Sharp owns everything beyond. The crossover is at
 * FR_CROSSOVER_CM (15 cm):
 *
 *   - range < 15 cm: use the ToF. Below FR_TOF_MIN_CM (~2 cm) it is too close
 *     for either sensor, so we report 0 ("too close to measure") rather than a
 *     misleading number.
 *   - range >= 15 cm (or the ToF is out of its short range / invalid): the Sharp
 *     reading is the final result.
 *
 * If the chosen sensor is unusable we fall back to the other one; if neither is
 * usable we hold the last value and clear valid.
 */
#define FR_TOF_MIN_CM           2.0   /* below this the ToF can't be trusted -> report 0 */
#define FR_SHARP_MIN_CM         8.0   /* Sharp is monotonic at/above this (near fold-back below) */
#define FR_CROSSOVER_CM        15.0   /* < this -> ToF; >= this -> Sharp */
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

    /* Near field (< 15 cm): the ToF owns it. */
    if (tof_ok && (tof_cm < FR_CROSSOVER_CM))
    {
        /* Point-blank: too close for either sensor -> report 0. */
        self->fused_cm = (tof_cm < FR_TOF_MIN_CM) ? 0.0 : tof_cm;
        self->valid = true;
        return;
    }

    /* >= 15 cm, or ToF out of its short range: the Sharp is the final result. */
    if (sharp_ok)
    {
        self->fused_cm = sharp_cm;
        self->valid = true;
        return;
    }

    /* Sharp unusable but the ToF still has a reading -> fall back to it. */
    if (tof_ok)
    {
        self->fused_cm = (tof_cm < FR_TOF_MIN_CM) ? 0.0 : tof_cm;
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
