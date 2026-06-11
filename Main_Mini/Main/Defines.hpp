#pragma once

#include "Library.hpp"

#include <Arduino.h>

#define LED           PC13

// Debug

#define kPrintDigitsAfterDecimal 3

// ------------------- DISTANCE SENSORS ----------------------

#define IR          PA3
#define INFRARED    PA4     // analog pin for the linear IR sensor

#define MODEL_LONG  1080    // GP2Y0A21YK0F : 10–80 cm
#define SHARP_UPDATE_INTERVAL 40
#define SHARP_SMOOTHING_ALPHA 0.3   // EMA weight: higher = snappier, lower = smoother
#define INFRARED_UPDATE_INTERVAL 40

#define INFRARED_MAX_ADC      1023.0   // full-scale ADC count
#define INFRARED_MAX_DISTANCE 5.0      // cm when ADC reads 0 (sensor at full voltage)

// -------------------- WHEELS -------------------------------

#define WH_RB         PB6 
#define WH_RF         PB7 
#define WH_LF         PB8 
#define WH_LB         PB9 

// -------------------- OTHERS -------------------------------

#define RESET         PB13

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

