#ifndef MYSRC_MAGNET_H
#define MYSRC_MAGNET_H

#include <stdbool.h>
#include <stdint.h>

/*
 * Electromagnet attachment control (PA1, plain on/off GPIO).
 *
 * The magnet pulls the chassis down onto the steel arena floor. It used to be
 * PWM-driven (PA11 / TIM1_CH4) for variable grip, but it now hangs off a plain
 * digital output on PA1, so grip is binary: any non-zero strength energizes the
 * coil, zero releases it. The two match levels collapse accordingly:
 *   - DEFAULT / CLOSE: magnet on (both non-zero).
 *   - OFF:             magnet released.
 *
 * The strength constants and the 0..MAGNET_PWM_PERIOD scale are kept so the rest
 * of the code (and a future return to PWM) needs no changes; only the sign of
 * the value matters while driven as GPIO.
 */
#define MAGNET_PWM_PERIOD        1000U
#define MAGNET_STRENGTH_OFF         0U
#define MAGNET_STRENGTH_DEFAULT   100U
#define MAGNET_STRENGTH_CLOSE     200U

typedef struct
{
    uint16_t strength;   /* last applied strength (non-zero = energized) */
    bool started;        /* true once the output has been brought up */
} Magnet;

/* Starts the PWM output and applies the DEFAULT holding strength. */
void Magnet_Init(Magnet *self);

/* Sets an arbitrary strength (clamped to MAGNET_PWM_PERIOD). */
void Magnet_SetStrength(Magnet *self, uint16_t strength);

void Magnet_Default(Magnet *self);   /* MAGNET_STRENGTH_DEFAULT */
void Magnet_Close(Magnet *self);     /* MAGNET_STRENGTH_CLOSE   */
void Magnet_Off(Magnet *self);       /* de-energize (release)   */

uint16_t Magnet_GetStrength(const Magnet *self);

#endif /* MYSRC_MAGNET_H */
