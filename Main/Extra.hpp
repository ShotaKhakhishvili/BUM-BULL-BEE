#pragma once

namespace Extra
{
  int getAdjustModeTime(int move);

  void printAllLightInfo(int sensor);
  void printAllSonicInfo(int sensor);
  void clearSerialMonitor();

  void switchToChase();
  void switchToSeek(SeekMode newSeekMode);
  void switchToAdjust(int newAdjustMode);
}