#define SN "Light"
#define SV "3.0"

#define MY_WIFI_SSID "APCeramika2"
#define MY_WIFI_PASSWORD "1qaz2wsx"
#define MY_HOSTNAME "AquaLed"

#define FADE_DELAY 10 //10 = 1s
//#define DEBUG


/** Classic -> {pin, sensor, name} **/
strLight_t light1 = {15, 1, "Wyspa"};

/** Dimmer -> {pin, sensor, channel, name}**/
strLightDimmer_t light2 = {13, 2, 0, "Ogólne lewy"};
strLightDimmer_t light3 = {12, 3, 1, "Ogólne prawy"};

/** RGB -> {pin[3], sensor, channel[3], name}**/
strLightRGB_t light4 = {{14, 27, 26}, 4, {2, 3, 4}, "Szafka RGB"};

/** RGBW -> {pin[4], sensor, channel[4], name}**/
strLightRGBW_t light5 = {{25, 33, 32, 34}, 5, {5, 6, 7, 8}, "Szafka RGBW"};


#define LIGHT_SIZE 1
strLight_t lights[LIGHT_SIZE] = {light1};

#define LIGHT_DIMMER_SIZE 2
strLightDimmer_t lightDimmers[LIGHT_DIMMER_SIZE] = {light2, light3};

#define LIGHT_RGB_SIZE 1
strLightRGB_t lightRGBs[LIGHT_RGB_SIZE] = {light4};

#define LIGHT_RGBW_SIZE 1
strLightRGBW_t lightRGBWs[LIGHT_RGBW_SIZE] = {light5};


/** Button -> {pin, sensor, lights, lightDimmers, lightRGBs, lightRGBWs} **/
strButton_t button1 = {2, 6, lights, 0, 0, 0};
strButton_t button2 = {4, 7, 0, lightDimmers, 0, 0};
strButton_t button3 = {5, 8, 0, 0, lightRGBs, 0};
strButton_t button4 = {16, 9, 0, 0, 0, lightRGBWs};

#define BUTTON_SIZE 4
strButton_t buttons[BUTTON_SIZE] = {button1, button2, button3, button4};
