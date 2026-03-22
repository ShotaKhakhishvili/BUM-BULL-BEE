#pragma once

#include <Arduino.h>

#define LED           PC13

// ------------------- DISTANCE SENSORS ----------------------

#define IR_CLOSE_A  PA1
#define IR_M        PA3
#define IR_L        PA7
#define IR_R        PA6
#define IR_SML      PA2

#define MODEL_SHORT 20150    // GP2Y0A41SK0F : 2–15 cm
#define MODEL_LONG  1080    // GP2Y0A21YK0F : 10–80 cm

// -------------------- WHEELS -------------------------------

#define WH_RB         PB6 
#define WH_RF         PB7 
#define WH_LF         PB8 
#define WH_LB         PB9 

// -------------------- COLORS -------------------------------

#define COL1          PA15     
#define COL2          PB3    
#define COL3          PB5    
#define COL4          PB4    

// -------------------- OTHERS -------------------------------

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
#define LIGHT_REFRESH 15L  
#define WHITE_LIMIT   25L

