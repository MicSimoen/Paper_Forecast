/* ESP32 Weather Display using an EPD 4.2" Display, obtains data from Open Weather Map, decodes it and then displays it.
  ####################################################################################################################################
  This software, the ideas and concepts is Copyright (c) David Bird 2018. All rights to this software are reserved.

  Any redistribution or reproduction of any part or all of the contents in any form is prohibited other than the following:
  1. You may print or download to a local hard disk extracts for your personal and non-commercial use only.
  2. You may copy the content to individual third parties for their personal use, but only if you acknowledge the author David Bird as the source of the material.
  3. You may not, except with my express written permission, distribute or commercially exploit the content.
  4. You may not transmit it or store it in any other website or other form of electronic retrieval system for commercial purposes.

  The above copyright ('as annotated') notice and this permission notice shall be included in all copies or substantial portions of the Software and where the
  software use is visible to an end-user.

  THE SOFTWARE IS PROVIDED "AS IS" FOR PRIVATE USE ONLY, IT IS NOT FOR COMMERCIAL USE IN WHOLE OR PART OR CONCEPT. FOR PERSONAL USE IT IS SUPPLIED WITHOUT WARRANTY
  OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
  IN NO EVENT SHALL THE AUTHOR OR COPYRIGHT HOLDER BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
  See more at http://www.dsbird.org.uk
*/

#include <string>
#include "owm_credentials2.h"  // See 'owm_credentials' tab and enter your OWM API key and set the Wifi SSID and PASSWORD
#include <ArduinoJson.h>       // https://github.com/bblanchon/ArduinoJson NOTE: *** MUST BE Version-6 or above ***
#include <ESP8266WiFi.h>       // Built-in
#include "time.h"              // Built-in
#include <SPI.h>               // Built-in 
#include "EPD_WaveShare_42BC.h"  // Copyright (c) 2017 by Daniel Eichhorn https://github.com/ThingPulse/minigrafx
#include "MiniGrafx.h"         // Copyright (c) 2017 by Daniel Eichhorn https://github.com/ThingPulse/minigrafx
#include "DisplayDriver.h"     // Copyright (c) 2017 by Daniel Eichhorn https://github.com/ThingPulse/minigrafx
#include "ArialRounded.h"      // Copyright (c) 2017 by Daniel Eichhorn https://github.com/ThingPulse/minigrafx

#define SCREEN_WIDTH  400.0    // Set for landscape mode, don't remove the decimal place!
#define SCREEN_HEIGHT 300.0
#define BITS_PER_PIXEL 1
#define EPD_BLACK 0
#define EPD_WHITE 1
uint16_t palette[] = { 0, 1 };

// LOLIN D1 MINI PRO
static const uint8_t EPD_BUSY = 4;
static const uint8_t EPD_SS   = 15;
static const uint8_t EPD_RST  = 2;
static const uint8_t EPD_DC   = 0;
static const uint8_t EPD_SCK  = 14;
// static const uint8_t EPD_MISO = 19; // Master-In Slave-Out not used, as no data from display
static const uint8_t EPD_MOSI = 13;

EPD_WaveShare42BC epd(EPD_SS, EPD_RST, EPD_DC, EPD_BUSY);
MiniGrafx gfx = MiniGrafx(&epd, BITS_PER_PIXEL, palette);

//################  VERSION  ##########################
std::string version = "1";       // Version of this program
//################ VARIABLES ###########################

const unsigned long UpdateInterval = (30L * 60L - 3) * 1000000L; // Delay between updates, in microseconds, WU allows 500 requests per-day maximum, set to every 15-mins or more
bool LargeIcon   =  true;
bool SmallIcon   =  false;
#define Large  10
#define Small  4
std::string time_str, Day_time_str, rxtext; // strings to hold time and received weather data;
int    wifisection, displaysection, MoonDay, MoonMonth, MoonYear;
int    Sunrise, Sunset;

