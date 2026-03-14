#pragma once
#include "Wheel.hpp"

namespace Move
{
  static Wheel L(WH_LF, WH_LB);
  static Wheel R(WH_RF, WH_RB);

  void rotateOnPoint(bool dir, int str);

  void rotateOnSide(bool dir, int str);

  void walk(bool dir, int str);

  void slideFwd(bool dir, int str, float coef);

  void slideBwd(bool dir, int str, float coef);
};

