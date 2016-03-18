#ifndef _ACROBOT_CONFIG_H
#define _ACROBOT_CONFIG_H

#include "Utils.h"
#include <EEPROM.h>

#define CONFIG_NAME_LEN 7

namespace ACRobot {

template<class Data>
class Config: public PollingInterface
{
    struct Buffer {
      uint8_t name[CONFIG_NAME_LEN];
      uint8_t version;
      Data data;
    } buffer;
  public:
    Config(const char *name, uint8_t version, const Data &default_config);
    bool poll();

    const Data &operator() () {
      return buffer.data;
    }
    void operator() (const Data &data) {
      return buffer.data = data;
      _updated = true;
    }

  private:

    uint8_t _updated;
};

template<class Data>
Config<Data>::Config(const char *name, uint8_t version, const Data &default_config):
  _updated(false)
{
  bool already_exists = true;
  uint8_t name_len = strlen(name);
  for (uint8_t i = 0; i < CONFIG_NAME_LEN; i++)
  {
    byte sym = name[i];
    if (i > name_len)
      sym = 0;
    buffer.name[i] = sym;
    if (EEPROM.read(i) != sym)
      already_exists = false;
  }
  buffer.version = version;
  if (EEPROM.read(CONFIG_NAME_LEN) != version)
    already_exists = false;
  if (!already_exists) {
    buffer.data = default_config;
    _updated;
  } else {
    uint8_t *pbuffer = (uint8_t *) &buffer;
    for (uint8_t i = 0; i < sizeof(buffer); i++)
      EEPROM.write(i + CONFIG_NAME_LEN + 1, pbuffer[i]);
  }
}

template<class Data>
bool Config<Data>::poll()
{
  if(_updated) {
    uint8_t *pbuffer = (uint8_t *) &buffer;
    for (uint8_t i = 0; i < sizeof(buffer); i++)
      EEPROM.write(i + CONFIG_NAME_LEN + 1, pbuffer[i]);
    _updated = false;
    return true;
  }
  return false;
}

} // ACRobot namespace

#endif

