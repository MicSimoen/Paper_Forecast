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

#include <Arduino.h>
#include "owm_credentials2.h"  // See 'owm_credentials' tab and enter your OWM API key and set the Wifi SSID and PASSWORD
#include "settings.h"
#include "Forecast_record_type.h"
#include "draw_manager.h"
#include <ArduinoJson.h>       // https://github.com/bblanchon/ArduinoJson NOTE: *** MUST BE Version-6 or above ***
#include <ESP8266WiFi.h>       // Built-in
#include "time.h"              // Built-in
#include <SPI.h>               // Built-in 

//################  VERSION  ##########################
String version = "1";       // Version of this program
//################ VARIABLES ###########################

const unsigned long UpdateInterval = (30L * 60L - 3) * 1000000L; // Delay between updates, in microseconds, WU allows 500 requests per-day maximum, set to every 15-mins or more
bool LargeIcon   =  true;
String time_str, Day_time_str, rxtext; // strings to hold time and received weather data;
int    wifisection, displaysection, MoonDay, MoonMonth, MoonYear;
int    Sunrise, Sunset;

//################ PROGRAM VARIABLES and OBJECTS ################

Forecast_record_type  WxConditions[1];
Forecast_record_type  WxForecast[MAX_READINGS];

float pressure_readings[MAX_READINGS]    = {0};
float temperature_readings[MAX_READINGS] = {0};
float rain_readings[MAX_READINGS]        = {0};

WiFiClient client; // wifi client object

//#########################################################################################
void begin_sleep(){
  //esp_sleep_enable_timer_wakeup(UpdateInterval);
  //esp_deep_sleep_start(); // Sleep for e.g. 30 minutes

  delay(UpdateInterval);
}
//#########################################################################################
void Display_Weather(draw_manager *Draw_Manager) {              // 4.2" e-paper display is 400x300 resolution
  Draw_Manager->Draw_Heading_Section(Day_time_str, time_str);             // Top line of the display
  Draw_Manager->Draw_Main_Weather_Section(170, 70, WxConditions); // Centre section of display for Location, temperature, Weather report, current Wx Symbol and wind direction
  //Draw_Manager->Draw_Forecast_Section(230, 18,
  //  pressure_readings, rain_readings, temperature_readings, WxForecast);     // 3hr forecast boxes
  // Draw_Manager->Draw_Astronomy_Section(230, 20, WxConditions);    // Astronomy section Sun rise/set, Moon phase and Moon icon
}
//#########################################################################################
void Convert_Readings_to_Imperial() {
  WxConditions[0].Pressure    = WxConditions[0].Pressure * 0.02953; //  hPa to ins
  WxForecast[1].Rainfall      = WxForecast[1].Rainfall * 0.0393701; // mm to inches of rainfall
}
//#########################################################################################
// Problems with stucturing JSON decodes, see here: https://arduinojson.org/assistant/
bool DecodeWeather(Stream &json_stream, String Type) {
  Serial.print("Creating object and ");
  DynamicJsonDocument doc(5*1024);
  DeserializationError error = deserializeJson(doc, json_stream);
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return false;
  }
  // Extract values
  JsonObject root = doc.as<JsonObject>();
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
    JsonArray list                 = root["list"];
    Serial.print("\nReceiving Forecast period - "); //------------------------------------------------
    for (byte r=0; r < MAX_READINGS; r++) {
      Serial.println("\nPeriod-"+String(r)+"--------------"); 
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

    if (UNITS == "I") Convert_Readings_to_Imperial();
  }
  return true;
}
//#########################################################################################
int StartWiFi() {
  int connAttempts = 0;
  Serial.print(F("\r\nConnecting to: ")); Serial.println(String(ssid1));
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
  Serial.println("WiFi connected at: " + String(WiFi.localIP()));
  return 1;
}
//#########################################################################################
void StopWiFi() {
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
  wifisection    = millis() - wifisection;
}
//#########################################################################################
void UpdateLocalTime() {
    struct tm *timeinfo;

    time_t now = time(nullptr);
    timeinfo = localtime (&now);

    //See http://www.cplusplus.com/reference/ctime/strftime/
    //Serial.println(timeinfo, "%a %b %d %Y   %H:%M:%S");     // Displays: Saturday, June 24 2017 14:05:49
    //Serial.println(&timeinfo, "%H:%M:%S");                     // Displays: 14:05:49
    char output[30], day_output[30];
    if (UNITS == "M") {
        strftime(day_output, 30, "%a  %d-%b-%y", timeinfo);     // Displays: Sat 24-Jun-17
        strftime(output, 30, "(@ %H:%M:%S )", timeinfo);        // Creates: '@ 14:05:49'
    }
    else {
        strftime(day_output, 30, "%a  %b-%d-%y", timeinfo);     // Creates: Sat Jun-24-17
        strftime(output, 30, "(@ %r )", timeinfo);              // Creates: '@ 2:05:49pm'
    }
    Day_time_str = day_output;
    time_str     = output;
}
//#########################################################################################
void SetupTime() {
  configTime(7200, 3600, "0.se.pool.ntp.org", "0.europe.pool.ntp.org", "time.nist.gov");
  //setenv("TZ", TIMEZONE, 1);
  while (!time(nullptr)) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("NTP Time Received!");
  UpdateLocalTime();
}
//#########################################################################################
bool obtain_wx_data(String RequestType) {
  rxtext = "";
  String units = (UNITS == "M"?"metric":"imperial");
  client.stop(); // close connection before sending a new request
  if (client.connect(server, 80)) { // if the connection succeeds
    // Serial.println("connecting...");
    // send the HTTP PUT request:
    if (RequestType == "weather")
      client.println("GET /data/2.5/" + RequestType + "?q=" + CITY + "," + COUNTRY + "&APPID=" + 
      apikey + "&mode=json&units="+units+"&lang="+LANGUAGE+" HTTP/1.1");
    else
      client.println("GET /data/2.5/" + RequestType + "?q=" + CITY + "," + COUNTRY + "&APPID=" + 
      apikey + "&mode=json&units="+units+"&lang="+LANGUAGE+"&cnt=12 HTTP/1.1");
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
void setup() {
  Serial.begin(115200);
  StartWiFi();
  SetupTime();
  draw_manager Draw_Manager(SCREEN_WIDTH, SCREEN_HEIGHT, BITS_PER_PIXEL, PALETTE);
  bool Received_WxData_OK = false;
  Received_WxData_OK = (obtain_wx_data("weather") && obtain_wx_data("forecast"));
  // Now only refresh the screen if all the data was received OK, otherwise wait until the next timed check otherwise wait until the next timed check
  if (Received_WxData_OK) {
    StopWiFi(); // Reduces power consumption
    Display_Weather(&Draw_Manager);
    Draw_Manager.DrawBattery(SCREEN_WIDTH-80, 0);
    Draw_Manager.Commit();
    delay(2000);
  }
  UpdateLocalTime();
  Serial.println(F("Starting deep-sleep period..."));
  begin_sleep();
}
//#########################################################################################
void loop() { // this will never run!
}