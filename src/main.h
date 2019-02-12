#include "Arduino.h"

typedef struct
{
    const uint8_t pin;
    const uint8_t sensor;
    const char *name;
    bool status;
} strLight_t;

typedef struct
{
    const uint8_t pin;
    const uint8_t sensor;
    const uint8_t channel;
    const char *name;
    bool status;
    int fadeTo;
    int fadeDelta;
    int dimValue;
    unsigned long lastFadeStep;
} strLightDimmer_t;

#define RGB_SIZE 3
typedef struct
{
    const uint8_t pin[RGB_SIZE];
    const uint8_t sensor;
    const uint8_t channel[RGB_SIZE];
    const char *name;
    bool status;
    int fadeTo[RGB_SIZE];
    int fadeDelta[RGB_SIZE];
    int dimValue[RGB_SIZE];
    unsigned long lastFadeStep[RGB_SIZE];
    const char *rgbValue;
} strLightRGB_t;

#define RGBW_SIZE 4
typedef struct
{
    const uint8_t pin[RGBW_SIZE];
    const uint8_t sensor;
    const uint8_t channel[RGBW_SIZE];
    const char *name;
    bool status;
    int fadeTo[RGBW_SIZE];
    int fadeDelta[RGBW_SIZE];
    int dimValue[RGBW_SIZE];
    unsigned long lastFadeStep[RGBW_SIZE];
    const char *rgbValue;
    const char *wValue;
} strLightRGBW_t;

typedef struct
{
    const uint8_t pin;
    const uint8_t sensor;
    strLight_t *lights;
    strLightDimmer_t *lightDimmers;
    strLightRGB_t *lightRGBs;
    strLightRGBW_t *lightRGBWs;
    bool status;
} strButton_t;

void setupLight(strLight_t &light);
void setupLightDimmer(strLightDimmer_t &light);
void setupLightRGB(strLightRGB_t &light);
void setupLightRGBW(strLightRGBW_t &light);
void setupButton();

void reciveLight(strLight_t &light, bool value);
void reciveLightDimmer(strLightDimmer_t &light, uint8_t type, byte value);
void reciveLightRGB(strLightRGB_t &light, uint8_t type, const char *value);
void reciveLightRGBW(strLightRGBW_t &light, uint8_t type, const char *value);

void fadeLightDimmer();
void fadeLightRGB();
void fadeLightRGBW();
void switchButton();

void startFadeLightDimmer(strLightDimmer_t &light);
void startFadeLightRGB(strLightRGB_t &light);
void startFadeLightRGBW(strLightRGBW_t &light);

void printHeader(const char *name);
void printLight(strLight_t &light);
void printLightDimmer(strLightDimmer_t &light);
void printLightRGB(strLightRGB_t &light);
void printLightRGBW(strLightRGBW_t &light);

void printButton(strButton_t &button);

bool getStatus(bool status1, bool status2);
byte fromhex(const char *str);
//index: status = 0, value1 = 1, value2 = 2, value3 = 3, value4 = 4
uint8_t loadLevelState(byte sensor, byte index);
void saveLevelState(byte sensor, byte index, byte data);