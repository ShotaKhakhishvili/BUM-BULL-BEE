#pragma once

class Wheel
{
private:
  bool dir = false;
  int forward,backward;

public:

  Wheel(int forwardIn, int backwardIn);

  void setRotation(bool dir, int str);
};
