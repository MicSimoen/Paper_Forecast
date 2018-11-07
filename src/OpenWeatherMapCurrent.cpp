/**The MIT License (MIT)
 
 Copyright (c) 2018 by ThingPulse Ltd., https://thingpulse.com
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 */

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#include "OpenWeatherMapCurrent.h"
#include "Forecast_record_type.h"

OpenWeatherMapCurrent::OpenWeatherMapCurrent() {

}

void OpenWeatherMapCurrent::updateCurrent(Forecast_record_type *data, String appId, String location) {
  doUpdate(data, buildUrl(appId, "q=" + location));
}

void OpenWeatherMapCurrent::updateCurrentById(Forecast_record_type *data, String appId, String locationId) {
  doUpdate(data, buildUrl(appId, "id=" + locationId));
}

String OpenWeatherMapCurrent::buildUrl(String appId, String locationParameter) {
  String units = metric ? "metric" : "imperial";
  return "http://api.openweathermap.org/data/2.5/weather?" + locationParameter + "&appid=" + appId + "&units=" + units + "&lang=" + language;
}

void OpenWeatherMapCurrent::doUpdate(Forecast_record_type *data, String url) {
  unsigned long lostTest = 10000UL;
  unsigned long lost_do = millis();
  this->weatherItemCounter = 0;
  this->data = data;
  JsonStreamingParser parser;
  parser.setListener(this);
  Serial.printf("Getting url: %s\n", url.c_str());
  HTTPClient http;

  http.begin(url);
  bool isBody = false;
  char c;
  int size;
  Serial.print("[HTTP] GET...\n");
  // start connection and send HTTP header
  int httpCode = http.GET();
  Serial.printf("[HTTP] GET... code: %d\n", httpCode);
  if(httpCode > 0) {

    WiFiClient * client = http.getStreamPtr();

    while(client->connected() || client->available()) {
      while((size = client->available()) > 0) {
		if ((millis() - lost_do) > lostTest) {
			Serial.println ("lost in client with a timeout");
			client->stop();
			ESP.restart();
	    }
        c = client->read();
        if (c == '{' || c == '[') {

          isBody = true;
        }
        if (isBody) {
          parser.parse(c);
        }
        // give WiFi and TCP/IP libraries a chance to handle pending events
        yield();
      }
    }
  }
  this->data = nullptr;
}

void OpenWeatherMapCurrent::whitespace(char c) {
  Serial.println("whitespace");
}

void OpenWeatherMapCurrent::startDocument() {
  Serial.println("start document");
}

void OpenWeatherMapCurrent::key(String key) {
  currentKey = String(key);
}

