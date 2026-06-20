#include "RunState.hpp"
#include "Defines.hpp"
#include "Move.hpp"

#include <Arduino.h>

namespace RunState
{
  static volatile bool running = false;

  static void onStart()
  {
    running = true;
    digitalWrite(INDICATOR, HIGH);
  }

  void Init()
  {
    pinMode(START_PIN, INPUT);
    pinMode(STOP_PIN,  INPUT);

    pinMode(INDICATOR, OUTPUT);
    digitalWrite(INDICATOR, LOW);

    attachInterrupt(digitalPinToInterrupt(START_PIN), onStart, RISING);

    // D1 has no external interrupt, so use a pin-change interrupt (PCINT17).
    PCICR  |= (1 << PCIE2);
    PCMSK2 |= (1 << PCINT17);
  }

  bool IsRunning() { return running; }

  void DebugPrint()
  {
    Serial.print("RunState | ");
    Serial.println(running ? "RUNNING" : "STOPPED");
  }
}

// PCINT fires on any edge of the masked pin, so check the level.
ISR(PCINT2_vect)
{
  if (digitalRead(STOP_PIN) == HIGH)
  {
    RunState::running = false;
    digitalWrite(INDICATOR, LOW);
    Move::Walk(FORWARD, 0);
  }
}
