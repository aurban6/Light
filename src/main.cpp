#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>
#include <Bounce2.h>
#include "main.h"
#include "config.h"

// Enable debug prints to serial monitor
//#define MY_DEBUG

#define MY_GATEWAY_ESP32
#define MY_PORT 5003
#define MY_GATEWAY_MAX_CLIENTS 2

#include <MySensors.h>

#define LED_TIMER_BIT 8
#define LED_BASE_FREQ 5000
#define RELAY_ON 0
#define RELAY_OFF 1

void setup()
{
  printHeader("SETUP");
  //setup for light
  for (int i = 0; i < LIGHT_SIZE; i++)
  {
    setupLight(lights[i]);
  }
  //setup for light dimmer
  for (int i = 0; i < LIGHT_DIMMER_SIZE; i++)
  {
    setupLightDimmer(lightDimmers[i]);
  }
  //setup for light RGB
  for (int i = 0; i < LIGHT_RGB_SIZE; i++)
  {
    setupLightRGB(lightRGBs[i]);
  }
  //setup for light RGB
  for (int i = 0; i < LIGHT_RGBW_SIZE; i++)
  {
    setupLightRGBW(lightRGBWs[i]);
  }
}

void setupLight(strLight_t &light)
{
  light.status = loadLevelState(light.sensor, 0);
  pinMode(light.pin, OUTPUT);
  digitalWrite(light.pin, light.status ? RELAY_ON : RELAY_OFF);
  printLight(light);
}

void setupLightDimmer(strLightDimmer_t &light)
{
  light.status = loadLevelState(light.sensor, 0);
  light.fadeTo = loadLevelState(light.sensor, 1);

  ledcSetup(light.channel, LED_BASE_FREQ, LED_TIMER_BIT);
  ledcAttachPin(light.pin, light.channel);

  if (!light.status)
  {
    light.dimValue = 0;
    light.fadeTo = 0;
  }
  printLightDimmer(light);
  startFadeDimmer(light);
}

void setupLightRGB(strLightRGB_t &light)
{
  light.status = loadLevelState(light.sensor, 0);
  for (int i = 0; i < RGB_SIZE; i++)
  {
    light.fadeTo[i] = loadLevelState(light.sensor, i + 1);
    ledcSetup(light.channel[i], LED_BASE_FREQ, LED_TIMER_BIT);
    ledcAttachPin(light.pin[i], light.channel[i]);
    if (!light.status)
    {
      light.dimValue[i] = 0;
      light.fadeTo[i] = 0;
    }
  }
  printLightRGB(light);
  startFadeRGB(light);
}

void setupLightRGBW(strLightRGBW_t &light)
{
  light.status = loadLevelState(light.sensor, 0);
  for (int i = 0; i < RGBW_SIZE; i++)
  {
    light.fadeTo[i] = loadLevelState(light.sensor, i + 1);
    ledcSetup(light.channel[i], LED_BASE_FREQ, LED_TIMER_BIT);
    ledcAttachPin(light.pin[i], light.channel[i]);
    if (!light.status)
    {
      light.dimValue[i] = 0;
      light.fadeTo[i] = 0;
    }
  }
  printLightRGBW(light);
  startFadeRGBW(light);
}

void presentation()
{
  printHeader("PRESENTATION");
  //presentation for light
  for (int i = 0; i < LIGHT_SIZE; i++)
  {
    present(lights[i].sensor, S_BINARY, lights[i].name);
  }
  //presentation for light dimmer
  for (int i = 0; i < LIGHT_DIMMER_SIZE; i++)
  {
    present(lightDimmers[i].sensor, S_DIMMER, lightDimmers[i].name);
  }
  //presentation for light RGB
  for (int i = 0; i < LIGHT_RGB_SIZE; i++)
  {
    present(lightRGBs[i].sensor, S_RGB_LIGHT, lightRGBs[i].name);
  }
  //presentation for light RGB
  for (int i = 0; i < LIGHT_RGBW_SIZE; i++)
  {
    present(lightRGBWs[i].sensor, S_RGBW_LIGHT, lightRGBWs[i].name);
  }
  sendSketchInfo(SN, SV);
}

