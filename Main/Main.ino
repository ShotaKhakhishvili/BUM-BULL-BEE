#include "Defines.hpp"
#include "ModeHandler.hpp"
#include "MoveHandler.hpp"
#include "Move.hpp"
#include "Light.hpp"
#include "Extra.hpp"

// ---------------------------- TEMP ----------------------------------- //

#include <SharpIR.h>

SharpIR sensor(IR_PIN, MODEL);

// ---------------------------- TEMP ----------------------------------- //

// Color Sensors
Light FR(COL1, false),FL(COL2, false),BR(COL3, false),BL(COL4, false);
Light* lights[] = {&FR, &FL, &BR, &BL};

// Modes
int mode = MODE_SEEK;
int adjMode = ADJ_NONE;
SeekMode seekMode = SeekMode::SEEK_NONE;

// Moves
int lastMoveCheck = 0;
int moveTime = 0;
bool seekRandDir = RIGHT;

void setup() 
{
  Serial.begin(115200);

  pinMode(LED, OUTPUT);

  pinMode(COL1, INPUT);
  pinMode(COL2, INPUT);
  pinMode(COL3, INPUT);
  pinMode(COL4, INPUT);

  pinMode(WH_LF, OUTPUT);
  pinMode(WH_LB, OUTPUT);
  pinMode(WH_RF, OUTPUT);
  pinMode(WH_RB, OUTPUT);

  pinMode(RESET, INPUT);

  lastMoveCheck = millis();
}

bool reset = 0;
int lastPrint = 0;

void loop() {

  int distance = sensor.distance();

  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  if(distance > 10){
    Move::walk(FORWARD, 100);
  }
  else{
    Move::walk(FORWARD, 0);
  }

  delay(100);
}

