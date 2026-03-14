#include "Defines.hpp"
#include "ModeHandler.hpp"
#include "MoveHandler.hpp"
#include "Move.hpp"
#include "Light.hpp"
#include "Sonic.hpp"
#include "Extra.hpp"

// Color Sensors
Light FR(COL1, false),FL(COL2, false),BR(COL3, false),BL(COL4, false);
Light* lights[] = {&FR, &FL, &BR, &BL};

// Ultrasonics
NewPing SON_LEFT(TRIG2, ECHO2, MAX_DISTANCE);
NewPing SON_RIGHT(TRIG3, ECHO3, MAX_DISTANCE);
NewPing sons[] = {SON_LEFT, SON_RIGHT};

// Modes
int mode = MODE_SEEK;
int adjMode = ADJ_NONE;
SeekMode seekMode = SeekMode::SEEK_NONE;

// Moves
int lastMoveCheck = millis();
int moveTime = 0;
bool seekRandDir = RIGHT;

void setup() 
{
  Serial.begin(115200);

  pinMode(LED, OUTPUT);

  // pinMode(TRIG1, OUTPUT);
  pinMode(TRIG2, OUTPUT);
  pinMode(TRIG3, OUTPUT);

  // pinMode(ECHO1, INPUT);
  pinMode(ECHO2, INPUT);
  pinMode(ECHO3, INPUT);

  pinMode(COL1, INPUT);
  pinMode(COL2, INPUT);
  pinMode(COL3, INPUT);
  pinMode(COL4, INPUT);

  pinMode(WH_LF, OUTPUT);
  pinMode(WH_LB, OUTPUT);
  pinMode(WH_RF, OUTPUT);
  pinMode(WH_RB, OUTPUT);

  pinMode(RESET, INPUT);
}

bool reset = 0;
int lastPrint = 0;

void loop() 
{
  bool newReset = digitalRead(RESET);

  if(!reset && newReset)
  {
    delay(5000);
  }
  if(reset && !newReset)
  {
    lastMoveCheck = millis();
    moveTime = 0;

    Move::walk(FORWARD, 0);
  }

  reset = newReset;

  if(!reset)
    return;

  ModeHandler::updateMode();
  MoveHandler::updateMove();

  // Serial.println();
  // Serial.print("MODE: ");
  // Serial.println(mode);

  // Serial.print("ADJUST MODE: ");
  // Serial.println(adjMode);

  // Serial.print("SEEK MODE: ");
  // Serial.println(static_cast<int>(seekMode));
  // Serial.println("-------------------------------------------");

  ////////////////////////////////////////////////////////////////////////

  // Extra::printAllSonicInfo(0);
  // Extra::printAllSonicInfo(1);
  // Serial.print("Seek Mode: ");
  // Serial.println(static_cast<int>(seekMode));
  // Serial.println("-------------------------------------------");
  
  ////////////////////////////////////////////////////////////////////////

  if(millis() - lastPrint > 1000)
  {
    lastPrint = millis();
    int line = ModeHandler::getLineCheck();
    Serial.print(line);
    Serial.println("-------------------------------------------");
  }


  ////////////////////////////////////////////////////////////////////////

  // delay(1000);
}