void OpenWeatherMapCurrent::value(String value) {
  // "lon": 8.54, float lon;
  if (currentKey == "lon") {
    this->data->lon = value.toFloat();
  }
  // "lat": 47.37 float lat;
  if (currentKey == "lat") {
    this->data->lat = value.toFloat();
  }
  // weatherItemCounter: only get the first item if more than one is available
  if (currentParent == "weather" && weatherItemCounter == 0) {
    // "main": "Rain", String main;
    if (currentKey == "main") {
      this->data->Main0 = value;
    }
    // "description": "shower rain", String description;
    if (currentKey == "description") {
      this->data->Forecast0 = value;
    }
    // "icon": "09d" String icon;
   //String iconMeteoCon;
    if (currentKey == "icon") {
      this->data->Icon = value;
    }

  }
  if (currentParent == "weather" && weatherItemCounter == 1) {
    // "description": "shower rain", String description;
    if (currentKey == "description") {
      this->data->Forecast1 = value;
    }
  }
  if (currentParent == "weather" && weatherItemCounter == 2) {
    // "description": "shower rain", String description;
    if (currentKey == "description") {
      this->data->Forecast2 = value;
    }
  }
  
  if (currentParent == "rain") {
    // "description": "shower rain", String description;
    if (currentKey == "3h") {
      this->data->Rainfall = value.toFloat();
    }
  }
  if (currentParent == "snow") {
    // "description": "shower rain", String description;
    if (currentKey == "3h") {
      this->data->Snowfall = value.toFloat();
    }
  }

  // "temp": 290.56, float temp;
  if (currentKey == "temp") {
    this->data->Temperature = value.toFloat();
  }
  // "pressure": 1013, uint16_t pressure;
  if (currentKey == "pressure") {
    this->data->Pressure = value.toInt();
  }
  // "humidity": 87, uint8_t humidity;
  if (currentKey == "humidity") {
    this->data->Humidity = value.toInt();
  }
  // "temp_min": 289.15, float tempMin;
  if (currentKey == "temp_min") {
    this->data->Low = value.toFloat();
  }
  // "temp_max": 292.15 float tempMax;
  if (currentKey == "temp_max") {
    this->data->High = value.toFloat();
  }
  // visibility: 10000, uint16_t visibility;
  if (currentKey == "visibility") {
    this->data->Visibility = value.toInt();
  }
  // "wind": {"speed": 1.5}, float windSpeed;
  if (currentKey == "speed") {
    this->data->Windspeed = value.toFloat();
  }
  // "wind": {deg: 226.505}, float windDeg;
  if (currentKey == "deg") {
    this->data->Winddir = value.toFloat();
  }
  // "clouds": {"all": 90}, uint8_t clouds;
  if (currentKey == "all") {
    this->data->Cloudcover = value.toInt();
  }
  // "country": "CH", String country;
  if (currentKey == "country") {
    this->data->Country = value;
  }
  // "sunrise": 1526960448, uint32_t sunrise;
  if (currentKey == "sunrise") {
    this->data->Sunrise = value.toInt();
  }
  // "sunset": 1527015901 uint32_t sunset;
  if (currentKey == "sunset") {
    this->data->Sunset = value.toInt();
  }
}

void OpenWeatherMapCurrent::endArray() {

}


void OpenWeatherMapCurrent::startObject() {
  currentParent = currentKey;
}

void OpenWeatherMapCurrent::endObject() {
  if (currentParent == "weather") {
    weatherItemCounter++;
  }
  currentParent = "";
}

void OpenWeatherMapCurrent::endDocument() {

}

void OpenWeatherMapCurrent::startArray() {

}


String OpenWeatherMapCurrent::getMeteoconIcon(String icon) {
 	// clear sky
  // 01d
  if (icon == "01d") 	{
    return "B";
  }
  // 01n
  if (icon == "01n") 	{
    return "C";
  }
  // few clouds
  // 02d
  if (icon == "02d") 	{
    return "H";
  }
  // 02n
  if (icon == "02n") 	{
    return "4";
  }
  // scattered clouds
  // 03d
  if (icon == "03d") 	{
    return "N";
  }
  // 03n
  if (icon == "03n") 	{
    return "5";
  }
  // broken clouds
  // 04d
  if (icon == "04d") 	{
    return "Y";
  }
  // 04n
  if (icon == "04n") 	{
    return "%";
  }
  // shower rain
  // 09d
  if (icon == "09d") 	{
    return "R";
  }
  // 09n
  if (icon == "09n") 	{
    return "8";
  }
  // rain
  // 10d
  if (icon == "10d") 	{
    return "Q";
  }
  // 10n
  if (icon == "10n") 	{
    return "7";
  }
  // thunderstorm
  // 11d
  if (icon == "11d") 	{
    return "P";
  }
  // 11n
  if (icon == "11n") 	{
    return "6";
  }
  // snow
  // 13d
  if (icon == "13d") 	{
    return "W";
  }
  // 13n
  if (icon == "13n") 	{
    return "#";
  }
  // mist
  // 50d
  if (icon == "50d") 	{
    return "M";
  }
  // 50n
  if (icon == "50n") 	{
    return "M";
  }
  // Nothing matched: N/A
  return ")";

}
