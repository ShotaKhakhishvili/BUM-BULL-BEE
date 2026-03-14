#include "Move.hpp"

void Move::rotateOnPoint(bool dir, int str)
{
  L.setRotation(dir, str);
  R.setRotation(!dir, str);
}

void Move::rotateOnSide(bool dir, int str)
{
  if(dir)
  {
    L.setRotation(0, str);
    R.setRotation(1, 0);
  }
  else
  {
    L.setRotation(1, 0);
    R.setRotation(0, str);
  }
}
void Move::walk(bool dir, int str)
{
  L.setRotation(dir, str);
  R.setRotation(dir, str);
}

void Move::slideFwd(bool dir, int str, float coef)
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

void Move::slideBwd(bool dir, int str, float coef)
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

