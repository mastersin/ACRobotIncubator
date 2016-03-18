#include "Interval.h"

namespace ACRobot {

bool Interval::poll(unsigned long ms)
{
  if (ms - _last >= _interval) {
    _last = ms;
    return true;
  }

  return false;
}

} // ACRobot namespace

