#pragma once

#include <Arduino.h>

#define LED           PC13

// ------------------- SHARP MODULE START ---------------

#define IR_SML PA2
#define IR_M   PA3
#define IR_L   PA7
#define IR_R   PA6

#define MODEL_SHORT 20150   // GP2Y0A41SK0F : 2–15 cm
#define MODEL_LONG  1080    // GP2Y0A21YK0F : 10–80 cm

// ------------------- SHARP MODULE END -----------------

// -------------------- WHEELS START -------------------------

#define WH_RB         PB6 
#define WH_RF         PB7 
#define WH_LF         PB8 
#define WH_LB         PB9 

// -------------------- WHEELS END ---------------------------

#define COL1          PA15     
#define COL2          PB3    
#define COL3          PB5    
#define COL4          PB4    

#define RESET         PB13

#define SW_ON         1
#define SW_OFF        0

#define RIGHT         false
#define LEFT          true
#define FORWARD       false
#define BACKWARD      true
#define WHITE         0
#define BLACK         1

#define DEL           delay(1000)

#define MAX_DISTANCE  65
#define SONAR_REFRESH 30L
#define LIGHT_REFRESH 15L  
#define WHITE_LIMIT   25L

// Mode
#define MODE_CHASE    1
#define MODE_SEEK     2
#define MODE_ADJUST   3

// Adjust
#define ADJ_NONE      0b0000
#define ADJ_FR        0b0001
#define ADJ_FL        0b0010
#define ADJ_BR        0b0100
#define ADJ_BL        0b1000
#define ADJ_FRONT     0b0011
#define ADJ_BACK      0b1100
#define ADJ_RIGHTS    0b0101
#define ADJ_LEFTS     0b1010

enum SeekMode
{
  SEEK_NONE     = 0b00,
  SEEK_RIGHT    = 0b01,
  SEEK_LEFT     = 0b10,
  SEEK_BOTH     = 0b11,
};

