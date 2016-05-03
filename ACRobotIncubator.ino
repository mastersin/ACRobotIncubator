#include "LCD.h"
#include "Interval.h"
#include "DHT.h"
#include "Config.h"

#define DEBUG

using namespace ACRobot;

#define HOUR_SECS (60*60)
#define DAY_SECS (HOUR_SECS*24)

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
  Stage2_1,
  Stage2_2,
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
  uint32_t turn_interval;
  uint32_t turn_duration;

  bool     vent;      // ventilation
  uint32_t vent_interval;
  uint32_t vent_duration;

  const char *desc;
};

Status status[] = {
  { 0,             37.5, 38.5, 60.0, 70.0,  false, 0,             0,    false, 0,             0,   "Init"     },
  { DAY_SECS * 4,  38.5, 38.5, 80.0, 85.0,  false, 0,             0,    false, 0,             0,   "Stage1"   },
  { DAY_SECS * 8,  37.9, 38.3, 60.0, 65.0,  true,  HOUR_SECS * 4, 2000, true,  HOUR_SECS * 4, 60,  "Stage2_1" },
  { DAY_SECS * 3,  37.9, 38.3, 60.0, 65.0,  true,  HOUR_SECS * 4, 2000, false, 0            , 0,   "Stage2_2" },
  { DAY_SECS * 3,  37.5, 37.5, 80.0, 90.0,  false, 0,             0,    true,  HOUR_SECS * 4, 360, "Stage3"   },
  { DAY_SECS * 3,  37.5, 37.5, 80.0, 90.0,  false, 0,             0,    true,  HOUR_SECS * 4, 360, "Stage4"   },
};

uint32_t durations[NumberOfStages];
uint8_t total_days;

enum IntervalList {
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

enum SecondsList {
  VENTILATION    = 0,
  EGG_TURNING    = 1,
};

unsigned long VENTILATION_INTERVAL = 60 ;
unsigned long EGG_TURNING_INTERVAL = 30 ;

const uint8_t NUMBER_OF_SECONDS_INTERVALS = 2;
Intervals<NUMBER_OF_SECONDS_INTERVALS> seconds_intervals;

bool ventilation_on = false;
bool egg_turning_on = false;

Interval ventilation = 5;
Interval egg_turning = 5;

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

void update_settings()
{
  for (uint8_t i = 0; i < NumberOfStages; i++)
  {
    if (counter < durations[i]) {
      stage = i;
      break;
    }
  }
  settings.stage = stage;
  settings.time_offset = counter;

  if (status[stage].turn_interval != seconds_intervals[EGG_TURN_STARTING])
    seconds_intervals[EGG_TURN_STARTING] = status[stage].turn_interval;
  if (status[stage].turn_duration != egg_turning)
    egg_turning = status[stage].turn_duration;

  if (status[stage].vent_interval != seconds_intervals[VENTILATION_STARTING])
    seconds_intervals[VENTILATION_STARTING] = status[stage].vent_interval;
  if (status[stage].vent_duration != ventilation)
    ventilation = status[stage].vent_duration;
}

void setup()
{
#ifdef DEBUG
  Serial.begin(9600);
#endif

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

  pinMode(vntPin, OUTPUT);

  dht.begin();

#ifdef DEBUG
  Serial.println("start");
#endif
}

void logic()
{
  if (temp >= status[stage].max_temp) {
    digitalWrite(hotPin, LOW);
    digitalWrite(fanPin, LOW); // after 5 seconds
  }
  if (temp < status[stage].min_temp) {
    digitalWrite(hotPin, HIGH);
    digitalWrite(fanPin, HIGH);
  }
  // humidity by motor

  if (ventilation_on) {
    if (ventilation.poll(counter))
      ventilation_on = false;
    else {
#ifdef DEBUG
      Serial.println("ventilation = HIGH");
#endif
      digitalWrite(vntPin, HIGH);
    }
  }
  if (!ventilation_on) {
    digitalWrite(vntPin, LOW);
  }

  if (egg_turning_on) {
    if (egg_turning.poll(counter))
      egg_turning_on = false;
    else {
#ifdef DEBUG
      Serial.println("egg_turning = HIGH");
#endif
      digitalWrite(eggPin, HIGH);
    }
  }
  if (!egg_turning_on) {
    digitalWrite(eggPin, LOW);
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

void printMeasure(float value, char type)
{
  if (value < 10.0)
    lcd.print(' ');
  lcd.print(value, 1);
  lcd.print(type);
}

void screen()
{
  float tmp;
  lcd.clear();
  lcd.print(day());
  lcd.setCursor(2, 0);
  lcd.print('|');
  printMeasure(status[stage].max_temp, 'C');
  lcd.setCursor(8, 0);
  lcd.print('|');
  printMeasure(temp, 'C');
  lcd.setCursor(14, 0);
  lcd.print('|');
  lcd.print(stage);
  lcd.setCursor(0, 1);
  lcd.print(days_left());
  lcd.setCursor(2, 1);
  lcd.print('|');
  printMeasure(status[stage].max_hum, '%');
  lcd.setCursor(8, 1);
  lcd.print('|');
  printMeasure(hum, '%');
  lcd.setCursor(14, 1);
  lcd.print('|');
}

void timer()
{
  counter++;

#ifdef DEBUG
  Serial.print("counter = ");
  Serial.println(counter);
#endif

  switch (seconds_intervals.poll(counter))
  {
    case VENTILATION:
      ventilation_on = true; // check for status
#ifdef DEBUG
      Serial.println("ventilation_on");
#endif
      ventilation.reset(counter);
      break;
    case EGG_TURNING:
      egg_turning_on = true;; // check for status
#ifdef DEBUG
      Serial.println("egg_turning_on");
#endif
      egg_turning.reset(counter);
      break;
  }

  float _hum = dht.readHumidity();
  float _temp = dht.readTemperature();

  if (!isnan(_temp))
    temp = _temp;
  if (!isnan(_hum))
    hum = _hum;

  settings.time_offset = counter;
  update_settings();
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
      config(settings);
      break;
  }
}

