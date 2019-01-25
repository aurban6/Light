#include "Arduino.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>

// Enable debug prints to serial monitor
//#define MY_DEBUG

#define MY_GATEWAY_ESP32
#define MY_WIFI_SSID "APCeramika2"
#define MY_WIFI_PASSWORD "1qaz2wsx"
#define MY_HOSTNAME "AquaLed"
#define MY_PORT 5003
#define MY_GATEWAY_MAX_CLIENTS 2

#include <MySensors.h>

#define SN "AquaLed"
#define SV "3.0"

#define FADE_DELAY 20 //10ms = 1s
#define LED_TIMER_BIT 8
#define LED_BASE_FREQ 5000

typedef struct
{
  const uint8_t channel;
  const uint8_t pin;
  const uint8_t eepromPos;
  const char *name;
  MyMessage myMessage;
  bool status;
  int fadeTo;
  int fadeDelta;
  int dimValue;
  unsigned long lastFadeStep;
} strLed_t;

strLed_t _led0 = {0, 13, 0, "Czerwony", MyMessage(0, V_DIMMER)};
strLed_t _led1 = {1, 12, 2, "Zielony", MyMessage(1, V_DIMMER)};
strLed_t _led2 = {2, 14, 4, "Niebieski", MyMessage(2, V_DIMMER)};
strLed_t _led3 = {3, 27, 6, "Biały", MyMessage(3, V_DIMMER)};
strLed_t _led4 = {4, 26, 8, "Biały 1", MyMessage(4, V_DIMMER)};
strLed_t _led5 = {5, 25, 10, "Biały 2", MyMessage(5, V_DIMMER)};
strLed_t _led6 = {6, 33, 12, "Biały 3", MyMessage(6, V_DIMMER)};
strLed_t _led7 = {7, 32, 14, "Biały 4", MyMessage(7, V_DIMMER)};

#define LED_SIZE 8
static strLed_t leds[LED_SIZE] = {_led0, _led1, _led2, _led3, _led4, _led5, _led6, _led7};

#define SERWIS_LED_SENSOR 20 //selector 0-off, 10-Auto, 20-Max
MyMessage serwisLedMsg(SERWIS_LED_SENSOR, V_PERCENTAGE);

void setupLed(strLed_t &led);
void setupWebServer();
void startFade(strLed_t &led);
void fadeStep();
void setSerwisLed(byte value);
int loadLevelState(byte pos);
void saveLevelState(byte pos, byte data);

/*
 * Web Update
 */
const char *host = MY_HOSTNAME;
const char *ssid = MY_WIFI_SSID;
const char *password = MY_WIFI_PASSWORD;

WebServer server(80);

const char *loginIndex =
    "<form name='loginForm'>"
    "<table width='20%' bgcolor='A09F9F' align='center'>"
    "<tr>"
    "<td colspan=2>"
    "<center><font size=4><b>ESP32 Login Page</b></font></center>"
    "<br>"
    "</td>"
    "<br>"
    "<br>"
    "</tr>"
    "<td>Username:</td>"
    "<td><input type='text' size=25 name='userid'><br></td>"
    "</tr>"
    "<br>"
    "<br>"
    "<tr>"
    "<td>Password:</td>"
    "<td><input type='Password' size=25 name='pwd'><br></td>"
    "<br>"
    "<br>"
    "</tr>"
    "<tr>"
    "<td><input type='submit' onclick='check(this.form)' value='Login'></td>"
    "</tr>"
    "</table>"
    "</form>"
    "<script>"
    "function check(form)"
    "{"
    "if(form.userid.value=='aurban6' && form.pwd.value=='aurban123')"
    "{"
    "window.open('/serverIndex')"
    "}"
    "else"
    "{"
    " alert('Error Password or Username')/*displays error message*/"
    "}"
    "}"
    "</script>";

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

void setup()
{
  for (int i = 0; i < LED_SIZE; i++)
  {
    setupLed(leds[i]);
  }
  setupWebServer();
}

