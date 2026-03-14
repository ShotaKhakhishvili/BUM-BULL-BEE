#include "Extra.hpp"
#include <string>

void Extra::switchToChase()
{
  mode = MODE_CHASE;
  adjMode = ADJ_NONE;
  seekMode = SeekMode::SEEK_NONE;
}

void Extra::switchToSeek(SeekMode newSeekMode)
{
  mode = MODE_SEEK;
  adjMode = ADJ_NONE;
  seekMode = newSeekMode;
}

void Extra::switchToAdjust(int newAdjustMode)
{
  mode = MODE_ADJUST;
  adjMode = newAdjustMode;
  seekMode = SeekMode::SEEK_NONE;

  if(adjMode == ADJ_NONE)return;

  if(adjMode == ADJ_FR || adjMode == ADJ_BR || adjMode == ADJ_RIGHTS)
    seekRandDir = LEFT;
  else 
    seekRandDir = RIGHT;
}

void Extra::printAllLightInfo(int sensor)
{
  std::string answer = "SENSOR " + std::to_string(sensor) + "\n";
  answer += "Color In Bool: ";
  answer += lights[sensor]->getCol() ? "1" : "0";
  answer += "\n";

  answer += "Color In White: ";
  answer += lights[sensor]->getCol() == WHITE ? "WHITE" : "BLACK";
  answer += "\n";

  Serial.println(answer.c_str());
}

void Extra::printAllSonicInfo(int sensor)
{
  std::string answer = "SENSOR " + std::to_string(sensor) + "\n";
  answer += "Dist In Int: ";
  answer += std::to_string(Sonic::getDist(sensor));
  answer += "\n";

  answer += "Can sonic see: ";
  answer += Sonic::canSonicSee(sensor) ? "YES" : "NO";
  answer += "\n";

  Serial.println(answer.c_str());
}

int Extra::getAdjustModeTime(int move)
{
  switch(move)
  {
    case ADJ_FRONT:
    case ADJ_BACK: 
      return 300;

    case ADJ_FR:
    case ADJ_FL:
    case ADJ_BR:
    case ADJ_BL:
      return 500;

    case ADJ_RIGHTS:
    case ADJ_LEFTS:
      return 100;

    default:
      return 0;
  }
}

void Extra::clearSerialMonitor()
{
  for(int i = 0; i < 30; i++)
    Serial.println();
}