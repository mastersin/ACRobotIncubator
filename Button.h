#ifndef _ACROBOT_BUTTON_H
#define _ACROBOT_BUTTON_H

#include "Utils.h"

namespace ACRobot {

class Button: public PollingInterface
{
  public:
    enum State { UNKNOWN, RELEASED, PRESSED };

    Button(uint8_t buttonPin): _buttonPin(buttonPin), _state(UNKNOWN), _wasReleased(false), _wasPressed(false), _wasClicked(false)
    {
      pinMode(buttonPin, INPUT_PULLUP);
    }

    bool poll()
    {
      register uint8_t state = digitalRead(_buttonPin);
      if (state == HIGH) {
        if (_state == PRESSED)
          _wasClicked = true;
        _wasReleased = true;
        _wasPressed = false;
        _state = RELEASED;
      } else {
        _wasPressed = true;
        _state = PRESSED;
      }
      return _state != RELEASED;
    }
    bool isReleased() {
      return _state == RELEASED;
    }
    bool isPressed() {
      return _state == PRESSED;
    }

    bool wasPressed() {
      if (_wasPressed && _wasReleased) {
        _wasReleased = false;
        return true;
      }
      return false;
    }
    bool isClicked() {
      if (_wasClicked) {
        _wasClicked = false;
        return true;
      }
      return false;
    }
    void reset()
    {
      _wasClicked = false;
      _wasReleased = false;
      _wasPressed = false;
    }

  private:

    uint8_t _buttonPin;
    State _state;
    bool _wasReleased;
    bool _wasPressed;
    bool _wasClicked;
};

} // ACRobot namespace

#endif

