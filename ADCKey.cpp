#include "ADCKey.h"

namespace ACRobot {

uint8_t ADCKey::readButtons()
{
  int keyIn = analogRead(_keyPin);
  if (keyIn > 1000) return NoneKey;

  if (keyIn < RightIn) return RightKey;
  if (keyIn < UpIn) return UpKey;
  if (keyIn < DownIn) return DownKey;
  if (keyIn < LeftIn) return LeftKey;
  if (keyIn < SelectIn) return SelectKey;

  return NoneKey;
}

} // ACRobot namespace

