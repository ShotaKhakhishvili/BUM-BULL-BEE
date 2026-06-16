#pragma once
#include "Defines.hpp"

namespace Behavior
{
  enum Mode { SEEK, CHASE };

  void Update(double frontDistance);

  Mode CurrentMode();

  void DebugPrint();
};
