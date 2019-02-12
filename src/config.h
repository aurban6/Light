#define FADE_DELAY 10 //10ms = 1s
//#define DEBUG

strLightItem_t _lightItem0 = {0, 13, 0};
strLight_t _light0 = {0, "Wyspa", &_lightItem0, 0, MyMessage(0, V_STATUS)}; //definicja klasycznej zarowki, wlacz/wylacz

strLightItem_t _lightItem1 = {0, 12, 2};
strLight_t _light1 = {1, "Ogólne lewy", &_lightItem1, 1, MyMessage(1, V_DIMMER)}; //definicja pojedynczego leda z sciemnianiem

strLightItem_t _lightItem2 = {1, 14, 4};
strLight_t _light2 = {2, "Ogólne prawy", &_lightItem2, 1, MyMessage(2, V_DIMMER)}; //definicja pojedynczego leda z sciemnianiem

strLightItem_t _lightItem3 = {2, 27, 6};
strLightItem_t _lightItem4 = {3, 26, 8};
strLightItem_t _lightItem5 = {4, 25, 10};
strLightItem_t _lightItem6 = {5, 33, 12};
//strLightItem_t _lightItem6 = {0, 0, 0}; //for RGB all value must be 0
strLightRGBW_t _light3 = {3, "Szafka", {&_lightItem3, &_lightItem4, &_lightItem5, &_lightItem6}, MyMessage(3, V_RGB)}; //definicja led RGBW (V_RGBW - RGBW, V_RGB - RGB)


#define LIGHT_SIZE 3
strLight_t _lights[LIGHT_SIZE] = {_light0, _light1, _light2}; //lista oswietlenia typu pojedyncze (zarowki, led)

#define LIGHT_RGBW_SIZE 1
strLightRGBW_t _lightRGBWs[LIGHT_RGBW_SIZE] = {_light3}; //lista oswietlenia typu RGB, RGBW

/** BUTTONS **/
#define BUTTON_LIGHTS0_SIZE 1
strLight_t _buttonLight0[BUTTON_LIGHTS0_SIZE] = {_light0};

#define BUTTON_LIGHTS1_SIZE 2
strLight_t _buttonLight1[BUTTON_LIGHTS1_SIZE] = {_light1, _light2};

#define BUTTON_LIGHTS2_SIZE 1
strLightRGBW_t _buttonLight2[BUTTON_LIGHTS2_SIZE] = {_light3};

strButton_t _button0 = {4, BUTTON_LIGHTS0_SIZE, _buttonLight0, nullptr, 14};
strButton_t _button1 = {5, BUTTON_LIGHTS1_SIZE, _buttonLight1, nullptr, 15};
strButton_t _button2 = {16, BUTTON_LIGHTS2_SIZE, nullptr, _buttonLight2, 16};

#define BUTTON_SIZE 3
strButton_t _buttons[BUTTON_SIZE] = {_button0, _button1, _button2};

//oko