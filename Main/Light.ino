#include "Light.hpp"

Light::Light(int pin, bool rev)
{
  this->pin = pin;
  this->rev = rev;
}

bool Light::getCol() {
  bool wasWhite = lastCol == WHITE;
  if(millis() - lastRefresh > LIGHT_REFRESH)
  {
    lastCol = digitalRead(pin);
    if(rev) lastCol = !lastCol;
    lastRefresh = millis();
  }

  if(!wasWhite && lastCol == WHITE)
    lastWhite = millis();

  return lastCol;
}