void receive(const MyMessage &message)
{
  printHeader("RECEIVE");
  //receive for light
  for (int i = 0; i < LIGHT_SIZE; i++)
  {
    strLight_t &light = lights[i];
    if (message.sensor == light.sensor)
    {
      bool value = message.getBool();
      reciveLight(light, value);
    }
  }
  //receive for light dimmer
  for (int i = 0; i < LIGHT_DIMMER_SIZE; i++)
  {
    strLightDimmer_t &light = lightDimmers[i];
    if (message.sensor == light.sensor)
    {
      byte value = message.getByte();
      reciveLightDimmer(light, message.type, value);
    }
  }
  //receive for light RGB
  for (int i = 0; i < LIGHT_RGB_SIZE; i++)
  {
    strLightRGB_t &light = lightRGBs[i];
    if (message.sensor == light.sensor)
    {
      const char *value = message.getString();
      reciveLightRGB(light, message.type, value);
    }
  }
  //receive for light RGBW
  for (int i = 0; i < LIGHT_RGBW_SIZE; i++)
  {
    strLightRGBW_t &light = lightRGBWs[i];
    if (message.sensor == light.sensor)
    {
      const char *value = message.getString();
      reciveLightRGBW(light, message.type, value);
    }
  }
}

void reciveLight(strLight_t &light, bool value)
{
  printHeader("Before");
  printLight(light);

  light.status = value;
  saveLevelState(light.sensor, 0, value);
  digitalWrite(light.pin, value ? RELAY_ON : RELAY_OFF);
  send(MyMessage(light.sensor, V_STATUS).set(value), true);

  printHeader("After");
  printLight(light);
}

void reciveLightDimmer(strLightDimmer_t &light, uint8_t type, byte value)
{
  printHeader("Before");
  printLightDimmer(light);

  if (type == V_LIGHT)
  {
    light.status = value;
    light.fadeTo = value ? loadLevelState(light.sensor, 1) : 0;
    saveLevelState(light.sensor, 0, value);
  }
  else if (type == V_DIMMER)
  {
    light.fadeTo = value;
    saveLevelState(light.sensor, 1, value);
  }
  startFadeDimmer(light);
}

void reciveLightRGB(strLightRGB_t &light, uint8_t type, const char *value)
{
  printHeader("Before");
  printLightRGB(light);

  if (type == V_LIGHT)
  {
    bool status = (char)atoi(value);
    light.status = status;
    for (int i = 0; i < RGB_SIZE; i++)
    {
      light.fadeTo[i] = status ? loadLevelState(light.sensor, i + 1) : 0;
    }
    saveLevelState(light.sensor, 0, status);
  }
  else
  {
    if (strlen(value) != 6)
      return;

    byte target_values[RGB_SIZE] = {0, 0, 0};
    target_values[0] = fromhex(&value[0]);
    target_values[1] = fromhex(&value[2]);
    target_values[2] = fromhex(&value[4]);

    for (int i = 0; i < RGB_SIZE; i++)
    {
      light.fadeTo[i] = target_values[i];
      saveLevelState(light.sensor, i + 1, target_values[i]);
    }
  }
  startFadeRGB(light);

  printHeader("After");
  printLightRGB(light);
}

void reciveLightRGBW(strLightRGBW_t &light, uint8_t type, const char *value)
{
  printHeader("BEFORE");
  printLightRGBW(light);

  if (type == V_LIGHT)
  {
    bool status = (char)atoi(value);
    light.status = status;
    for (int i = 0; i < RGBW_SIZE; i++)
    {
      light.fadeTo[i] = status ? loadLevelState(light.sensor, i + 1) : 0;
    }
    saveLevelState(light.sensor, 0, status);
  }
  startFadeRGBW(light);

  printHeader("After");
  printLightRGBW(light);
}

void loop()
{
}

