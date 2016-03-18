#ifndef _ACROBOT_UTILS_H
#define _ACROBOT_UTILS_H

#if (ARDUINO >= 100)
  #include "Arduino.h"
#else
  #if defined(__AVR__)
    #include <avr/io.h>
  #endif
  #include "WProgram.h"
#endif

#define clear_bit(port,bit) \
    asm volatile (             \
        "cbi %0, %1" "\n\t" : : "I" (_SFR_IO_ADDR(port)), "I" (bit))

#define set_bit(port,bit) \
    asm volatile (             \
        "sbi %0, %1" "\n\t" : : "I" (_SFR_IO_ADDR(port)), "I" (bit))

#include <inttypes.h>

namespace ACRobot {

void setDigitalPin(uint8_t pin);
void clearDigitalPin(uint8_t pin);
uint8_t getDigitalPin(uint8_t pin);

class PollingInterface
{
  public:
    void poll() {};
};

inline void waitForStart(uint8_t pin)
{
  pinMode(pin, INPUT_PULLUP);
  while(getDigitalPin(pin) == HIGH);
  pinMode(pin, INPUT);
}

enum {
  NoneKey = 0,
  RightKey,
  LeftKey,
  UpKey,
  DownKey,
  SelectKey
};

} // ACRobot namespace

#endif
