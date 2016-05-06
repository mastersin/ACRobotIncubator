#ifndef _ACROBOT_BUTTON_H
#define _ACROBOT_BUTTON_H

#include "Utils.h"

namespace ACRobot {

class Button: public PollingInterface
{
  public:
    enum State { UNKNOWN, RELEASED, PRESSED };

    Button(uint8_t buttonPin): _buttonPin(buttonPin), _state(UNKNOWN)
    {
      pinMode(buttonPin, INPUT_PULLUP);
    }

    bool poll()
    {
      register uint8_t state = digitalRead(_buttonPin);
      _state = state == LOW ? PRESSED : RELEASED;
      return _state == PRESSED;
    }
    bool isReleased() const {
      return _state == RELEASED ;
    }
    bool isPressed() const {
      return _state == PRESSED;
    }

    operator bool() const { return isPressed(); }

  private:
    uint8_t _buttonPin;
    State _state;
};

class PressButton: public Button
{
  public:
    PressButton(uint8_t buttonPin):
      Button(buttonPin), _wasReleased(false), _wasPressed(false) {}

    bool poll()
    {
      if (Button::poll()) {
        _wasPressed = true;
        return _wasReleased; // _wasPressed && _wasReleased;
      }
      _wasReleased = true;
      _wasPressed = false;
      return false;
    }
    bool wasPressed() {
      if (_wasPressed && _wasReleased) {
        _wasReleased = false;
        return true;
      }
      return false;
    }
    void reset()
    {
      _wasReleased = false;
      _wasPressed = false;
    }

    operator bool() { return wasPressed(); }

  private:
    bool _wasReleased;
    bool _wasPressed;
};

class ClickButton: public Button
{
  public:
    ClickButton(uint8_t buttonPin):
      Button(buttonPin), _wasClicked(false) {}

    bool poll()
    {
      register bool pressed = isPressed();
      if (!Button::poll()) {
        if (pressed)
          _wasClicked = true;
      }
      return _wasClicked;
    }
    bool wasClicked() {
      if (_wasClicked) {
        _wasClicked = false;
        return true;
      }
      return false;
    }
    void reset()
    {
      _wasClicked = false;
    }

    operator bool() { return wasClicked(); }

  private:
    bool _wasClicked;
};

class SwitchButton: protected PressButton
{
  public:
    SwitchButton(uint8_t buttonPin):
      PressButton(buttonPin), _isOn(false) {}

    bool poll()
    {
      return PressButton::poll();
    }
    bool isOn() {
      if (wasPressed())
        _isOn = !_isOn;
      return _isOn;
    }
    void reset()
    {
      _isOn = false;
    }

    operator bool() { return isOn(); }

  private:
    bool _isOn;
};

} // ACRobot namespace

#endif

