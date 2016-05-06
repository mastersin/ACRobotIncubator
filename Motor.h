#ifndef _ACROBOT_MOTOR_H
#define _ACROBOT_MOTOR_H

#include "Utils.h"

namespace ACRobot {

class DCMotorInterface: public PollingInterface
{
  public:
    DCMotorInterface(): _new_power(0), _power(0) {}
    void setPower(uint8_t power) { _power = power; }

  protected:
    uint8_t _new_power;
    uint8_t _power;
};

class DCMotor: public DCMotorInterface
{
  public:
    DCMotor(uint8_t directPin, uint8_t pwmPin): _directPin(directPin), _pwmPin(pwmPin)
    {
      pinMode(directPin, OUTPUT);
      pinMode(pwmPin, OUTPUT);

      clearDigitalPin(directPin);
      clearDigitalPin(pwmPin);
    }

    void setPower(uint8_t power) { _new_power = power; poll(); };
    uint8_t& operator= (uint8_t power) { return _new_power = power; };

    bool poll();

  private:

    uint8_t _directPin;
    uint8_t _pwmPin;
};

} // ACRobot namespace

#endif

