#pragma once
#include "Light.hpp"
#include "Defines.hpp"

namespace ModeHandler
{
  int getLineCheck();

  void handleAdjust(int newAdjMode);
  void handleSeek();
  void handleChase();
  
  void updateMode();
};
