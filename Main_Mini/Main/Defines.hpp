#pragma once

#include "Library.hpp"

#include <Arduino.h>

#define LED  5

// Debug

#define kPrintDigitsAfterDecimal 3

// ------------------- DISTANCE SENSORS ----------------------

#define SHARP_PIN       A1      // Sharp GP2Y0A21 analog out

// IR mode: false = digital modules on D7/D8, true = analog on A0/A1.
#define IR_READS_ANALOG false

#define IR1             2       // D7 - digital IR sensor
#define IR2             3       // D8 - digital IR sensor

#define IR1_ANALOG      A0      // analog IR sensor
#define IR2_ANALOG      A1      // analog IR sensor

// Logic level a digital IR module outputs when it sees a target.
// Flip to HIGH if your modules are active-high.
#define IR_DETECTED     LOW

// Analog mode: detected when the reading is at/above this (flip >= in
// Infrared::Detected if your sensor reads the other way).
#define IR_ANALOG_THRESHOLD 500

#define MODEL_LONG  1080    // GP2Y0A21YK0F : 10–80 cm
#define SHARP_UPDATE_INTERVAL 40
#define SHARP_SMOOTHING_ALPHA 0.3   // EMA weight: higher = snappier, lower = smoother

// VL53L0X Time-of-Flight (I2C: SDA = A4, SCL = A5)
#define TOF_UPDATE_INTERVAL 40

// Sharp <-> ToF handoff (cm). The ToF reports ~819 cm (8190 mm) when it sees
// nothing. So use the ToF whenever it returns a real reading, and fall back
// to the Sharp only when the ToF is blind (>= this) or times out (<= 0).
#define TOF_BLIND_CM            800.0

// -------------------- MOTOR DRIVER (TB6612FNG) -------------
// Each motor: one PWM (speed) pin + two direction pins.
// NOTE: STBY must be tied HIGH (to VCC) to enable the driver.

#define PWMA          9     // D3  - Motor A speed (PWM)
#define AIN1           8   // D12 - Motor A direction
#define AIN2          7    // D4  - Motor A direction

#define PWMB          10     // D9  - Motor B speed (PWM)
#define BIN1          6    // Motor B direction
#define BIN2          5    // Motor B direction

// -------------------- OTHERS -------------------------------

// Start/stop signal lines. WARNING: STOP is on D1, the UART TX pin - it
// can't be read reliably while Serial is active.
#define START_PIN     12   
#define STOP_PIN      13    

// Active indicator: HIGH while running, LOW while stopped.
#define INDICATOR     11     // TODO: set to an available digital output pin

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

#define DETECT_DISTANCE   55.0
#define LOSE_DISTANCE     60.0

#define SEEK_SPEED        120   
#define CHASE_SPEED       190  

#define SEEK_DIR          RIGHT

