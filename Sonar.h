#ifndef _ACROBOT_SONAR_H
#define _ACROBOT_SONAR_H

#include "Utils.h"

namespace ACRobot {

class Sonar: public PollingInterface
{
  public:
    Sonar(uint8_t trigPin, uint8_t echoPin, int maxDist = 1000):
      _trigPin(trigPin), _echoPin(echoPin), _value(0), _maxDist(maxDist)
    {
      pinMode(_trigPin, OUTPUT); 
      pinMode(_echoPin, INPUT); 
    }

    int operator() () { return _value; }
    void poll() { _value = readSensor(); }

  protected:
    uint8_t _trigPin;
    uint8_t _echoPin;
    int _value;

    int readSensor();

  public:
    const int _maxDist;
};

} // ACRobot namespace

#endif