//################ PROGRAM VARIABLES and OBJECTS ################

typedef struct { // For current Day and Day 1, 2, 3, etc
  std::string   Dt;
  std::string   Period;
  std::string   Trend;
  std::string   Main0;
  std::string   Forecast0;
  std::string   Forecast1;
  std::string   Forecast2;
  std::string   Description;
  std::string   Time;
  std::string   Icon;
  std::string   Country;
  float    lat;
  float    lon;
  float    Temperature;
  float    Humidity;
  float    High;
  float    Low;
  float    Winddir;
  float    Windspeed;
  float    Rainfall;
  float    Snowfall;
  float    Pressure;
  int      Cloudcover;
  int      Visibility;
  int      Sunrise;
  int      Sunset;
} Forecast_record_type;

#define max_readings 24

Forecast_record_type  WxConditions[1];
Forecast_record_type  WxForecast[max_readings];

#define autoscale_on  true
#define autoscale_off false
#define barchart_on   true
#define barchart_off  false

float pressure_readings[max_readings]    = {0};
float temperature_readings[max_readings] = {0};
float rain_readings[max_readings]        = {0};

WiFiClient client; // wifi client object

//#########################################################################################
void begin_sleep(){
  //esp_sleep_enable_timer_wakeup(UpdateInterval);
  //esp_deep_sleep_start(); // Sleep for e.g. 30 minutes

  delay(UpdateInterval);
}
//#########################################################################################
void Display_Weather() {              // 4.2" e-paper display is 400x300 resolution
  Draw_Heading_Section();             // Top line of the display
  Draw_Main_Weather_Section(170, 70); // Centre section of display for Location, temperature, Weather report, current Wx Symbol and wind direction
  Draw_Forecast_Section(230, 18);     // 3hr forecast boxes
  Draw_Astronomy_Section(230, 20);    // Astronomy section Sun rise/set, Moon phase and Moon icon
}
//#########################################################################################
bool obtain_wx_data(std::string RequestType) {
  rxtext = "";
  std::string units = (Units == "M"?"metric":"imperial");
  client.stop(); // close connection before sending a new request
  if (client.connect(server, 80)) { // if the connection succeeds
    // Serial.println("connecting...");
    // send the HTTP PUT request:
    if (RequestType == "weather")
      client.println("GET /data/2.5/" + RequestType + "?q=" + City + "," + Country + "&APPID=" + apikey + "&mode=json&units="+units+"&lang="+Language+" HTTP/1.1");
    else
      client.println("GET /data/2.5/" + RequestType + "?q=" + City + "," + Country + "&APPID=" + apikey + "&mode=json&units="+units+"&lang="+Language+"&cnt=12 HTTP/1.1");
    client.println("Host: api.openweathermap.org");
    client.println("User-Agent: ESP OWM Receiver/1.1");
    client.println("Connection: close");
    client.println();
    // Check HTTP status
    char status[32] = {0};
    client.readBytesUntil('\r', status, sizeof(status));
    if (strcmp(status, "HTTP/1.1 200 OK") != 0) {
      Serial.print(F("Unexpected response: "));
      Serial.println(status);
      return 0;
    }
    // Skip HTTP headers
    char endOfHeaders[] = "\r\n\r\n";
    if (!client.find(endOfHeaders)) {
      Serial.println(F("Invalid response"));
      return 0;
    }
    DecodeWeather(client, RequestType);
    client.stop();
    return true;
  }
  else
  { // if no connection was made
    Serial.println("connection failed");
    return false;
  }
  return true;
}
//#########################################################################################
// Problems with stucturing JSON decodes, see here: https://arduinojson.org/assistant/
bool DecodeWeather(Stream &json_stream, std::string Type) {
  Serial.print("Creating object and ");
  DynamicJsonDocument doc(5*1024);
  DeserializationError error = deserializeJson(doc, json_stream);
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return false;
  }
  // Extract values
  JsonObject& root = doc.as<JsonObject>();
  Serial.println(" Decoding " + Type + " data");
  if (Type == "weather") {
    // All Serial.println statements are for diagnostic purposes and not required, remove if not needed 
    WxConditions[0].lon         = root["coord"]["lon"].as<float>();              Serial.println(WxConditions[0].lon);
    WxConditions[0].lat         = root["coord"]["lat"].as<float>();              Serial.println(WxConditions[0].lat);
    WxConditions[0].Main0       = root["weather"][0]["main"].as<char*>();        Serial.println(WxConditions[0].Main0);
    WxConditions[0].Forecast0   = root["weather"][0]["description"].as<char*>(); Serial.println(WxConditions[0].Forecast0);
    WxConditions[0].Icon        = root["weather"][0]["icon"].as<char*>();        Serial.println(WxConditions[0].Icon);
    WxConditions[0].Forecast1   = root["weather"][1]["main"].as<char*>();        Serial.println(WxConditions[0].Forecast1);
    WxConditions[0].Forecast2   = root["weather"][2]["main"].as<char*>();        Serial.println(WxConditions[0].Forecast2);
    WxConditions[0].Temperature = root["main"]["temp"].as<float>();              Serial.println(WxConditions[0].Temperature);
    WxConditions[0].Pressure    = root["main"]["pressure"].as<float>();          Serial.println(WxConditions[0].Pressure);
    WxConditions[0].Humidity    = root["main"]["humidity"].as<float>();          Serial.println(WxConditions[0].Humidity);
    WxConditions[0].Low         = root["main"]["temp_min"].as<float>();          Serial.println(WxConditions[0].Low);
    WxConditions[0].High        = root["main"]["temp_max"].as<float>();          Serial.println(WxConditions[0].High);
    WxConditions[0].Windspeed   = root["wind"]["speed"].as<float>();             Serial.println(WxConditions[0].Windspeed);
    WxConditions[0].Winddir     = root["wind"]["deg"].as<float>();               Serial.println(WxConditions[0].Winddir);
    WxConditions[0].Cloudcover  = root["clouds"]["all"].as<int>();               Serial.println(WxConditions[0].Cloudcover); // in % of cloud cover
    WxConditions[0].Rainfall    = root["rain"]["3h"].as<float>();                Serial.println(WxConditions[0].Rainfall);   // in mm of rain
    WxConditions[0].Snowfall    = root["snow"]["3h"].as<float>();                Serial.println(WxConditions[0].Snowfall);   // in mm of snow
    WxConditions[0].Visibility  = root["visibility"].as<int>();                  Serial.println(WxConditions[0].Visibility); // in metres
    WxConditions[0].Country     = root["sys"]["country"].as<char*>();            Serial.println(WxConditions[0].Country);
    WxConditions[0].Sunrise     = root["sys"]["sunrise"].as<int>();              Serial.println(WxConditions[0].Sunrise);
    WxConditions[0].Sunset      = root["sys"]["sunset"].as<int>();               Serial.println(WxConditions[0].Sunset);
  }
  if (Type == "forecast") {
    //Serial.println(json);
    const char* cod                 = root["cod"]; // "200"
    float message                   = root["message"]; 
    int cnt                         = root["cnt"]; 
    JsonArray& list                 = root["list"];
    Serial.print("\nReceiving Forecast period - "); //------------------------------------------------
    for (byte r=0; r < max_readings; r++) {
      Serial.println("\nPeriod-"+std::string(r)+"--------------"); 
      WxForecast[r].Dt                = list[r]["dt"].as<char*>(); 
      WxForecast[r].Temperature       = list[r]["main"]["temp"].as<float>();              Serial.println(WxForecast[r].Temperature);
      WxForecast[r].Low               = list[r]["main"]["temp_min"].as<float>();          Serial.println(WxForecast[r].Low);
      WxForecast[r].High              = list[r]["main"]["temp_max"].as<float>();          Serial.println(WxForecast[r].High);
      WxForecast[r].Pressure          = list[r]["main"]["pressure"].as<float>();          Serial.println(WxForecast[r].Pressure);
      WxForecast[r].Humidity          = list[r]["main"]["humidity"].as<float>();          Serial.println(WxForecast[r].Humidity);
      WxForecast[r].Forecast0         = list[r]["weather"][0]["main"].as<char*>();        Serial.println(WxForecast[r].Forecast0);
      WxForecast[r].Forecast0         = list[r]["weather"][1]["main"].as<char*>();        Serial.println(WxForecast[r].Forecast1);
      WxForecast[r].Forecast0         = list[r]["weather"][2]["main"].as<char*>();        Serial.println(WxForecast[r].Forecast2);
      WxForecast[r].Description       = list[r]["weather"][0]["description"].as<char*>(); Serial.println(WxForecast[r].Description);
      WxForecast[r].Icon              = list[r]["weather"][0]["icon"].as<char*>();        Serial.println(WxForecast[r].Icon);
      WxForecast[r].Cloudcover        = list[r]["clouds"]["all"].as<int>();               Serial.println(WxForecast[0].Cloudcover); // in % of cloud cover
      WxForecast[r].Windspeed         = list[r]["wind"]["speed"].as<float>();             Serial.println(WxForecast[r].Windspeed);
      WxForecast[r].Winddir           = list[r]["wind"]["deg"].as<float>();               Serial.println(WxForecast[r].Winddir);
      WxForecast[r].Rainfall          = list[r]["rain"]["3h"].as<float>();                Serial.println(WxForecast[r].Rainfall);
      WxForecast[r].Snowfall          = list[r]["snow"]["3h"].as<float>();                Serial.println(WxForecast[r].Snowfall);
      WxForecast[r].Period            = list[r]["dt_txt"].as<char*>();                    Serial.println(WxForecast[r].Period);
    }
    //------------------------------------------
    float pressure_trend = WxForecast[0].Pressure - WxForecast[1].Pressure; // Measure pressure slope between ~now and later
    pressure_trend = ((int)(pressure_trend * 10)) / 10.0; // Remove any small variations less than 0.1
    WxConditions[0].Trend = "0";
    if (pressure_trend > 0)  WxConditions[0].Trend = "+";
    if (pressure_trend < 0)  WxConditions[0].Trend = "-";
    if (pressure_trend == 0) WxConditions[0].Trend = "0";

    if (Units == "I") Convert_Readings_to_Imperial();
  }
  return true;
}
//#########################################################################################
void Convert_Readings_to_Imperial() {
  WxConditions[0].Pressure    = WxConditions[0].Pressure * 0.02953; //  hPa to ins
  WxForecast[1].Rainfall      = WxForecast[1].Rainfall * 0.0393701; // mm to inches of rainfall
}
//#########################################################################################
int StartWiFi() {
  int connAttempts = 0;
  Serial.print(F("\r\nConnecting to: ")); Serial.println(std::string(ssid1));
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid1, password1);
  while (WiFi.status() != WL_CONNECTED ) {
    delay(500); Serial.print(".");
    if (connAttempts > 20) {
      WiFi.disconnect();
      begin_sleep();
    }
    connAttempts++;
  }
  Serial.println("WiFi connected at: " + std::string(WiFi.localIP()));
  return 1;
}
//#########################################################################################
void StopWiFi() {
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
  wifisection    = millis() - wifisection;
}
//#########################################################################################
void SetupTime() {
  configTime(0, 0, "0.uk.pool.ntp.org", "time.nist.gov");
  setenv("TZ", Timezone, 1);
  delay(500);
  //UpdateLocalTime();
}
//#########################################################################################
void UpdateLocalTime() {
  struct tm timeinfo;
  while (!getLocalTime(&timeinfo)) {
    Serial.println(F("Failed to obtain time"));
  }
  //See http://www.cplusplus.com/reference/ctime/strftime/
  //Serial.println(&timeinfo, "%a %b %d %Y   %H:%M:%S");     // Displays: Saturday, June 24 2017 14:05:49
  Serial.println(&timeinfo, "%H:%M:%S");                     // Displays: 14:05:49
  char output[30], day_output[30];
  if (Units == "M") {
    strftime(day_output, 30, "%a  %d-%b-%y", &timeinfo);     // Displays: Sat 24-Jun-17
    strftime(output, 30, "(@ %H:%M:%S )", &timeinfo);        // Creates: '@ 14:05:49'
  }
  else {
    strftime(day_output, 30, "%a  %b-%d-%y", &timeinfo);     // Creates: Sat Jun-24-17
    strftime(output, 30, "(@ %r )", &timeinfo);              // Creates: '@ 2:05:49pm'
  }
  Day_time_str = day_output;
  time_str     = output;
}
//#########################################################################################
std::string ConvertUnixTime(int unix_time) {
  struct tm *now_tm;
  int hour, min, second, day, month, year, wday;
  // timeval tv = {unix_time,0};
  time_t tm = unix_time;
  now_tm = localtime(&tm);
  hour   = now_tm->tm_hour;
  min    = now_tm->tm_min;
  second = now_tm->tm_sec;
  wday   = now_tm->tm_wday;
  day    = now_tm->tm_mday;
  month  = now_tm->tm_mon + 1;
  year   = 1900 + now_tm->tm_year; // To get just YY information
  MoonDay   = day;
  MoonMonth = month;
  MoonYear  = year;
  if (Units == "M") {
    time_str =  (hour < 10 ? "0" + std::string(hour) : std::string(hour)) + ":" + (min < 10 ? "0" + std::string(min) : std::string(min)) + ":" + "  ";                     // HH:MM   05/07/17
    time_str += (day < 10 ? "0" + std::string(day) : std::string(day)) + "/" + (month < 10 ? "0" + std::string(month) : std::string(month)) + "/" + (year < 10 ? "0" + std::string(year) : std::string(year)); // HH:MM   05/07/17
  }
  else {
    std::string ampm = "am";
    if (hour > 11) ampm = "pm";
    hour = hour % 12; if (hour == 0) hour = 12;
    time_str =  (hour % 12 < 10 ? "0" + std::string(hour % 12) : std::string(hour % 12)) + ":" + (min < 10 ? "0" + std::string(min) : std::string(min)) + ampm + " ";      // HH:MMam 07/05/17
    time_str += (month < 10 ? "0" + std::string(month) : std::string(month)) + "/" + (day < 10 ? "0" + std::string(day) : std::string(day)) + "/" + "/" + (year < 10 ? "0" + std::string(year) : std::string(year)); // HH:MMpm 07/05/17
  }
  // Returns either '21:12  ' or ' 09:12pm' depending on Units
  //Serial.println(time_str);
  return time_str;
}

void setup() {
  Serial.begin(115200);
  StartWiFi();
  SetupTime();
  bool Received_WxData_OK = false;
  Received_WxData_OK = (obtain_wx_data("weather") && obtain_wx_data("forecast"));
  // Now only refresh the screen if all the data was received OK, otherwise wait until the next timed check otherwise wait until the next timed check
  if (Received_WxData_OK) {
    StopWiFi(); // Reduces power consumption
    gfx.init();
    gfx.setRotation(0);
    gfx.setColor(EPD_BLACK);
    gfx.fillBuffer(EPD_WHITE);
    gfx.setTextAlignment(TEXT_ALIGN_LEFT);
    Display_Weather();
    DrawBattery(SCREEN_WIDTH-80, 0);
    gfx.commit();
    delay(2000);
  }
  Serial.println(F("Starting deep-sleep period..."));
  begin_sleep();
}
//#########################################################################################
void loop() { // this will never run!
}