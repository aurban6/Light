#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>
#include <Bounce2.h>

// Enable debug prints to serial monitor
//#define MY_DEBUG

#define MY_GATEWAY_ESP32
#define MY_PORT 5003
#define MY_GATEWAY_MAX_CLIENTS 2

#include "configMySensors.h"
#include "main.h"

#define LED_TIMER_BIT 8
#define LED_BASE_FREQ 5000
#define RELAY_ON 0
#define RELAY_OFF 1

#include "config.h"

Bounce *bounces = new Bounce[BUTTON_SIZE];

void setup()
{
#ifdef DEBUG
  Serial.println();
  Serial.println("SETUP");
#endif
  //setup for Led dimmer, Light
  for (int i = 0; i < LIGHT_SIZE; i++)
  {
    setupLight(_lights[i]);
  }
  //setup for RGB, RGBW Led
  for (int i = 0; i < LIGHT_RGBW_SIZE; i++)
  {
    setupLightRGBW(_lightRGBWs[i]);
  }
  //setup for buttons
  for (int i = 0; i < BUTTON_SIZE; i++)
  {
    bounces[i].attach(_buttons[i].pin, INPUT_PULLUP); //setup the bounce instance for the current button
    bounces[i].interval(25);                          //interval in ms
    _buttons[i].status = loadLevelState(_buttons[i].eepromPos);
#ifdef DEBUG
    Serial.print("    Button: pin=");
    Serial.print(_buttons[i].pin);
    Serial.print(", status=");
    Serial.println(_buttons[i].status);
#endif
  }
}

void presentation()
{
  //presentation for Led dimmer, Light
  for (int i = 0; i < LIGHT_SIZE; i++)
  {
    presentationLight(_lights[i]);
  }
  //presentation for RGB, RGBW Led
  for (int i = 0; i < LIGHT_RGBW_SIZE; i++)
  {
    presentationLightRGBW(_lightRGBWs[i]);
  }
  sendSketchInfo(SN, SV);
}

void receive(const MyMessage &message)
{
#ifdef DEBUG
  Serial.println();
  Serial.println("RECIVE");
#endif
  //recive for Led dimmer, Light
  for (int i = 0; i < LIGHT_SIZE; i++)
  {
    strLight_t &light = _lights[i];
    if (message.sensor == light.sensor)
    {
#ifdef DEBUG
      Serial.print("    ");
      Serial.print(light.name);
      Serial.print(": sensor=");
      Serial.print(light.sensor);
      Serial.print(", dimmer=");
      Serial.println(light.dimmer);
#endif
      strLightItem_t &lightItem = *light.lightItem;
      if (light.dimmer)
      {
        reciveLightDimmer(lightItem, message.type, message.getByte());
      }
      else
      {
        reciveLight(light, message.getBool());
      }
    }
  }
  //recive for RGB, RGBW Led
  for (int i = 0; i < LIGHT_RGBW_SIZE; i++)
  {
    strLightRGBW_t &light = _lightRGBWs[i];
    if (message.sensor == light.sensor)
    {
#ifdef DEBUG
      Serial.print("    ");
      Serial.print(light.name);
      Serial.print(": sensor=");
      Serial.println(light.sensor);
#endif
      strLightItem_t &lightItem_3 = *light.lightItems[3];
      byte type = lightItem_3.channel == 0 && lightItem_3.pin == 0;
      byte size = type ? 3 : 4;
      if (message.type == V_LIGHT)
      {
        for (int j = 0; j < size; j++)
        {
          strLightItem_t &lightItem = *light.lightItems[j];
          reciveLightDimmer(lightItem, message.type, message.getByte());
        }
        send(light.myMessage.set(message.getBool()), true);
      }
      else if (message.type == type ? V_RGB : V_RGBW)
      {
        light.rgbValue = message.getString();
        reciveLightRGBW(light);
      }
    }
  }
}

void loop()
{
  fadeStep();
  fadeRGBWStep();
  switchButton();
}

