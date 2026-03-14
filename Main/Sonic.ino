#include "Sonic.hpp"

static int cachedDist[] = {0, 0};
static unsigned int lastCheck[] = {0, 0};

int Sonic::getDist(int son)
{
  if(son < 0 || son > 1) return -1;

  if(millis() - lastCheck[son] > SONAR_REFRESH)
  {
    lastCheck[son] = millis();
    cachedDist[son] = sons[son].ping_cm(MAX_DISTANCE);
  }

  return cachedDist[son];
}

bool Sonic::canSonicSee(int son)
{
  return getDist(son) > 0;
}

SeekMode Sonic::seek()
{
  bool bLeft = canSonicSee(0);
  bool bRight = canSonicSee(1);

  SeekMode answer = SeekMode::SEEK_NONE;

  if(bLeft && bRight)
    answer = SeekMode::SEEK_BOTH;
  else if(bLeft)
    answer = SeekMode::SEEK_LEFT;
  else if(bRight)
    answer = SeekMode::SEEK_RIGHT;

  return answer;
}

