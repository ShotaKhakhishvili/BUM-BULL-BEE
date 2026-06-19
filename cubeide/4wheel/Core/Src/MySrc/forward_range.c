#include "MySrc/forward_range.h"

/*
 * Switching policy (no blending), keyed on the ToF reading: the ToF owns the
 * near field and the Sharp owns everything beyond a single crossover:
 *
 *   - ToF reads under FR_TOF_NEAR_CM (18 cm)  -> use the ToF.
 *   - ToF reads at/above 18 cm, or drops out  -> use the Sharp.
 *
 * If the chosen sensor is unusable we fall back to the other one; if neither is
 * usable we hold the last value and clear valid. On the ToF, a reading below
 * FR_TOF_MIN_CM (~2 cm) is point-blank and reported as 0 ("too close to measure")
 * rather than a misleading number.
 */
#define FR_TOF_MIN_CM           2.0   /* below this the ToF can't be trusted -> report 0 */
#define FR_TOF_NEAR_CM         18.0   /* ToF owns < 18 cm; hand off to the Sharp at/above it */
#define FR_SHARP_MIN_CM         8.0   /* Sharp is monotonic at/above this (near fold-back below) */
#define FR_SHARP_VALID_VOLT    0.43   /* mirrors Seek's SEEK_SEE_MIN_VOLT (see ~60 cm) */
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
    self->using_tof = false;   /* start on the Sharp (far range) */
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

    /* Use the ToF only while it reports inside the near field (< 18 cm); past
     * that, or if it drops out, the Sharp takes over. */
    self->using_tof = tof_ok && (tof_cm < FR_TOF_NEAR_CM);

    if (self->using_tof)
    {
        /* Point-blank: too close for either sensor -> report 0. */
        self->fused_cm = (tof_cm < FR_TOF_MIN_CM) ? 0.0 : tof_cm;
        self->valid = true;
        return;
    }

    if (sharp_ok)
    {
        self->fused_cm = sharp_cm;
        self->valid = true;
        return;
    }

    /* Sharp unusable -> fall back to the ToF even past the threshold. */
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
