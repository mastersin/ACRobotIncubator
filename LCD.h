#ifndef _ACROBOT_LCD_H
#define _ACROBOT_LCD_H

#include <LiquidCrystal.h>
#include "ADCKey.h"

namespace ACRobot {

enum LCD_type
{
  LCD_1602A_STD,
  LCD_1602A_KEYPAD_SHIELD,
};

template<LCD_type type>
class LCD {};

template<>
class LCD<LCD_1602A_STD>: public LiquidCrystal
{
    // const uint8_t resetPin  = 12;
    // const uint8_t enablePin = 11;
    // const uint8_t data4Pin  = 5;
    // const uint8_t data5Pin  = 4;
    // const uint8_t data6Pin  = 3;
    // const uint8_t data7Pin  = 2;

  public:
    LCD():
      LiquidCrystal (12, 11, 5, 4, 3, 2)
    {
      begin(16, 2);
    }
};

template<>
class LCD<LCD_1602A_KEYPAD_SHIELD>: public LiquidCrystal, public ADCKey
{
    // const uint8_t resetPin  = 8;
    // const uint8_t enablePin = 9;
    // const uint8_t data4Pin  = 4;
    // const uint8_t data5Pin  = 5;
    // const uint8_t data6Pin  = 6;
    // const uint8_t data7Pin  = 7;
    // const uint8_t adcPin    = A0;

  public:
    LCD():
      LiquidCrystal (8, 9, 4, 5, 6, 7),
      ADCKey(A0)
    {
      begin(16, 2);
    }
};

} // ACRobot namespace

#endif