void setupLight(strLight_t &light)
{
#ifdef DEBUG
  Serial.print("    ");
  Serial.print(light.name);
  Serial.print(": sensor=");
  Serial.print(light.sensor);
  Serial.print(", dimmer=");
  Serial.println(light.dimmer);
#endif
  strLightItem_t &lightItem = *light.lightItem;
  if (light.dimmer)
  {
    lightItem.status = loadLevelState(lightItem.eepromPos);
    lightItem.fadeTo = loadLevelState(lightItem.eepromPos + 1);
    ledcSetup(lightItem.channel, LED_BASE_FREQ, LED_TIMER_BIT);
    ledcAttachPin(lightItem.pin, lightItem.channel);
    if (!lightItem.status)
    {
      lightItem.dimValue = 0;
      lightItem.fadeTo = 0;
    }
#ifdef DEBUG
    Serial.print("        Item: pin=");
    Serial.print(lightItem.pin);
    Serial.print(", channel=");
    Serial.print(lightItem.channel);
    Serial.print(", status=");
    Serial.print(lightItem.status);
    Serial.print(", fadeTo=");
    Serial.print(lightItem.fadeTo);
    Serial.print(", dimValue=");
    Serial.println(lightItem.dimValue);
#endif
    startFade(lightItem);
  }
  else
  {
    lightItem.status = loadLevelState(lightItem.eepromPos);
    pinMode(lightItem.pin, OUTPUT);
    digitalWrite(lightItem.pin, lightItem.status ? RELAY_ON : RELAY_OFF);
#ifdef DEBUG
    Serial.print("        Item: pin=");
    Serial.print(lightItem.pin);
    Serial.print(", status=");
    Serial.println(lightItem.status);
#endif
  }
}

void setupLightRGBW(strLightRGBW_t &light)
{
#ifdef DEBUG
  Serial.print("    ");
  Serial.print(light.name);
  Serial.print(": sensor=");
  Serial.println(light.sensor);
#endif

  strLightItem_t &lightItem_3 = *light.lightItems[3];
  byte size = lightItem_3.channel == 0 && lightItem_3.pin == 0 ? 3 : 4;
  for (int i = 0; i < size; i++)
  {
    strLightItem_t &lightItem = *light.lightItems[i];
    lightItem.status = loadLevelState(lightItem.eepromPos);
    lightItem.fadeTo = loadLevelState(lightItem.eepromPos + 1);
    ledcSetup(lightItem.channel, LED_BASE_FREQ, LED_TIMER_BIT);
    ledcAttachPin(lightItem.pin, lightItem.channel);
    if (!lightItem.status)
    {
      lightItem.dimValue = 0;
      lightItem.fadeTo = 0;
    }
#ifdef DEBUG
    Serial.print("        Item: pin=");
    Serial.print(lightItem.pin);
    Serial.print(", channel=");
    Serial.print(lightItem.channel);
    Serial.print(", status=");
    Serial.print(lightItem.status);
    Serial.print(", fadeTo=");
    Serial.print(lightItem.fadeTo);
    Serial.print(", dimValue=");
    Serial.println(lightItem.dimValue);
#endif
    startFade(lightItem);
  }
}

void presentationLight(strLight_t &light)
{
  present(light.sensor, light.dimmer ? S_DIMMER : S_BINARY, light.name);
}

void presentationLightRGBW(strLightRGBW_t &light)
{
  strLightItem_t &lightItem_3 = *light.lightItems[3];
  byte type = lightItem_3.channel == 0 && lightItem_3.pin == 0;
  present(light.sensor, type ? S_RGB_LIGHT : S_RGBW_LIGHT, light.name);
}

void reciveLight(strLight_t &light, byte value)
{
  strLightItem_t &lightItem = *light.lightItem;

#ifdef DEBUG
  Serial.print("        Before Item: pin=");
  Serial.print(lightItem.pin);
  Serial.print(", status=");
  Serial.println(lightItem.status);
#endif

  lightItem.status = value;
  saveLevelState(lightItem.eepromPos, lightItem.status);
  digitalWrite(lightItem.pin, lightItem.status ? RELAY_ON : RELAY_OFF);
  send(light.myMessage.set(lightItem.status), true);

#ifdef DEBUG
  Serial.print("         After Item: pin=");
  Serial.print(lightItem.pin);
  Serial.print(", status=");
  Serial.println(lightItem.status);
#endif

  setStatusButton(lightItem.pin);
}

