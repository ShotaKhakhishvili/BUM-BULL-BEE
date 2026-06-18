#ifndef MYSRC_MAGNET_H
#define MYSRC_MAGNET_H

#include <stdbool.h>
#include <stdint.h>

/*
 * Electromagnet attachment control (PA11 / TIM1_CH4 PWM).
 *
 * The magnets pull the chassis down onto the steel arena floor; the PWM duty
 * sets how hard it grips. Two operating levels are used during a match:
 *   - DEFAULT: a light, always-on holding force that still lets the bot drive.
 *   - CLOSE:   a stronger clamp engaged when an opponent is near, to resist
 *              being shoved off.
 *
 * Strength is the raw TIM1 compare value on the 0..MAGNET_PWM_PERIOD scale.
 * TIM1's period is 1000, so 100 = 10%% duty and 200 = 20%%. Retune the DEFAULT /
 * CLOSE levels here if the holding force needs adjusting.
 */
#define MAGNET_PWM_PERIOD        1000U
#define MAGNET_STRENGTH_OFF         0U
#define MAGNET_STRENGTH_DEFAULT   100U
#define MAGNET_STRENGTH_CLOSE     200U
/* Full-on: compare > period keeps the output active every tick (true 100%% / DC). */
#define MAGNET_STRENGTH_MAX     (MAGNET_PWM_PERIOD + 1U)

typedef struct
{
    uint16_t strength;   /* last applied TIM1 compare value */
    bool started;        /* true once the PWM channel is running */
} Magnet;

/* Starts the PWM output and applies the DEFAULT holding strength. */
void Magnet_Init(Magnet *self);

/* Sets an arbitrary strength (clamped to MAGNET_PWM_PERIOD). */
void Magnet_SetStrength(Magnet *self, uint16_t strength);

void Magnet_Default(Magnet *self);   /* MAGNET_STRENGTH_DEFAULT */
void Magnet_Close(Magnet *self);     /* MAGNET_STRENGTH_CLOSE   */
void Magnet_Full(Magnet *self);      /* MAGNET_STRENGTH_MAX (full-on) */
void Magnet_Off(Magnet *self);       /* de-energize (release)   */

uint16_t Magnet_GetStrength(const Magnet *self);

#endif /* MYSRC_MAGNET_H */
