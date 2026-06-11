#include "Wheel.hpp"

Wheel::Wheel(int forwardIn, int backwardIn) 
: forward(forwardIn), backward(backwardIn){}

void Wheel::setRotation(bool dir, int str)
{
  if(dir != this->dir)
  {
    analogWrite(forward, 0);
    analogWrite(backward, 0);
    delay(1);
  }
  if(dir)
  {
    analogWrite(forward, str);
    analogWrite(backward, 0);
  }
  else
  {
    analogWrite(forward, 0);
    analogWrite(backward, str);
  }
  this->dir = dir;
}
