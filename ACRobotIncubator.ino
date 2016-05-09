#include "LCD.h"
#include "Interval.h"
#include "DHT.h"
#include "Config.h"
#include "Button.h"

#define DEBUG

using namespace ACRobot;

#define HOUR_SECS (60*60)
#define DAY_SECS (HOUR_SECS*24UL)

const uint8_t rightSideBtnPin = A0;
const uint8_t rightBtnPin     = A1;
const uint8_t downBtnPin      = A2;
const uint8_t upBtnPin        = A3;
const uint8_t leftBtnPin      = A4;
const uint8_t leftSideBtnPin  = A5;

const uint8_t lightPin = 13;
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
uint32_t total_secs;
uint8_t total_days;

enum IntervalList {
  GLOBAL = 0,
  SCREEN = 1,
  SECOND = 2,
  CONFIG = 3,
  EGG_TURNING = 4,
};

const unsigned long GLOBAL_INTERVAL = 50;
const unsigned long SCREEN_INTERVAL = 500;
const unsigned long SECOND_INTERVAL = 1000;
const unsigned long CONFIG_INTERVAL = 10000;

const uint8_t NUMBER_OF_INTERVALS = 5;
Intervals<NUMBER_OF_INTERVALS> intervals;

enum SecondsList {
  VENTILATION_STARTING  = 0,
  EGG_TURN_STARTING     = 1,
};

const uint8_t NUMBER_OF_SECONDS_INTERVALS = 2;
Intervals<NUMBER_OF_SECONDS_INTERVALS> seconds_intervals;

bool heat_is_on      = false;
bool fan_continue_on = false;

bool ventilation_on  = false;
bool egg_turning_on  = false;

SwitchButton<RattlePressButton> modeSw(rightBtnPin);
DigitalSwitch<RattlePressButton> lightSw(rightSideBtnPin, lightPin);
DigitalButton<Button> eggBtn(leftSideBtnPin, eggPin);
RattlePressButton upBtn(upBtnPin);
RattlePressButton downBtn(downBtnPin);

enum SelectMode
{
  NONE_MODE,
  STAGE_MODE,
  DAY_MODE,
  HOUR_MODE,
  MINUTE_MODE,
  MAX_SELECT_MODE
};

SequenceButton<RattlePressButton> selectBtn(leftBtnPin, MAX_SELECT_MODE);
uint8_t selectMode = NONE_MODE;

Interval fan_continue = 5; // 5 seconds after heating
Interval ventilation;
Interval egg_turning;

DHT dht(dhtPin, DHT11);

uint32_t current_millis;
int poll()
{
  current_millis = millis();

  modeSw.poll();
  upBtn.poll();
  downBtn.poll();
  selectBtn.poll();

  lightSw.poll();
  eggBtn.poll();
  config.poll();

  return intervals.status(current_millis);
}


static uint8_t stage = 0;
static uint32_t counter = 0;
static const char *str = "Init";

static FlipFlop dhtFlipFLop;
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
  total_secs = _duration;
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

  pinMode(fanPin, OUTPUT);
  pinMode(hotPin, OUTPUT);
  pinMode(eggPin, OUTPUT);

  pinMode(vntPin, OUTPUT);

  dht.begin();

  pinMode(upBtnPin,        INPUT_PULLUP);
  pinMode(downBtnPin,      INPUT_PULLUP);
  pinMode(rightBtnPin,     INPUT_PULLUP);
  pinMode(leftBtnPin,      INPUT_PULLUP);
  pinMode(leftSideBtnPin,  INPUT_PULLUP);

#ifdef DEBUG
  Serial.println("start");
#endif
}

inline void regulator()
{
  if (temp >= status[stage].max_temp) {
    digitalWrite(hotPin, LOW);
    digitalWrite(fanPin, LOW);
    if (heat_is_on)  // after 5 seconds
      fan_continue_on = true;
    heat_is_on = false;
  }
  if (temp < status[stage].min_temp) {
    digitalWrite(hotPin, HIGH);
    digitalWrite(fanPin, HIGH);
    heat_is_on = true;
  }
}

bool next_stage()
{
  bool retval = false;
  if (stage < (NumberOfStages-1)) {
    counter = durations[stage];
    retval = true;
  }
  return retval;
}

bool prev_stage()
{
  bool retval = false;
  if (stage > 0) {
    counter = 0;
    retval = true;
  }
  if (stage > 1)
    counter = durations[stage-2];
  return retval;
}

bool next_day()
{
  bool retval = false;
  if (counter < (total_secs - DAY_SECS)) {
    counter += DAY_SECS;
    retval = true;
  }
  return retval;
}

bool prev_day()
{
  bool retval = false;
  if (counter > DAY_SECS) {
    counter -= DAY_SECS;
    retval = true;
  }
  return retval;
}


