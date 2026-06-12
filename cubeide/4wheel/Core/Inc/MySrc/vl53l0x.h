#ifndef MYSRC_VL53L0X_H
#define MYSRC_VL53L0X_H

#include <stdbool.h>
#include <stdint.h>

/*
 * Compact single-file VL53L0X time-of-flight driver (register-level port of the
 * Pololu/ST reduced API) running on the project's hi2c2 bus.
 *
 * Usage: Vl53l0x_Init() once (boots the sensor and starts continuous ranging),
 * then call Vl53l0x_Update() periodically from the main loop. The latest range
 * is exposed in mm/cm with a validity flag.
 */
typedef struct
{
    uint8_t address;                     /* 7-bit I2C address (default 0x29) */
    uint8_t stop_variable;               /* captured during init, used to restart ranging */
    uint32_t measurement_timing_budget_us;
    uint32_t last_read;                  /* Platform_Millis() of last successful poll */
    uint16_t last_distance_mm;           /* last raw range reading (mm) */
    bool initialized;                    /* true once Vl53l0x_Init succeeded */
    bool valid;                          /* true when last_distance_mm is in range */
} Vl53l0x;

/* Boots the sensor and starts continuous back-to-back ranging.
 * Returns false if the device does not answer / fails the model-ID check. */
bool Vl53l0x_Init(Vl53l0x *self);

/* Non-blocking: reads a new range only when the sensor has one ready. */
void Vl53l0x_Update(Vl53l0x *self);

double Vl53l0x_GetDistanceCm(const Vl53l0x *self);
uint16_t Vl53l0x_GetDistanceMm(const Vl53l0x *self);
bool Vl53l0x_IsValid(const Vl53l0x *self);

#endif /* MYSRC_VL53L0X_H */
