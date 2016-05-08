#ifndef _ACROBOT_BUTTON_H
#define _ACROBOT_BUTTON_H

#include "Utils.h"
#include "Interval.h"

namespace ACRobot {

class Button: public ButtonInterface
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

    operator bool() { return isPressed(); }

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

class RattlePressButton: public PressButton
{
  static const unsigned long rattle_interval = 300;

  public:
    RattlePressButton(uint8_t buttonPin):
      PressButton(buttonPin), _interval(rattle_interval) {}

    bool poll() { return poll(millis()); }
    bool poll(unsigned long ms)
    {
      if(_interval.poll(ms, true))
        return PressButton::poll();
      return false;
    }
    bool wasPressed() {
      if (PressButton::wasPressed()) {
        _interval.reset();
        return true;
      }
      return false;
    }

    operator bool() { return wasPressed(); }

  private:
    Interval _interval;
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

template <class T>
class SwitchButton: public ButtonInterface
{
  public:
    SwitchButton(uint8_t buttonPin):
      _button(buttonPin), _isOn(false) {}

    bool poll()
    {
      return _button.poll();
    }
    bool isOn() {
      if (_button)
        _isOn = !_isOn;
      return _isOn;
    }
    void reset()
    {
      _isOn = false;
    }

    operator bool() { return isOn(); }

  protected:
    bool state() const { return _isOn; }

  private:
    T _button;
    bool _isOn;
};

template <class T>
class DigitalSwitch: public SwitchButton<T>
{
  public:
    DigitalSwitch(uint8_t buttonPin, uint8_t pin, bool inv = false):
      SwitchButton<T>(buttonPin), _pin(pin), _inv(inv)
    {
      pinMode(pin, OUTPUT);
    }

    bool poll()
    {
      SwitchButton<T>::poll();
      return set(SwitchButton<T>::isOn());
    }

    operator bool() { return SwitchButton<T>::state(); }

  private:
    bool set(bool state)
    {
      if(_inv)
        state = !state;
      digitalWrite(_pin, state ? HIGH : LOW);
      return state;
    }

    uint8_t _pin;
    bool _inv;
};

template <class T>
class DigitalButton
{
  public:
    DigitalButton(uint8_t buttonPin, uint8_t pin, bool inv = false):
      _button(buttonPin), _pin(pin), _state(false), _inv(inv)
    {
      pinMode(pin, OUTPUT);
    }

    bool poll()
    {
      _button.poll();
      return set(_button);
    }

    operator bool() { return _state; }

  private:
    bool set(bool state)
    {
      _state = state;
      if(_inv)
        state = !state;
      digitalWrite(_pin, state ? HIGH : LOW);
      return state;
    }

    T _button;
    uint8_t _pin;
    bool _state;
    bool _inv;
};

} // ACRobot namespace

#endif

