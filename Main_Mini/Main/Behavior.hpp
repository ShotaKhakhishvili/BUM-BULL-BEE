#pragma once
#include "Defines.hpp"

namespace Behavior
{
  enum Mode { SEEK, CHASE };

  // Drives one tick of the state machine using the forward distance (cm).
  // A reading <= 0 is treated as "nothing in range".
  void Update(double frontDistance);

  Mode CurrentMode();

  void DebugPrint();
};
