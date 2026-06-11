#pragma once
#include "Defines.hpp"
#include "Wheel.hpp"

namespace Move
{
  static Wheel L(WH_LF, WH_LB);
  static Wheel R(WH_RF, WH_RB);

  void RotateOnPoint(bool dir, int str);

  void RotateOnSide(bool dir, int str);

  void Walk(bool dir, int str);

  void SlideFwd(bool dir, int str, float coef);

  void SlideBwd(bool dir, int str, float coef);
};

