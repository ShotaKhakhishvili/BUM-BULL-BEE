#include "RunState.hpp"
#include "Defines.hpp"
#include "Move.hpp"

#include <Arduino.h>

namespace RunState
{
  static volatile bool running = false;
  static volatile bool stopped = false;   // latched once stop is pressed

  // Previous pin levels, so we react to a rising edge instead of a level.
  static volatile uint8_t prevStart = LOW;
  static volatile uint8_t prevStop  = LOW;

  // Enable a pin-change interrupt on any pin, on any port (PCINT group is
  // derived from the pin, so this works wherever START/STOP are defined).
  static void enablePinChange(uint8_t pin)
  {
    *digitalPinToPCMSK(pin) |= bit(digitalPinToPCMSKbit(pin));
    PCIFR |= bit(digitalPinToPCICRbit(pin));   // clear any stale flag
    PCICR |= bit(digitalPinToPCICRbit(pin));   // enable that group
  }

  // Shared handler: a pin-change ISR fires on either edge of any masked pin,
  // so read the levels and act only on a LOW->HIGH transition.
  static void handleChange()
  {
    uint8_t s = digitalRead(START_PIN);
    uint8_t e = digitalRead(STOP_PIN);

    // Start only works once, and never after a stop.
    if (s == HIGH && prevStart == LOW && !stopped)
    {
      running = true;
      digitalWrite(INDICATOR, HIGH);
    }

    // Stop latches: once pressed, start can no longer re-enable.
    if (e == HIGH && prevStop == LOW)
    {
      running = false;
      stopped = true;
      digitalWrite(INDICATOR, LOW);
      Move::Walk(FORWARD, 0);
    }

    prevStart = s;
    prevStop  = e;
  }

  void Init()
  {
    pinMode(START_PIN, INPUT);
    pinMode(STOP_PIN,  INPUT);

    pinMode(INDICATOR, OUTPUT);
    digitalWrite(INDICATOR, LOW);

    enablePinChange(START_PIN);
    enablePinChange(STOP_PIN);
  }

  bool IsRunning() { return running; }

  void DebugPrint()
  {
    Serial.print("RunState | ");
    Serial.println(running ? "RUNNING" : "STOPPED");
  }
}

// One vector per pin-change group; whichever pins are used route here.
ISR(PCINT0_vect) { RunState::handleChange(); }   // PORTB  (D8-D13)
ISR(PCINT1_vect) { RunState::handleChange(); }   // PORTC  (A0-A5)
ISR(PCINT2_vect) { RunState::handleChange(); }   // PORTD  (D0-D7)
