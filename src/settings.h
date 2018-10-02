#ifndef SETTINGS_H
#define SETTINGS_H

#include <Arduino.h>
#include <stdint.h>

static int SMALL = 4;
static int LARGE = 14;
static float SCREEN_WIDTH = 400.0;    // Set for landscape mode, don't remove the decimal place!
static float SCREEN_HEIGHT = 300.0;
static uint8_t BITS_PER_PIXEL = 1;
static int EPD_BLACK = 0;
static int EPD_WHITE = 1;
static uint16_t PALETTE[2] = {0, 1 };

static bool MAIN_WEATHER_LARGE_ICON = true;
static bool FORECAST_LARGE_ICON = false;
static bool PRES_AUTOSCALE_ON = true;
static bool PRES_BARCHART_ON = false;
static bool TEMP_AUTOSCALE_ON = true;
static bool TEMP_BARCHART_ON = false;
static bool RAIN_AUTOSCALE_ON = true;
static bool RAIN_BARCHART_ON = true;

static const int MAX_READINGS = 12;
//http://api.openweathermap.org/data/2.5/forecast?q=Melksham,UK&APPID=your_OWM_API_key&mode=json&units=metric&cnt=40
//http://api.openweathermap.org/data/2.5/weather?q=Melksham,UK&APPID=your_OWM_API_key&mode=json&units=metric&cnt=1
//Set your location according to OWM locations
static String CITY          = "Goeteborg";                      // Your home city See: http://bulk.openweathermap.org/sample/
static String COUNTRY       = "SE";                            // Your country  
static String LANGUAGE      = "EN";                            // NOTE: Only the weather description (not used) is translated by OWM
                                                        // Arabic (AR) Czech (CZ) English (EN) Greek (EL) Persian(Farsi) (FA) Galician (GL) Hungarian (HU) Japanese (JA)
                                                        // Korean (KR) Latvian (LA) Lithuanian (LT) Macedonian (MK) Slovak (SK) Slovenian (SL) Vietnamese (VI)
static String HEMISPHERE    = "north";                         // or "south"  
static String UNITS         = "M";                             // Use 'M' for Metric or I for Imperial 
static char* TIMEZONE = "CET-1CEST,M3.5.0,M10.5.0/3";  // Choose your time zone from: https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv 
// Example time zones
//char* Timezone = "MET-1METDST,M3.5.0/01,M10.5.0/02"; // Most of Europe
//char* Timezone = "CET-1CEST,M3.5.0,M10.5.0/3";       // Central Europe
//char* Timezone = "EST-2METDST,M3.5.0/01,M10.5.0/02"; // Most of Europe
//char* Timezone = "EST5EDT,M3.2.0,M11.1.0";           // EST USA  
//char* Timezone = "CST6CDT,M3.2.0,M11.1.0";           // CST USA
//char* Timezone = "MST7MDT,M4.1.0,M10.5.0";           // MST USA
//char* Timezone = "NZST-12NZDT,M9.5.0,M4.1.0/3";      // Auckland
//char* Timezone = "EET-2EEST,M3.5.5/0,M10.5.5/0";     // Asia
//char* Timezone = "ACST-9:30ACDT,M10.1.0,M4.1.0/3":   // Australia



// LOLIN D1 MINI PRO
static uint8_t EPD_BUSY = 4;
static uint8_t EPD_SS   = 15;
static uint8_t EPD_RST  = 2;
static uint8_t EPD_DC   = 0;
static uint8_t EPD_SCK  = 14;
// static uint8_t EPD_MISO = 19; // Master-In Slave-Out not used, as no data from display
static uint8_t EPD_MOSI = 13;

#endif // SETTINGS_H