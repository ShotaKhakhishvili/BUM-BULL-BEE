#include "MoveHandler.hpp"
#include "Sonic.hpp"
#include "Extra.hpp"
#include "Move.hpp"

void MoveHandler::handleSeek()
{
  if(seekMode == SeekMode::SEEK_RIGHT)
  {
    Move::rotateOnPoint(RIGHT, 255);
    seekRandDir = RIGHT;
  }
  else if(seekMode == SeekMode::SEEK_LEFT)
  {
    Move::rotateOnPoint(LEFT, 255);
    seekRandDir = LEFT;
  }
  else
    Move::rotateOnPoint(seekRandDir, 255);
}

void MoveHandler::handleChase()
{
  Move::walk(FORWARD, 255);
}

void MoveHandler::handleAdjust()
{
  switch(adjMode)
  {
    case ADJ_FRONT:
      Move::walk(BACKWARD, 255);
      return;
    case ADJ_BACK:
      Move::walk(FORWARD, 255);
      return;
    case ADJ_RIGHTS:
    case ADJ_BR:
      Move::slideFwd(LEFT, 255, 0.5);  
      return;  
    case ADJ_LEFTS:
    case ADJ_BL:
      Move::slideFwd(RIGHT, 255, 0.5);
      return;
    case ADJ_FR:
      Move::slideBwd(LEFT, 255, 0.5);
      return;
    case ADJ_FL:
      Move::slideBwd(RIGHT, 255, 0.5);
      return;
    case ADJ_NONE:
      return;
    default:
      Move::walk(FORWARD, 0);
      return;
  }
}

void MoveHandler::updateMove()
{
  switch(mode)
  {
    case MODE_ADJUST:
      handleAdjust();
      return;
    case MODE_SEEK:
      handleSeek();
      return;
    default:
      handleChase();
      return;
  }
}