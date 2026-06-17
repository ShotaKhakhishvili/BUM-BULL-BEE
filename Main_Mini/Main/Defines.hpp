#pragma once

#include "Library.hpp"

#include <Arduino.h>

#define LED  13

// Debug

#define kPrintDigitsAfterDecimal 3

// ------------------- DISTANCE SENSORS ----------------------

#define SHARP_PIN       A3      // Sharp GP2Y0A21 analog out

#define IR1             7       // D7 - digital IR sensor
#define IR2             8       // D8 - digital IR sensor

// Logic level a digital IR module outputs when it sees a target.
// Flip to HIGH if your modules are active-high.
#define IR_DETECTED     LOW

#define MODEL_LONG  1080    // GP2Y0A21YK0F : 10–80 cm
#define SHARP_UPDATE_INTERVAL 40
#define SHARP_SMOOTHING_ALPHA 0.3   // EMA weight: higher = snappier, lower = smoother

// VL53L0X Time-of-Flight (I2C: SDA = A4, SCL = A5)
#define TOF_UPDATE_INTERVAL 40

// Sharp <-> ToF handoff (cm). The ToF is linear and low-error, so it owns
// the decision: use the ToF while it reads closer than this limit, hand to
// the Sharp when the ToF reads farther (or drops out of range).
#define TOF_RANGE_LIMIT         18.0

// -------------------- MOTOR DRIVER (TB6612FNG) -------------
// Each motor: one PWM (speed) pin + two direction pins.
// NOTE: STBY must be tied HIGH (to VCC) to enable the driver.

#define PWMA          3     // D3  - Motor A speed (PWM)
#define AIN1          12    // D12 - Motor A direction
#define AIN2          4     // D4  - Motor A direction

#define PWMB          9     // D9  - Motor B speed (PWM)
#define BIN1          A2    // Motor B direction
#define BIN2          A1    // Motor B direction

// -------------------- OTHERS -------------------------------

#define RESET         13

#define SW_ON         1
#define SW_OFF        0

#define RIGHT         false
#define LEFT          true
#define FORWARD       true
#define BACKWARD      false
#define WHITE         0
#define BLACK         1

#define DEL           delay(1000)

#define MAX_DISTANCE  65
#define LIGHT_REFRESH 15L
#define WHITE_LIMIT   25L

// -------------------- BEHAVIOR -----------------------------

#define DETECT_DISTANCE   60.0
#define LOSE_DISTANCE     70.0

#define SEEK_SPEED        120   
#define CHASE_SPEED       190  

#define SEEK_DIR          RIGHT

