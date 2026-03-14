#include "ModeHandler.hpp"
#include "Extra.hpp"

int ModeHandler::getLineCheck()
{
  // int vFR = (lights[0]->getCol() == WHITE) * ADJ_FR;
  // int vFL = (lights[1]->getCol() == WHITE) * ADJ_FL;
  // int vBR = (lights[2]->getCol() == WHITE) * ADJ_BR;
  // int vBL = (lights[3]->getCol() == WHITE) * ADJ_BL;

  bool wFR = lights[0]->getCol() == WHITE;
  bool wFL = lights[1]->getCol() == WHITE;
  bool wBR = lights[2]->getCol() == WHITE;
  bool wBL = lights[3]->getCol() == WHITE;

  int vFR = ((millis() - lights[0]->lastWhite) > WHITE_LIMIT) * wFR * ADJ_FR;
  int vFL = ((millis() - lights[1]->lastWhite) > WHITE_LIMIT) * wFL * ADJ_FL;
  int vBR = ((millis() - lights[2]->lastWhite) > WHITE_LIMIT) * wBR * ADJ_BR;
  int vBL = ((millis() - lights[3]->lastWhite) > WHITE_LIMIT) * wBL * ADJ_BL;

  return vFR + vFL + vBR + vBL;
}

void ModeHandler::handleAdjust(int newAdjMode)
{
  if(newAdjMode != adjMode && newAdjMode != ADJ_NONE) // If the new adjust mode differs from the previous adjust mode, we refresh the move time
  {
    lastMoveCheck = millis();
    moveTime = Extra::getAdjustModeTime(newAdjMode);
  }

  if(newAdjMode == ADJ_NONE && (millis() - lastMoveCheck >= moveTime))
    Extra::switchToSeek(Sonic::seek());
  else
    Extra::switchToAdjust(newAdjMode);
}

void ModeHandler::handleSeek()
{
  seekMode = Sonic::seek();

  if(seekMode == SeekMode::SEEK_BOTH) // If we are looking right at the opponent, we switch to the chasing mode
  {
    lastMoveCheck = millis();
    Extra::switchToChase();
  }
  else
    Extra::switchToSeek(seekMode);
}

void ModeHandler::handleChase()
{
  seekMode = Sonic::seek();

  if(!(seekMode == SeekMode::SEEK_BOTH)) // If we are not looking right at the opponent, we switch to the seeking mode
  {
    Extra::switchToSeek(seekMode);
  }
  else
    Extra::switchToChase();
}

void ModeHandler::updateMode()
{
  int newAdjMode = getLineCheck();

  int newMode = newAdjMode ? MODE_ADJUST : mode;

  // int newMode = mode;

  switch(newMode)
  {
    case MODE_ADJUST :
      handleAdjust(newAdjMode);
      return;
    case MODE_SEEK :
      handleSeek();
      return;
    default:
      handleChase();
      return;
  }
}
