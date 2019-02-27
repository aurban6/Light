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
