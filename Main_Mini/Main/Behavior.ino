#include "Behavior.hpp"
#include "Move.hpp"

namespace Behavior
{
  static Mode mode = SEEK;

  static bool inRange(double frontDistance, double limit)
  {
    return frontDistance > 0 && frontDistance <= limit;
  }

  void Update(double frontDistance)
  {
    switch (mode)
    {
      case SEEK:
        if (inRange(frontDistance, DETECT_DISTANCE))
        {
          mode = CHASE;
          Move::Walk(FORWARD, CHASE_SPEED);
        }
        else
        {
          Move::RotateOnPoint(SEEK_DIR, SEEK_SPEED);
        }
        break;

      case CHASE:
        if (inRange(frontDistance, LOSE_DISTANCE))
        {
          Move::Walk(FORWARD, CHASE_SPEED);
        }
        else
        {
          mode = SEEK;
          Move::RotateOnPoint(SEEK_DIR, SEEK_SPEED);
        }
        break;
    }
  }

  Mode CurrentMode()
  {
    return mode;
  }

  void DebugPrint()
  {
    Serial.print("Behavior | Mode: ");
    Serial.println(mode == SEEK ? "SEEK" : "CHASE");
  }
};
