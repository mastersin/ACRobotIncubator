#include "LCD.h"
#include "Interval.h"
#include "DHT.h"
#include "Config.h"

using namespace ACRobot;

const uint8_t doorPin = A5;
const uint8_t eggPin = 10;
const uint8_t fanPin = 9;
const uint8_t hotPin = 8;
const uint8_t dhtPin = 6;

const char   *Name = "ACRobotI";
const uint8_t Version = 1;

struct Settings
{
  uint8_t stage;
  int8_t adjust_t;
  int8_t adjust_h;
  uint32_t time_offset;
};

enum Stages
{
  Init,
  Period1,
  Period2,
  Period3,
  Period4,
};

Settings settings = { Init, 0, 0, 0 };

Config<Settings> config(Name, Version, settings);

LCD<LCD_1602A_STD> lcd;

enum State {
  GLOBAL = 0,
  SCREEN = 1,
  BLINK  = 2
};

const unsigned long GLOBAL_INTERVAL = 50;
const unsigned long SCREEN_INTERVAL = 250;
const unsigned long BLINK_INTERVAL  = 1000;

const uint8_t NUMBER_OF_INTERVALS = 3;
Intervals<NUMBER_OF_INTERVALS> intervals;

DHT dht(dhtPin, DHT11);

int poll()
{
  config.poll();
  return intervals.poll();
}

void setup()
{
  lcd.print("Wait for start");
  settings = config();

  intervals[GLOBAL] = GLOBAL_INTERVAL;
  intervals[SCREEN] = SCREEN_INTERVAL;
  intervals[BLINK]  = BLINK_INTERVAL;

  lcd.clear();
  lcd.print("Starting");

  pinMode(doorPin, OUTPUT);
  digitalWrite(doorPin, HIGH);

  pinMode(fanPin, OUTPUT);
  pinMode(hotPin, OUTPUT);
  pinMode(eggPin, OUTPUT);

  dht.begin();
}

static uint8_t cnt = 0;
static const char *str = "Init";

static float h = 0.0;
static float t = 0.0;

void logic()
{
  if(t > 40.0) {
    digitalWrite(hotPin, LOW);
    digitalWrite(fanPin, LOW);
  }
  else {
    digitalWrite(hotPin, HIGH);
    digitalWrite(fanPin, HIGH);
  }
}

void screen()
{
  lcd.clear();
  lcd.print(str);
  lcd.setCursor(8, 0);
  lcd.print(h);
  lcd.print('%');
  lcd.setCursor(8, 1);
  lcd.print(t);
  lcd.print(" *C");
}

void blink()
{
  static bool fanState = false;
  if (fanState)
  {
    fanState = false;
    //digitalWrite(fanPin, LOW);
    //digitalWrite(eggPin, LOW);
  } else {
    fanState = true;
    //digitalWrite(fanPin, HIGH);
    //digitalWrite(eggPin, HIGH);
  }

  float _h = dht.readHumidity();
  float _t = dht.readTemperature();

  if(!isnan(_t))
    t = _t;
  if(!isnan(_h))
    h = _h;
}

void loop()
{
  switch (poll())
  {
    case GLOBAL:
      logic();
      break;
    case SCREEN:
      screen();
      break;
    case BLINK:
      blink();
      break;
  }
}

