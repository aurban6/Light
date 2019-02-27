#include "config.h"

void setupLight(strLight_t &light);
void setupLightDimmer(strLightDimmer_t &light);
void setupLightRGB(strLightRGB_t &light);
void setupLightRGBW(strLightRGBW_t &light);
void setupButton();
void setupWebServer();

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

/*
 * Server Index Page
 */
const char *serverIndex =
    "<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
    "<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"
    "<input type='file' name='update'>"
    "<input type='submit' value='Update'>"
    "</form>"
    "<div id='prg'>progress: 0%</div>"
    "<script>"
    "$('form').submit(function(e){"
    "e.preventDefault();"
    "var form = $('#upload_form')[0];"
    "var data = new FormData(form);"
    " $.ajax({"
    "url: '/update',"
    "type: 'POST',"
    "data: data,"
    "contentType: false,"
    "processData:false,"
    "xhr: function() {"
    "var xhr = new window.XMLHttpRequest();"
    "xhr.upload.addEventListener('progress', function(evt) {"
    "if (evt.lengthComputable) {"
    "var per = evt.loaded / evt.total;"
    "$('#prg').html('progress: ' + Math.round(per*100) + '%');"
    "}"
    "}, false);"
    "return xhr;"
    "},"
    "success:function(d, s) {"
    "console.log('success!')"
    "},"
    "error: function (a, b, c) {"
    "}"
    "});"
    "});"
    "</script>";