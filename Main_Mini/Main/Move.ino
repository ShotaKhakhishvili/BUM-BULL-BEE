#include "Move.hpp"

void Move::Init()
{
  L.Init();
  R.Init();
}

void Move::DebugPrint()
{
  Serial.print("Left  ");
  L.DebugPrint();
  Serial.print("Right ");
  R.DebugPrint();
}

void Move::RotateOnPoint(bool dir, int str)
{
  L.setRotation(dir, str);
  R.setRotation(!dir, str);
}

void Move::RotateOnSide(bool dir, int str)
{
  if(dir)
  {
    L.setRotation(BACKWARD, str);
    R.setRotation(FORWARD, 0);
  }
  else
  {
    L.setRotation(FORWARD, 0);
    R.setRotation(BACKWARD, str);
  }
}
void Move::Walk(bool dir, int str)
{
  L.setRotation(dir, str);
  R.setRotation(dir, str);
}

void Move::SlideFwd(bool dir, int str, float coef)
{
  if(!dir)
  {
    L.setRotation(FORWARD, str);
    R.setRotation(FORWARD, str * coef);
  }
  else
  {
    L.setRotation(FORWARD, str * coef);
    R.setRotation(FORWARD, str);
  }
}

void Move::SlideBwd(bool dir, int str, float coef)
{
  if(dir)
  {
    L.setRotation(BACKWARD, str);
    R.setRotation(BACKWARD, str * coef);
  }
  else
  {
    L.setRotation(BACKWARD, str * coef);
    R.setRotation(BACKWARD, str);
  }
}