void reciveLightDimmer(strLightItem_t &lightItem, uint8_t type, byte value)
{
#ifdef DEBUG
  Serial.print("        Before Item: pin=");
  Serial.print(lightItem.pin);
  Serial.print(", channel=");
  Serial.print(lightItem.channel);
  Serial.print(", status=");
  Serial.print(lightItem.status);
  Serial.print(", dimValue=");
  Serial.print(lightItem.dimValue);
  Serial.print(", fadeTo=");
  Serial.println(lightItem.fadeTo);
#endif
  if (type == V_LIGHT)
  {
    lightItem.fadeTo = value ? loadLevelState(lightItem.eepromPos + 1) : 0;
    lightItem.status = value;
    saveLevelState(lightItem.eepromPos, lightItem.status);
    setStatusButton(lightItem.pin);
  }
  else if (type == V_DIMMER)
  {
    lightItem.fadeTo = value;
    saveLevelState(lightItem.eepromPos + 1, lightItem.fadeTo);
  }
  startFade(lightItem);
}

void reciveLightRGBW(strLightRGBW_t &light)
{
  byte target_values[4] = {0, 0, 0, 0};
#ifdef DEBUG
  Serial.print("        color=");
  Serial.print(light.rgbValue);
  Serial.print(", length=");
  Serial.print(strlen(light.rgbValue));
  Serial.print(", RGBW=");
#endif
  if (strlen(light.rgbValue) == 6)
  {
    target_values[0] = fromhex(&light.rgbValue[0]);
    target_values[1] = fromhex(&light.rgbValue[2]);
    target_values[2] = fromhex(&light.rgbValue[4]);
    target_values[3] = 0;
  }
  else if (strlen(light.rgbValue) == 9)
  {
    target_values[0] = fromhex(&light.rgbValue[1]); // ignore # as first sign
    target_values[1] = fromhex(&light.rgbValue[3]);
    target_values[2] = fromhex(&light.rgbValue[5]);
    target_values[3] = fromhex(&light.rgbValue[7]);
  }
  else
  {
#ifdef DEBUG
    Serial.println("Wrong length of input");
#endif
    return;
  }
  byte start = 0;
  if (strlen(light.rgbValue) == 9)
    start = 3;
  strLightItem_t &lightItem_3 = *light.lightItems[3];
  byte size = lightItem_3.channel == 0 && lightItem_3.pin == 0 ? 3 : 4;

  for (int i = start; i < size; i++)
  {
#ifdef DEBUG
    Serial.print(target_values[i]);
    if (i < size - 1)
      Serial.print(", ");
#endif
    strLightItem_t &lightItem = *light.lightItems[i];
    lightItem.fadeTo = target_values[i];
    saveLevelState(lightItem.eepromPos + 1, lightItem.fadeTo);
    startFade(lightItem);
  }
  Serial.println();
}

void setStatusButton(byte pin)
{
  for (int i = 0; i < BUTTON_SIZE; i++)
  {
    strButton_t &button = _buttons[i];
    byte size = button.lightsSize;
    for (int j = 0; j < size; j++)
    {
      if (&button.lights[j] != nullptr)
      {
        strLight_t &light = button.lights[j];
        strLightItem_t &lightItem = *light.lightItem;
        if (lightItem.pin == pin)
        {
          button.status = lightItem.status;
        }
      }
      else if (&button.lightLeds[j] != nullptr)
      {
        strLightRGBW_t &light = button.lightLeds[j];
        strLightItem_t &lightItem_3 = *light.lightItems[3];
        byte type = lightItem_3.channel == 0 && lightItem_3.pin == 0;
        byte size = type ? 3 : 4;
        for (int j = 0; j < size; j++)
        {
          strLightItem_t &lightItem = *light.lightItems[j];
          if (lightItem.pin == pin)
          {
            button.status = lightItem.status;
          }
        }
      }
    }
  }
}

void startFade(strLightItem_t &lightItem)
{
  lightItem.fadeDelta = (lightItem.fadeTo - lightItem.dimValue) < 0 ? -1 : 1;
  lightItem.lastFadeStep = millis();
}

