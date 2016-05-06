#include "Interval.h"

namespace ACRobot {

bool Interval::poll(unsigned long ms, bool test)
{
  if (ms - _last >= _interval) {
    if(!test)
      reset(ms);
    return true;
  }

  return false;
}

} // ACRobot namespace

