#include "Arduino.h"
#include <MySensors.h>

typedef struct
{
    const uint8_t channel;
    const uint8_t pin;
    const uint8_t eepromPos;
    bool status;
    int fadeTo;
    int fadeDelta;
    int dimValue;
    unsigned long lastFadeStep;
} strLightItem_t;

typedef struct
{
    const uint8_t sensor;
    const char *name;
    strLightItem_t *lightItem;
    const bool dimmer;
    MyMessage myMessage;
} strLight_t;

typedef struct
{
    const uint8_t sensor;
    const char *name;
    strLightItem_t *lightItems[4];
    MyMessage myMessage;
    const char *rgbValue;
} strLightRGBW_t;

typedef struct
{
    const uint8_t pin;
    const byte lightsSize;
    strLight_t *lights;
    strLightRGBW_t *lightLeds;
    bool status;
    const uint8_t eepromPos;
} strButton_t;

void setupLight(strLight_t &light);
void setupLightRGBW(strLightRGBW_t &light);

void presentationLight(strLight_t &light);
void presentationLightRGBW(strLightRGBW_t &light);

void reciveLight(strLight_t &light, byte value);
void reciveLightDimmer(strLightItem_t &lightItem, uint8_t type, byte value);
void reciveLightRGBW(strLightRGBW_t &light);

void setStatusButton(byte pin);

void startFade(strLightItem_t &lightItem);
void fadeStep();
void fadeRGBWStep();
void switchButton();

byte fromhex(const char *str);
int loadLevelState(byte pos);
void saveLevelState(byte pos, byte data);