void fadeStep()
{
  for (int i = 0; i < LIGHT_SIZE; i++)
  {
    strLight_t &light = _lights[i];
    if (light.dimmer)
    {
      strLightItem_t &lightItem = *light.lightItem;
      unsigned long currentTime = millis();
      if (lightItem.dimValue != lightItem.fadeTo && currentTime > lightItem.lastFadeStep + FADE_DELAY)
      {
        lightItem.dimValue += lightItem.fadeDelta;
        uint32_t duty = (lightItem.dimValue / 100. * 256);
        ledcWrite(lightItem.channel, duty);
        lightItem.lastFadeStep = currentTime;
        if (lightItem.fadeTo == lightItem.dimValue)
        {
          send(light.myMessage.set(lightItem.dimValue), true);
#ifdef DEBUG
          Serial.print("         After Item: pin=");
          Serial.print(lightItem.pin);
          Serial.print(", channel=");
          Serial.print(lightItem.channel);
          Serial.print(", status=");
          Serial.print(lightItem.status);
          Serial.print(", dimValue=");
          Serial.print(lightItem.dimValue);
          Serial.print(", fadeTo=");
          Serial.println(lightItem.fadeTo);
#endif
        }
      }
    }
  }
}

void fadeRGBWStep()
{
  for (int i = 0; i < LIGHT_RGBW_SIZE; i++)
  {
    strLightRGBW_t &light = _lightRGBWs[i];
    strLightItem_t &lightItem_3 = *light.lightItems[3];
    byte size = lightItem_3.channel == 0 && lightItem_3.pin == 0 ? 3 : 4;
    for (int j = 0; j < size; j++)
    {
      strLightItem_t &lightItem = *light.lightItems[j];
      unsigned long currentTime = millis();
      if (lightItem.dimValue != lightItem.fadeTo && currentTime > lightItem.lastFadeStep + FADE_DELAY)
      {
        lightItem.dimValue += lightItem.fadeDelta;
        ledcWrite(lightItem.channel, lightItem.dimValue);
        lightItem.lastFadeStep = currentTime;
        if (lightItem.fadeTo == lightItem.dimValue)
        {
#ifdef DEBUG
          Serial.print("         After Item: pin=");
          Serial.print(lightItem.pin);
          Serial.print(", channel=");
          Serial.print(lightItem.channel);
          Serial.print(", status=");
          Serial.print(lightItem.status);
          Serial.print(", dimValue=");
          Serial.print(lightItem.dimValue);
          Serial.print(", fadeTo=");
          Serial.println(lightItem.fadeTo);
#endif
        }
      }
    }
  }
}

void switchButton()
{
  for (int i = 0; i < BUTTON_SIZE; i++)
  {
    bounces[i].update();
    if (bounces[i].fell())
    {
      strButton_t &button = _buttons[i];
      byte size = button.lightsSize;
      byte status = !button.status;

#ifdef DEBUG
      Serial.println();
      Serial.print("BUTTON");
      Serial.print(": size=");
      Serial.print(size);
      Serial.print(", Before status=");
      Serial.print(button.status);
      Serial.print(", After status=");
      Serial.println(status);
#endif

      for (int j = 0; j < size; j++)
      {
        if (&button.lights[j] != nullptr)
        {
          strLight_t &light = button.lights[j];
#ifdef DEBUG
          Serial.print("    ");
          Serial.print(light.name);
          Serial.print(": sensor=");
          Serial.print(light.sensor);
          Serial.print(", dimmer=");
          Serial.println(light.dimmer);
#endif
          strLightItem_t &lightItem = *light.lightItem;
          if (light.dimmer)
          {
            reciveLightDimmer(lightItem, V_LIGHT, status);
          }
          else
          {
            reciveLight(light, status);
          }
        }
        else if (&button.lightLeds[j] != nullptr)
        {
          strLightRGBW_t &light = button.lightLeds[j];
#ifdef DEBUG
          Serial.print("    ");
          Serial.print(light.name);
          Serial.print(", rgbValue=");
          Serial.println(light.rgbValue);
#endif
          strLightItem_t &lightItem_3 = *light.lightItems[3];
          byte type = lightItem_3.channel == 0 && lightItem_3.pin == 0;
          byte size = type ? 3 : 4;
          for (int j = 0; j < size; j++)
          {
            strLightItem_t &lightItem = *light.lightItems[j];
            reciveLightDimmer(lightItem, V_LIGHT, status);
          }
          send(light.myMessage.set(status), true);
        }
      }
      button.status = status;
      saveLevelState(button.eepromPos, button.status);
    }
  }
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

int loadLevelState(byte pos)
{
  return min(max(loadState(pos), 0), 100);
}

void saveLevelState(byte pos, byte data)
{
  saveState(pos, min(max(data, 0), 100));
}
