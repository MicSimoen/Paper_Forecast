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
#include "utils.h"
#include "Forecast_record_type.h"
#include "OpenWeatherMapForecast.h"
#include "OpenWeatherMapCurrent.h"
#include "draw_manager.h"
//#include <ArduinoJson.h>       // https://github.com/bblanchon/ArduinoJson NOTE: *** MUST BE Version-6 or above ***
#include <ESP8266WiFi.h>       // Built-in
//#include "time.h"              // Built-in
#include <SPI.h>               // Built-in

//################  VERSION  ##########################
String version = "1";       // Version of this program
//################ VARIABLES ###########################

String time_str, Day_time_str, rxtext; // strings to hold time and received weather data;
int    wifisection, displaysection, MoonDay, MoonMonth, MoonYear;
int    Sunrise, Sunset;

//################ PROGRAM VARIABLES and OBJECTS ################

Forecast_record_type  WxConditions;
Forecast_record_type  WxForecast[MAX_READINGS];

float pressure_readings[MAX_READINGS]    = {0};
float temperature_readings[MAX_READINGS] = {0};
float rain_readings[MAX_READINGS]        = {0};

WiFiClient client; // wifi client object

// initiate the client
OpenWeatherMapForecast forecast_client;
OpenWeatherMapCurrent current_client;

//#########################################################################################
void begin_sleep(){
    Serial.println(F("Starting deep-sleep period..."));
    Serial.println("+---------------------------------------------+");
    ESP.deepSleep(UPDATE_INTERVAL);
    delay(100);
}
//#########################################################################################
void Display_Weather(draw_manager *Draw_Manager) {              // 4.2" e-paper display is 400x300 resolution
  Serial.println("Drawing weather to buffer");
  Draw_Manager->Draw_Heading_Section(Day_time_str, time_str);             // Top line of the display
  Draw_Manager->Draw_Weather(&WxConditions, WxForecast);
}
//#########################################################################################
int StartWiFi() {
  int connAttempts = 0;
  Serial.print(F("Connecting to: ")); Serial.println(String(ssid1));
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
  IPAddress ip = WiFi.localIP();
  String ip_str  =String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
  Serial.println("\nWiFi connected at: " + ip_str);
  return 1;
}
//#########################################################################################
void StopWiFi() {
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
  wifisection    = millis() - wifisection;
}
//#########################################################################################
void setup() {
  Serial.begin(115200);
  Serial.println("\r\nWaking up");

  StartWiFi();
  utils::SetupTime();
  utils::UpdateLocalTime(Day_time_str, time_str);
  draw_manager Draw_Manager(SCREEN_WIDTH, SCREEN_HEIGHT, BITS_PER_PIXEL, PALETTE);
  
  Serial.println("Requesting current weather conditions");
  current_client.setMetric((UNITS == "M"));
  current_client.setLanguage(LANGUAGE);
  current_client.updateCurrent(&WxConditions, apikey, CITY + "," + COUNTRY);
  
  Serial.println("Requesting weather forecast");
  forecast_client.setMetric((UNITS == "M"));
  forecast_client.setLanguage(LANGUAGE);
  forecast_client.updateForecasts(WxForecast, apikey, CITY + "," + COUNTRY, MAX_READINGS);
 
  StopWiFi(); // Reduces power consumption
  Display_Weather(&Draw_Manager);
  // Draw_Manager.DrawBattery(SCREEN_WIDTH-80, 0);
  Draw_Manager.Commit();
  delay(2000);

  utils::UpdateLocalTime(Day_time_str, time_str);
  begin_sleep();
}
//#########################################################################################
void loop() { // this will never run!
}