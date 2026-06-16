#ifndef MYSRC_FORWARD_RANGE_H
#define MYSRC_FORWARD_RANGE_H

#include <stdbool.h>

#include "MySrc/vl53l0x.h"
#include "MySrc/sharp_manager.h"

/*
 * Forward-distance combiner.
 *
 * Switches between the VL53L0X time-of-flight sensor (near field, ~2-20 cm) and
 * the long Sharp IR sensor (useful out to ~150 cm) with hysteresis: while on the
 * Sharp it drops to the ToF once the distance falls below 13 cm, and while on the
 * ToF it climbs back to the Sharp once the distance rises above 16 cm. The 3 cm
 * dead-band stops chatter near the crossover. The result is the single forward
 * distance consumed by Seek's middle channel and the collision check.
 */
typedef struct
{
    Vl53l0x *tof;                 /* time-of-flight sensor (primary) */
    const SharpManager *sharp;    /* Sharp half = long forward IR sensor */
    double fused_cm;              /* last selected forward distance (cm) */
    bool valid;                   /* true when fused_cm reflects a fresh reading */
    bool using_tof;               /* hysteresis state: true = ToF active, false = Sharp */
} ForwardRange;

void ForwardRange_Init(ForwardRange *self, Vl53l0x *tof, const SharpManager *sharp);

/* Recomputes the fused distance from the latest ToF + Sharp readings. */
void ForwardRange_Update(ForwardRange *self);

double ForwardRange_GetDistanceCm(const ForwardRange *self);
bool ForwardRange_IsValid(const ForwardRange *self);

#endif /* MYSRC_FORWARD_RANGE_H */