void presentation()
{
  for (int i = 0; i < LED_SIZE; i++)
  {
    strLed_t &led = leds[i];
    present(led.channel, S_DIMMER, led.name);
  }
  present(SERWIS_LED_SENSOR, S_DIMMER, "Led akwarium");
  sendSketchInfo(SN, SV);
}

void loop()
{
  fadeStep();
  server.handleClient();
}

void receive(const MyMessage &message)
{
  switch (message.sensor)
  {
  case SERWIS_LED_SENSOR:
    setSerwisLed(message.getByte());
    break;
  default:
    for (int i = 0; i < LED_SIZE; i++)
    {
      strLed_t &led = leds[i];
      if (led.channel == message.sensor)
      {
        if (message.type == V_LIGHT)
        {
          led.fadeTo = message.getBool() ? loadLevelState(led.eepromPos + 1) : 0;
          saveLevelState(led.eepromPos, message.getBool());
        }
        else if (message.type == V_DIMMER)
        {
          led.fadeTo = message.getByte();
          saveLevelState(led.eepromPos + 1, led.fadeTo);
        }
        startFade(led);
        return;
      }
    }
    break;
  }
}

void setupLed(strLed_t &led)
{
  led.status = loadLevelState(led.eepromPos);
  led.fadeTo = loadLevelState(led.eepromPos + 1);
  ledcSetup(led.channel, LED_BASE_FREQ, LED_TIMER_BIT);
  ledcAttachPin(led.pin, led.channel);
  if (!led.status)
  {
    led.dimValue = 0;
    led.fadeTo = 0;
  }
  startFade(led);
}

void setupWebServer()
{
  // Connect to WiFi network
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  /*use mdns for host name resolution*/
  if (!MDNS.begin(host))
  { //http://esp32.local
    Serial.println("Error setting up MDNS responder!");
    while (1)
    {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");
  /*return index page which is stored in serverIndex */
  server.on("/", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", loginIndex);
  });
  server.on("/serverIndex", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex);
  });
  /*handling uploading firmware file */
  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart(); }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("Update: %s\n", upload.filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      /* flashing firmware to ESP*/
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
    } });
  server.begin();
}

void startFade(strLed_t &led)
{
  led.fadeDelta = (led.fadeTo - led.dimValue) < 0 ? -1 : 1;
  led.lastFadeStep = millis();
}

void fadeStep()
{
  for (int i = 0; i < LED_SIZE; i++)
  {
    strLed_t &led = leds[i];
    unsigned long currentTime = millis();
    if (led.dimValue != led.fadeTo && currentTime > led.lastFadeStep + FADE_DELAY)
    {
      led.dimValue += led.fadeDelta;
      uint32_t duty = (led.dimValue / 100. * 256);
      ledcWrite(led.channel, duty);
      led.lastFadeStep = currentTime;

      if (led.fadeTo == led.dimValue)
      {
        send(led.myMessage.set(led.dimValue), true);
      }
    }
  }
}

void setSerwisLed(byte value)
{
  switch (value)
  {
  case 0: // Off
    for (int i = 0; i < LED_SIZE; i++)
    {
      strLed_t &led = leds[i];
      led.fadeTo = 0;
      saveLevelState(led.eepromPos, 0);
      startFade(led);
    }
    break;
  case 10: //Auto
    for (int i = 0; i < LED_SIZE; i++)
    {
      strLed_t &led = leds[i];
      led.fadeTo = loadLevelState(led.eepromPos + 1);
      saveLevelState(led.eepromPos, 1);
      startFade(led);
    }
    break;
  case 20: //Max
    for (int i = 0; i < LED_SIZE; i++)
    {
      strLed_t &led = leds[i];
      led.fadeTo = 100;
      startFade(led);
    }
    break;
  }
  send(serwisLedMsg.set(value));
}

int loadLevelState(byte pos)
{
  return min(max(loadState(pos), 0), 100);
}

void saveLevelState(byte pos, byte data)
{
  saveState(pos, min(max(data, 0), 100));
}
