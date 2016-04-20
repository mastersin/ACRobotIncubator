#include "Sonar.h"

namespace ACRobot {

int Sonar::readSensor()
{
  int duration;
  digitalWrite(_trigPin, LOW); 
  delayMicroseconds(2); 
  digitalWrite(_trigPin, HIGH); 
  delayMicroseconds(10); 
  digitalWrite(_trigPin, LOW); 

  duration = pulseIn(_echoPin, HIGH, _maxDist * 29 / 5); 
  return duration * 5 / 29;
}

} // ACRobot namespace

