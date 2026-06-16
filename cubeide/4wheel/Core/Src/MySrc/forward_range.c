#include "MySrc/forward_range.h"

/*
 * Switching policy (no blending): the ToF only ranges to ~20 cm, so it owns the
 * near field and the Sharp owns everything beyond. The crossover uses hysteresis
 * with a 3 cm dead-band to stop chatter when the target hovers near it:
 *
 *   - while the Sharp is active: drop to the ToF once the distance falls below
 *     FR_SWITCH_TO_TOF_CM (13 cm).
 *   - while the ToF is active: climb back to the Sharp once the distance rises
 *     above FR_SWITCH_TO_SHARP_CM (16 cm).
 *
 * Either way, if the active sensor is unusable we fall back to the other one; if
 * neither is usable we hold the last value and clear valid. On the ToF, a reading
 * below FR_TOF_MIN_CM (~2 cm) is point-blank and reported as 0 ("too close to
 * measure") rather than a misleading number.
 */
#define FR_TOF_MIN_CM           2.0   /* below this the ToF can't be trusted -> report 0 */
#define FR_SHARP_MIN_CM         8.0   /* Sharp is monotonic at/above this (near fold-back below) */
#define FR_SWITCH_TO_TOF_CM    13.0   /* on Sharp: drop to ToF below this */
#define FR_SWITCH_TO_SHARP_CM  16.0   /* on ToF: climb to Sharp above this */
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

    /* Advance the hysteresis state machine using the active sensor's distance. */
    if (self->using_tof)
    {
        /* On the ToF: climb back to the Sharp once we're clearly past 16 cm,
         * or whenever the ToF reading drops out and the Sharp is usable. */
        if ((!tof_ok || (tof_cm > FR_SWITCH_TO_SHARP_CM)) && sharp_ok)
        {
            self->using_tof = false;
        }
    }
    else
    {
        /* On the Sharp: drop to the ToF once we fall below 13 cm, or whenever
         * the Sharp reading drops out and the ToF is usable. */
        if ((!sharp_ok || (sharp_cm < FR_SWITCH_TO_TOF_CM)) && tof_ok)
        {
            self->using_tof = true;
        }
    }

    /* Emit the active sensor, falling back to the other if it is unusable. */
    if (self->using_tof)
    {
        if (tof_ok)
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
    }
    else
    {
        if (sharp_ok)
        {
            self->fused_cm = sharp_cm;
            self->valid = true;
            return;
        }

        if (tof_ok)
        {
            self->fused_cm = (tof_cm < FR_TOF_MIN_CM) ? 0.0 : tof_cm;
            self->valid = true;
            return;
        }
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
