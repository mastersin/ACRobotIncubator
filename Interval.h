#ifndef _ACROBOT_INTERVAL_H
#define _ACROBOT_INTERVAL_H

#include "Utils.h"

namespace ACRobot {

class Interval: public PollingInterface
{
  public:
    Interval(): _interval(0), _last(0) {}
    Interval(long ms): _interval(ms), _last(0) {}
    bool poll() { return poll(millis()); }
    bool poll(unsigned long ms);
    void reset() { reset(millis()); }
    void reset(unsigned long ms) { _last = ms; }

    unsigned long& operator =(unsigned long ms) { return _interval = ms; }
    operator unsigned long () const { return _interval; }
    bool operator !() const { return _interval == 0; }

  private:

    unsigned long _interval;
    unsigned long _last;
};

template <int n>
class Intervals: public PollingInterface
{
  public:
    Intervals() {}
    int poll(unsigned long ms)
    {
      for (register int i = 0; i < n; i++)
      {
        if (!_intervals[i])
          continue;
        if (_intervals[i].poll(ms))
          return i;
      }
      return -1;
    }
    int poll()
    {
      return poll(millis());
    }

    Interval& operator[] (const int index)
    {
      return _intervals[index];
    }

  private:

    Interval _intervals[n];
};

} // ACRobot namespace

#endif