void startFadeDimmer(strLightDimmer_t &light)
{
  light.fadeDelta = (light.fadeTo - light.dimValue) < 0 ? -1 : 1;
  light.lastFadeStep = millis();
}

void startFadeRGB(strLightRGB_t &light)
{
  for (int i = 0; i < RGB_SIZE; i++)
  {
    light.fadeDelta[i] = (light.fadeTo[i] - light.dimValue[i]) < 0 ? -1 : 1;
    light.lastFadeStep[i] = millis();
  }
}

void startFadeRGBW(strLightRGBW_t &light)
{
  for (int i = 0; i < RGBW_SIZE; i++)
  {
    light.fadeDelta[i] = (light.fadeTo[i] - light.dimValue[i]) < 0 ? -1 : 1;
    light.lastFadeStep[i] = millis();
  }
}

void printHeader(const char *name)
{
#ifdef DEBUG
  Serial.println(name);
#endif
}

void printLight(strLight_t &light)
{
#ifdef DEBUG
  Serial.print("\t");
  Serial.print(light.name);
  Serial.print(strlen(light.name) > 8 ? "\t" : "\t\t");
  Serial.print("pin=");
  Serial.print(light.pin);
  Serial.print(", sensor=");
  Serial.print(light.sensor);
  Serial.print(", status=");
  Serial.println(light.status);
#endif
}

void printLightDimmer(strLightDimmer_t &light)
{
#ifdef DEBUG
  Serial.print("\t");
  Serial.print(light.name);
  Serial.print(strlen(light.name) > 8 ? "\t" : "\t\t");
  Serial.print("pin=");
  Serial.print(light.pin);
  Serial.print(", sensor=");
  Serial.print(light.sensor);
  Serial.print(", channel=");
  Serial.print(light.channel);
  Serial.print(", status=");
  Serial.print(light.status);
  Serial.print(", fadeTo=");
  Serial.print(light.fadeTo);
  Serial.print(", dimValue=");
  Serial.println(light.dimValue);
#endif
}

void printLightRGB(strLightRGB_t &light)
{
#ifdef DEBUG
  for (int i = 0; i < RGB_SIZE; i++)
  {
    Serial.print("\t");
    Serial.print(i == 0 ? light.name : "\t");
    Serial.print(strlen(light.name) > 8 ? "\t" : "\t\t");
    Serial.print("pin=");
    Serial.print(light.pin[i]);
    Serial.print(", sensor=");
    Serial.print(light.sensor);
    Serial.print(", channel=");
    Serial.print(light.channel[i]);
    Serial.print(", status=");
    Serial.print(light.status);
    Serial.print(", fadeTo=");
    Serial.print(light.fadeTo[i]);
    Serial.print(", dimValue=");
    Serial.println(light.dimValue[i]);
  }
#endif
}

void printLightRGBW(strLightRGBW_t &light)
{
#ifdef DEBUG
  for (int i = 0; i < RGBW_SIZE; i++)
  {
    Serial.print("\t");
    Serial.print(i == 0 ? light.name : "\t");
    Serial.print(strlen(light.name) > 8 ? "\t" : "\t\t");
    Serial.print("pin=");
    Serial.print(light.pin[i]);
    Serial.print(", sensor=");
    Serial.print(light.sensor);
    Serial.print(", channel=");
    Serial.print(light.channel[i]);
    Serial.print(", status=");
    Serial.print(light.status);
    Serial.print(", fadeTo=");
    Serial.print(light.fadeTo[i]);
    Serial.print(", dimValue=");
    Serial.println(light.dimValue[i]);
  }
#endif
}

byte fromhex(const char *str)
{
  char c = str[0] - '0';
  if (c > 9)
    c -= 7;
  int result = c;
  c = str[1] - '0';
  if (c > 9)
    c -= 7;
  return (result << 4) | c;
}

uint8_t loadLevelState(byte sensor, byte index)
{
  byte pos = ((sensor * 5) - 4) + index;
  return loadState(pos);
}

void saveLevelState(byte sensor, byte index, byte data)
{
  byte pos = ((sensor * 5) - 4) + index;
  saveState(pos, data);
}
