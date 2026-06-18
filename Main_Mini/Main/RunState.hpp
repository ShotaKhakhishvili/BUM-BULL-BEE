#pragma once

// Interrupt-driven start (D2) / stop (D1) gate. ISRs set a flag; loop()
// gates all behavior on IsRunning().
namespace RunState
{
  void Init();

  bool IsRunning();

  void DebugPrint();
};