bool next_hour()
{
  bool retval = false;
  if (counter < (total_secs - HOUR_SECS)) {
    counter += HOUR_SECS;
    retval = true;
  }
  return retval;
}

bool prev_hour()
{
  bool retval = false;
  if (counter > HOUR_SECS) {
    counter -= HOUR_SECS;
    retval = true;
  }
  return retval;
}


bool next_minute()
{
  bool retval = false;
  if (counter < (total_secs - 60)) {
    counter += 60;
    retval = true;
  }
  return retval;
}

bool prev_minute()
{
  bool retval = false;
  if (counter > 60) {
    counter -= 60;
    retval = true;
  }
  return retval;
}

bool upCommand(uint8_t mode)
{
  bool retval = false;
  switch (mode) {
  case STAGE_MODE:
    retval = next_stage();
    break;
  case DAY_MODE:
    retval = next_day();
    break;
  case HOUR_MODE:
    retval = next_hour();
    break;
  case MINUTE_MODE:
    retval = next_minute();
    break;
  }
  return retval;
}

bool downCommand(uint8_t mode)
{
  bool retval = false;
  switch (mode) {
  case STAGE_MODE:
    retval = prev_stage();
    break;
  case DAY_MODE:
    retval = prev_day();
    break;
  case HOUR_MODE:
    retval = prev_hour();
    break;
  case MINUTE_MODE:
    retval = prev_minute();
    break;
  }
  return retval;
}

void buttons()
{
  static bool prevModeSw = false;
  selectMode = selectBtn.next();
  if(modeSw) {
    if (!prevModeSw) {
      prevModeSw = true;
      selectBtn.reset();
      selectMode = NONE_MODE;
    }
    if(upBtn) {
      if(upCommand(selectMode))
        update_settings();
    }
    if(downBtn) {
      if(downCommand(selectMode))
        update_settings();
    }
  } else {
    if (prevModeSw) {
      prevModeSw = false;
      config(settings);
    }
  }
}

void logic()
{
  regulator();
  buttons();

  // humidity by motor
  if (fan_continue_on) {
    if (fan_continue.poll(counter))
      fan_continue_on = false;
    else {
#ifdef DEBUG
      Serial.println("post_fan = HIGH");
#endif
      digitalWrite(fanPin, HIGH);
    }
  }
  if (!fan_continue_on) {
    digitalWrite(fanPin, LOW);
  }

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
    if (egg_turning.poll(current_millis))
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

void printNumber(int value)
{
  if (value < 10)
    lcd.print('0');
  lcd.print(value);
}

void printTime(unsigned long seconds)
{
  int second = seconds % 60;
  int minute = (seconds / 60) % 60;
  int hour = (seconds / HOUR_SECS) % 24;
  int day = (seconds / DAY_SECS) % 99;
  printNumber(day);
  lcd.print(':');
  printNumber(hour);
  lcd.print(':');
  printNumber(minute);
  lcd.print(':');
  printNumber(second);
}

char modeSymbol(uint8_t mode)
{
  char symbol = ' ';
  switch (mode) {
  case STAGE_MODE:
    symbol = 'S';
    break;
  case DAY_MODE:
    symbol = 'd';
    break;
  case HOUR_MODE:
    symbol = 'h';
    break;
  case MINUTE_MODE:
    symbol = 'm';
    break;
  }
  return symbol;
}

void showTime()
{
  lcd.clear();
  lcd.print(day());
  lcd.setCursor(2, 0);
  lcd.print('|');
  printTime(counter);
  lcd.setCursor(14, 0);
  lcd.print('|');
  lcd.print(stage);
  lcd.setCursor(0, 1);
  lcd.print(days_left());
  lcd.setCursor(2, 1);
  lcd.print('|');
  printTime(total_secs - counter);
  lcd.setCursor(14, 1);
  lcd.print('|');
  lcd.print(modeSymbol(selectMode));
}

void showStatus()
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

void screen()
{
  if (modeSw)
    showTime();
  else
    showStatus();
}

void timer()
{
  counter++;

#ifdef DEBUG
  Serial.print("counter = ");
  Serial.println(counter);
#endif

  switch (seconds_intervals.status(counter))
  {
    case VENTILATION_STARTING:
      ventilation_on = true; // check for status
#ifdef DEBUG
      Serial.println("ventilation_on");
#endif
      ventilation.reset(counter);
      break;
    case EGG_TURN_STARTING:
      egg_turning_on = true; // check for status
#ifdef DEBUG
      Serial.println("egg_turning_on");
#endif
      egg_turning.reset(counter);
      break;
  }

  if (dhtFlipFLop) {
    float _hum = dht.readHumidity();
    if (!isnan(_hum))
      hum = _hum;
  } else {
    float _temp = dht.readTemperature();
    if (!isnan(_temp))
      temp = _temp;
  }

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

