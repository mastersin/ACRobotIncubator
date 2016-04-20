#include "LCD.h"
#include "Interval.h"
#include "DHT.h"
#include "Config.h"

using namespace ACRobot;

#define DAY_SECS (60*60*24)

const uint8_t doorPin = A5;
const uint8_t eggPin = 10;
const uint8_t fanPin = 9;
const uint8_t hotPin = 8;
const uint8_t vntPin = 7;
const uint8_t dhtPin = 6;

const char   *Name = "ACRobotI";
const uint8_t Version = 1;

struct Settings
{
  uint8_t  stage;
  uint32_t time_offset;
};

enum Stages
{
  Init,
  Stage1,
  Stage2,
  Stage3,
  Stage4,
  NumberOfStages
};

Settings settings = { Init, 0 };

Config<Settings> config(Name, Version, settings);

LCD<LCD_1602A_STD> lcd;

struct Status
{
  uint32_t duration;

  float    min_temp;  // temperature
  float    max_temp;

  uint8_t  min_hum;   // humidity
  uint8_t  max_hum;

  bool     turn;      // turning
  uint32_t turn_time;

  bool     vent;      // ventilation
  uint32_t vent_time;

  const char *desc;
};

Status status[] = {
  { 0,             37.5, 38.5, 0.0,  0.0,  false, 0,            false, 0,            "Init" },
  { DAY_SECS * 4,  38.5, 38.5, 80.0, 85.0, false, 0,            false, 0,            "Stage1" },
  { DAY_SECS * 11, 37.9, 38.3, 60.0, 65.0, true,  DAY_SECS * 4, true,  DAY_SECS * 4, "Stage2" },
  { DAY_SECS * 3,  37.5, 37.5, 80.0, 90.0, false, 0,            true,  DAY_SECS * 4, "Stage3" },
  { DAY_SECS * 3,  37.5, 37.5, 80.0, 90.0, false, 0,            true,  DAY_SECS * 4, "Stage4" },
};

uint32_t durations[NumberOfStages];
uint8_t total_days;

enum State {
  GLOBAL = 0,
  SCREEN = 1,
  SECOND = 2,
  CONFIG = 3,
};

const unsigned long GLOBAL_INTERVAL = 50;
const unsigned long SCREEN_INTERVAL = 500;
const unsigned long SECOND_INTERVAL = 1000;
const unsigned long CONFIG_INTERVAL = 10000;

const uint8_t NUMBER_OF_INTERVALS = 4;
Intervals<NUMBER_OF_INTERVALS> intervals;

DHT dht(dhtPin, DHT11);

int poll()
{
  config.poll();
  return intervals.poll();
}


static uint8_t stage = 0;
static uint32_t counter = 0;
static const char *str = "Init";

static float hum = 0.0;
static float temp = 0.0;

void init_constants()
{
  uint32_t _duration = 0;
  for (uint8_t i = 0; i < NumberOfStages; i++)
  {
    _duration += status[i].duration;
    durations[i] = _duration;    
  }
  total_days = _duration / DAY_SECS;
}

void update_stage()
{
  for (uint8_t i = 0; i < NumberOfStages; i++)
  {
    if(counter < durations[i]) {
      stage = i;
      break;
    }
  }
}

void setup()
{
  lcd.print("Load settings");
  settings = config();

  intervals[GLOBAL] = GLOBAL_INTERVAL;
  intervals[SCREEN] = SCREEN_INTERVAL;
  intervals[SECOND] = SECOND_INTERVAL;
  intervals[CONFIG] = CONFIG_INTERVAL;

  counter = settings.time_offset;
  stage = settings.stage;

  init_constants();
  str = status[stage].desc;

  lcd.clear();
  lcd.print("Starting");

  pinMode(doorPin, OUTPUT);
  digitalWrite(doorPin, HIGH);

  pinMode(fanPin, OUTPUT);
  pinMode(hotPin, OUTPUT);
  pinMode(eggPin, OUTPUT);

  dht.begin();
}

void logic()
{
  if (temp >= status[stage].max_temp) {
    digitalWrite(hotPin, LOW);
    digitalWrite(fanPin, LOW);
  }
  if (temp < status[stage].min_temp) {
    digitalWrite(hotPin, HIGH);
    digitalWrite(fanPin, HIGH);
  }
}

uint8_t day()
{
  return counter / DAY_SECS;
}

uint8_t days_left()
{
  return total_days - day();
}

void screen()
{
  lcd.clear();
  lcd.print(day());
  lcd.setCursor(2, 0);
  lcd.print('|');
  lcd.print(status[stage].max_temp, 1);
  lcd.print('C');
  lcd.setCursor(9, 0);
  lcd.print('|');
  lcd.print(temp, 1);
  lcd.print('C');
  lcd.setCursor(14, 0);
  lcd.print('|');
  lcd.print(stage);
  lcd.setCursor(0, 1);
  lcd.print(days_left());
  lcd.setCursor(2, 1);
  lcd.print('|');
  lcd.print(status[stage].max_hum);
  lcd.print('%');
  lcd.setCursor(9, 0);
  lcd.print('|');
  lcd.print(hum);
  lcd.print('%');
  lcd.setCursor(14, 0);
  lcd.print('|');
}

void timer()
{
  static bool fanState = false;
  if (fanState)
  {
    fanState = false;
  } else {
    fanState = true;
  }

  float _hum = dht.readHumidity();
  float _temp = dht.readTemperature();

  if (!isnan(_temp))
    temp = _temp;
  if (!isnan(_hum))
    hum = _hum;
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
    case SECOND:
      timer();
      break;
    case CONFIG:
      config.poll();
      break;
  }
}

