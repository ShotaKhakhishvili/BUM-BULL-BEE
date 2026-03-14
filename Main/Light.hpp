#pragma once
#include <utility>

class Light
{
private:

  int pin;

  bool lastCol = BLACK;
  unsigned long lastRefresh = 0;
  bool rev = false;

public:
  unsigned long lastWhite = 0;

  Light(int pin, bool rev);

  bool getCol();
};

