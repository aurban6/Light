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
  startFadeLightDimmer(light);
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
  startFadeLightRGB(light);
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
  startFadeLightRGBW(light);
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
    send(MyMessage(light.sensor, V_STATUS).set(value), true);
  }
  else if (type == V_DIMMER)
  {
    light.fadeTo = value;
    saveLevelState(light.sensor, 1, value);
  }
  startFadeLightDimmer(light);
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
  else if (type == V_RGB)
  {
    if (strlen(value) != 6)
      return;

    byte target_values[RGB_SIZE] = {0, 0, 0};
    target_values[0] = fromhex(&value[0]);
    target_values[1] = fromhex(&value[2]);
    target_values[2] = fromhex(&value[4]);
    light.rgbValue = strdup(value);

    for (int i = 0; i < RGB_SIZE; i++)
    {
      light.fadeTo[i] = target_values[i];
      saveLevelState(light.sensor, i + 1, target_values[i]);
    }
  }
  startFadeLightRGB(light);
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
  else if (type == V_RGBW)
  {
    byte target_values[4] = {0, 0, 0, 0};
    bool isWhite = strlen(value) == 9;
    if (isWhite)
    {
      target_values[0] = fromhex(&value[1]);
      target_values[1] = fromhex(&value[3]);
      target_values[2] = fromhex(&value[5]);
      target_values[3] = fromhex(&value[7]);
      light.wValue = strdup(value);
    }
    else
    {
      target_values[0] = fromhex(&value[0]);
      target_values[1] = fromhex(&value[2]);
      target_values[2] = fromhex(&value[4]);
      target_values[3] = 0;
      light.rgbValue = strdup(value);
    }

    byte sizeFrom = 0;
    byte sizeTo = RGBW_SIZE - 1;
    if (isWhite)
    {
      sizeFrom = 3;
      sizeTo = RGBW_SIZE;
    }

    for (int i = sizeFrom; i < sizeTo; i++)
    {
      light.fadeTo[i] = target_values[i];
      saveLevelState(light.sensor, i + 1, target_values[i]);
    }
  }
  startFadeLightRGBW(light);
}

void loop()
{
  fadeLightDimmer();
  fadeLightRGB();
  fadeLightRGBW();
}

void fadeLightDimmer()
{
  for (int i = 0; i < LIGHT_DIMMER_SIZE; i++)
  {
    strLightDimmer_t &light = lightDimmers[i];
    unsigned long currentTime = millis();
    if (light.dimValue != light.fadeTo && currentTime > light.lastFadeStep + FADE_DELAY)
    {
      light.dimValue += light.fadeDelta;
      uint32_t duty = (light.dimValue / 100. * 256);
      ledcWrite(light.channel, duty);
      light.lastFadeStep = currentTime;
      if (light.fadeTo == light.dimValue)
      {
        send(MyMessage(light.sensor, V_DIMMER).set(light.dimValue), true);

        printHeader("After");
        printLightDimmer(light);
      }
    }
  }
}

void fadeLightRGB()
{
  for (int i = 0; i < LIGHT_RGB_SIZE; i++)
  {
    strLightRGB_t &light = lightRGBs[i];
    for (int j = 0; j < RGB_SIZE; j++)
    {
      unsigned long currentTime = millis();
      if (light.dimValue[j] != light.fadeTo[j] && currentTime > light.lastFadeStep[j] + FADE_DELAY)
      {
        light.dimValue[j] += light.fadeDelta[j];
        ledcWrite(light.channel[j], light.dimValue[j]);
        light.lastFadeStep[j] = currentTime;

        if (light.fadeTo[0] == light.dimValue[0] && light.fadeTo[1] == light.dimValue[1] && light.fadeTo[2] == light.dimValue[2])
        {
          printHeader("After");
          printLightRGB(light);
        }
      }
    }
  }
}

void fadeLightRGBW()
{
  for (int i = 0; i < LIGHT_RGBW_SIZE; i++)
  {
    strLightRGBW_t &light = lightRGBWs[i];
    for (int j = 0; j < RGBW_SIZE; j++)
    {
      unsigned long currentTime = millis();
      if (light.dimValue[j] != light.fadeTo[j] && currentTime > light.lastFadeStep[j] + FADE_DELAY)
      {
        light.dimValue[j] += light.fadeDelta[j];
        ledcWrite(light.channel[j], light.dimValue[j]);
        light.lastFadeStep[j] = currentTime;

        if (light.fadeTo[0] == light.dimValue[0] && light.fadeTo[1] == light.dimValue[1] && light.fadeTo[2] == light.dimValue[2] && light.fadeTo[3] == light.dimValue[3])
        {
          printHeader("After");
          printLightRGBW(light);
        }
      }
    }
  }
}

void startFadeLightDimmer(strLightDimmer_t &light)
{
  light.fadeDelta = (light.fadeTo - light.dimValue) < 0 ? -1 : 1;
  light.lastFadeStep = millis();
}

void startFadeLightRGB(strLightRGB_t &light)
{
  for (int i = 0; i < RGB_SIZE; i++)
  {
    light.fadeDelta[i] = (light.fadeTo[i] - light.dimValue[i]) < 0 ? -1 : 1;
    light.lastFadeStep[i] = millis();
  }
}

void startFadeLightRGBW(strLightRGBW_t &light)
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
    Serial.print(light.dimValue[i]);
    Serial.print(", rgbValue=");
    Serial.println(light.rgbValue);
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
    Serial.print(light.dimValue[i]);
    Serial.print(", rgbValue=");
    Serial.print(light.rgbValue);
    Serial.print(", wValue=");
    Serial.println(light.wValue);
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
