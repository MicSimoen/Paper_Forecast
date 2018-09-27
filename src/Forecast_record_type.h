#ifndef FORECAST_RECORD_TYPE_H
#define FORECAST_RECORD_TYPE_H

#include <Arduino.h>

typedef struct { // For current Day and Day 1, 2, 3, etc
  String   Dt;
  String   Period;
  String   Trend;
  String   Main0;
  String   Forecast0;
  String   Forecast1;
  String   Forecast2;
  String   Description;
  String   Time;
  String   Icon;
  String   Country;
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

#endif // FORECAST_RECORD_TYPE_H