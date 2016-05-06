#include "Motor.h"

namespace ACRobot {

bool DCMotor::poll()
{
  if (_power == _new_power)
    return false;

  register uint8_t power = _new_power;

  if (_new_power >= 0 && _power < 0)
    clearDigitalPin(_directPin);
  else if (_new_power < 0 && _power >= 0) {
    setDigitalPin(_directPin);
    power = -_new_power;
  }

  _power = _new_power;
  analogWrite(_pwmPin, power);

  return true;
}

} // ACRobot namespace

