#pragma once
#include "Defines.hpp"
#include "Wheel.hpp"

namespace Move
{
  // Motor A -> Left, Motor B -> Right.
  // Swap these two lines (or the motor wiring) if the bot drives mirrored.
  static Wheel L(PWMA, AIN1, AIN2);
  static Wheel R(PWMB, BIN1, BIN2);

  void Init();

  void RotateOnPoint(bool dir, int str);

  void RotateOnSide(bool dir, int str);

  void Walk(bool dir, int str);

  void SlideFwd(bool dir, int str, float coef);

  void SlideBwd(bool dir, int str, float coef);
};

