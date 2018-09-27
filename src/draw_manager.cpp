#include "draw_manager.h"

#include <Arduino.h>

#include "settings.h"
#include "utils.h"
#include "Forecast_record_type.h"
#include "EPD_WaveShare_42BC.h"  // Copyright (c) 2017 by Daniel Eichhorn https://github.com/ThingPulse/minigrafx
#include "MiniGrafx.h"         // Copyright (c) 2017 by Daniel Eichhorn https://github.com/ThingPulse/minigrafx
#include "DisplayDriver.h"     // Copyright (c) 2017 by Daniel Eichhorn https://github.com/ThingPulse/minigrafx
#include "ArialRounded.h"      // Copyright (c) 2017 by Daniel Eichhorn https://github.com/ThingPulse/minigrafx

draw_manager::draw_manager(float screen_width_in,
                           float screen_height_in,
                           uint8_t bits_per_pixel_in,
                           uint16_t *palette_in) {
    screen_width = screen_width_in;
    screen_height = screen_height_in;
    bits_per_pixel = bits_per_pixel_in;
    palette = palette_in;
    EPaperDisplay = new EPD_WaveShare42BC(EPD_SS, EPD_RST, EPD_DC, EPD_BUSY);
    gfx = new MiniGrafx(EPaperDisplay, bits_per_pixel, palette);
    gfx->init();
    gfx->setRotation(0);
    gfx->setColor(EPD_BLACK);
    gfx->fillBuffer(EPD_WHITE);
    gfx->setTextAlignment(TEXT_ALIGN_LEFT);
}

void draw_manager::Commit(void){
    gfx->commit();
}

void draw_manager::Draw_Heading_Section(String Day_time_str, String time_str) {
    gfx->setFont(ArialRoundedMTBold_14);
    gfx->setTextAlignment(TEXT_ALIGN_CENTER);
    gfx->drawString(screen_width / 2, -2, CITY);
    gfx->setFont(ArialMT_Plain_10);
    gfx->setTextAlignment(TEXT_ALIGN_RIGHT);
    gfx->drawString(screen_width-3, 0, Day_time_str);
    gfx->setTextAlignment(TEXT_ALIGN_LEFT);
    gfx->drawString(5, 0, time_str);
    gfx->drawLine(0, 15, screen_width, 15);
}
//#########################################################################################
void draw_manager::Draw_Main_Weather_Section(int x,
                                             int y,
                                             Forecast_record_type *WxConditions) {
    DisplayWXicon(x+5, y-5, WxConditions[0].Icon, MAIN_WEATHER_LARGE_ICON);
    gfx->setFont(ArialRoundedMTBold_14);
    DrawPressureTrend(x, y + 50, WxConditions[0].Pressure, WxConditions[0].Trend);
    gfx->setTextAlignment(TEXT_ALIGN_CENTER);
    Draw_Rain(x - 100, y + 35, WxConditions);
    gfx->setFont(ArialMT_Plain_24);
    gfx->setTextAlignment(TEXT_ALIGN_LEFT);
    String Wx_Description = WxConditions[0].Main0;
    if (WxConditions[0].Forecast0 != "") {
        Wx_Description += " (" +  WxConditions[0].Forecast0;
        if (WxConditions[0].Forecast1 != "" && \
            WxConditions[0].Forecast1 != WxConditions[0].Forecast2) {
            Wx_Description += " & " +  WxConditions[0].Forecast1 + ")";
        } else {
            Wx_Description += ")";
        }
    }
    gfx->drawString(x - 170, y + 70, utils::TitleCase(Wx_Description));
    Draw_Main_Wx(x -98, y - 1, WxConditions);
    gfx->drawLine(0, y + 68, screen_width, y + 68);
}
//#########################################################################################
void draw_manager::Draw_Forecast_Section(int x,
                                         int y,
                                         float *pressure_readings,
                                         float *rain_readings,
                                         float *temperature_readings,
                                         Forecast_record_type *WxForecast) {
  gfx->setFont(ArialMT_Plain_10);
  Draw_Forecast_Weather(x, y, 0, WxForecast);
  Draw_Forecast_Weather(x + 56, y, 1, WxForecast);
  Draw_Forecast_Weather(x + 112, y, 2, WxForecast);
  //       (x,y,width,height,MinValue, MaxValue, Title, Data Array, AutoScale, ChartMode)
  for (int r = 1; r <= MAX_READINGS; r++) {
    if (UNITS == "I") pressure_readings[r] = WxForecast[r].Pressure * 0.02953;  
    else              pressure_readings[r] = WxForecast[r].Pressure;
    temperature_readings[r] = WxForecast[r].Temperature;
    if (UNITS == "I") rain_readings[r]     = WxForecast[r].Rainfall * 0.0393701;
    else              rain_readings[r]     = WxForecast[r].Rainfall;
  }
  gfx->drawLine(0, y + 173, screen_width, y + 173);
  gfx->setTextAlignment(TEXT_ALIGN_CENTER);
  gfx->setFont(ArialRoundedMTBold_14);
  gfx->drawString(x - 40, y + 173, "3-Day Forecast Values");
  gfx->setTextAlignment(TEXT_ALIGN_LEFT);
  gfx->setFont(ArialMT_Plain_10);
  DrawGraph(screen_width/400*30, screen_height/300*222, screen_width/400*100, screen_height/300*60,
            900,1050,"Pressure", pressure_readings, MAX_READINGS,
            PRES_AUTOSCALE_ON, PRES_BARCHART_ON);
  DrawGraph(screen_width/400*158, screen_height/300*222, screen_width/400*100, screen_height/300*60,
            10,30, "Temperature", temperature_readings, MAX_READINGS,
            TEMP_AUTOSCALE_ON, TEMP_BARCHART_ON);
  DrawGraph(screen_width/400*288, screen_height/300*222, screen_width/400*100, screen_height/300*60,
            0,30, "Rainfall", rain_readings, MAX_READINGS,
            RAIN_AUTOSCALE_ON, RAIN_BARCHART_ON);
}
//#########################################################################################
void draw_manager::Draw_Forecast_Weather(int x,
                                         int y,
                                         int index,
                                         Forecast_record_type *WxForecast) {
  gfx->setFont(ArialMT_Plain_10);
  gfx->setColor(EPD_BLACK); // Sometimes gets set to WHITE, so change back
  gfx->drawRect(x, y, 55, 65);
  gfx->drawLine(x + 1, y + 13, x + 55, y + 13);
  DisplayWXicon(x + 28, y + 35, WxForecast[index].Icon, FORECAST_LARGE_ICON);
  gfx->setTextAlignment(TEXT_ALIGN_CENTER);
  gfx->setFont(ArialMT_Plain_10);
  gfx->drawString(x + 28, y, String(WxForecast[index].Period.substring(11,16)));
  gfx->drawString(x + 28, y + 50, String(WxForecast[index].High,0) + "° / " + String(WxForecast[index].Low,0) + "°");
}
//#########################################################################################
void draw_manager::Draw_Main_Wx(int x, int y, Forecast_record_type *WxConditions) {
  DrawWind(x, y, WxConditions[0].Winddir, WxConditions[0].Windspeed);
  gfx->setTextAlignment(TEXT_ALIGN_CENTER);
  gfx->setFont(ArialRoundedMTBold_14);
  gfx->drawString(x, y - 28, String(WxConditions[0].High,0) + "° | " + String(WxConditions[0].Low,0) + "°"); // Show forecast high and Low
  gfx->setFont(ArialMT_Plain_24);
  gfx->drawString(x - 5, y - 10, String(WxConditions[0].Temperature,1) + "°"); // Show current Temperature
  gfx->setFont(ArialRoundedMTBold_14);
  gfx->setTextAlignment(TEXT_ALIGN_LEFT);  
  gfx->drawString(x+String(WxConditions[0].Temperature,1).length()*11/2,y-9,UNITS=="M"?"C":"F"); // Add in smaller Temperature unit
  gfx->setFont(ArialRoundedMTBold_14);
  gfx->setTextAlignment(TEXT_ALIGN_LEFT);
}
//#########################################################################################
void draw_manager::DrawWind(int x, int y, float angle, float windspeed) {
  int Cradius = 44;
  float dx = Cradius * cos((angle - 90) * PI / 180) + x; // calculate X position
  float dy = Cradius * sin((angle - 90) * PI / 180) + y; // calculate Y position
  arrow(x, y, Cradius - 3, angle, 15, 15); // Show wind direction on outer circle
  gfx->drawCircle(x, y, Cradius + 2);
  gfx->drawCircle(x, y, Cradius + 3);
  gfx->setTextAlignment(TEXT_ALIGN_CENTER);
  gfx->setFont(ArialRoundedMTBold_14);
  gfx->drawString(x, y + Cradius - 25, utils::WindDegToDirection(angle));
  gfx->setFont(ArialMT_Plain_10);
  gfx->drawString(x - Cradius + 3, y - Cradius - 6, String(windspeed,1) + (UNITS == "M" ? " m/s" : " mph"));
  gfx->setTextAlignment(TEXT_ALIGN_LEFT);
}
//#########################################################################################
void draw_manager::DrawPressureTrend(int x, int y, float pressure, String slope) {
  gfx->setTextAlignment(TEXT_ALIGN_CENTER);
  gfx->drawString(x, y, String(pressure,1) + (UNITS == "M" ? "mb" : "in"));
  x = x + 45; y = y + 8;
  if      (slope == "+") {
    gfx->drawLine(x,  y,  x + 4, y - 4);
    gfx->drawLine(x + 4, y - 4, x + 8, y);
  }
  else if (slope == "0") {
    gfx->drawLine(x + 3, y - 4, x + 8, y);
    gfx->drawLine(x + 3, y + 4, x + 8, y);
  }
  else if (slope == "-") {
    gfx->drawLine(x,  y,  x + 4, y + 4);
    gfx->drawLine(x + 4, y + 4, x + 8, y);
  }
}
//#########################################################################################
void draw_manager::Draw_Rain(int x, int y, Forecast_record_type *WxForecast) {
    gfx->setFont(ArialRoundedMTBold_14);
    gfx->setTextAlignment(TEXT_ALIGN_CENTER);
    if (WxForecast[1].Rainfall > 0)
    {
        gfx->drawString(x, y + 14,
            String(WxForecast[1].Rainfall,3) + (UNITS == "M" ? "mm" : "in") + " Rainfall");
    } // Only display rainfall total today if > 0
    gfx->setFont(ArialMT_Plain_10);
}
//#########################################################################################
void draw_manager::Draw_Astronomy_Section(int x,
                                          int y,
                                          Forecast_record_type *WxConditions) {
  gfx->setTextAlignment(TEXT_ALIGN_LEFT);
  gfx->setFont(ArialMT_Plain_10);
  gfx->drawRect(x, y + 64, 167, 53);
  gfx->drawString(x + 4, y + 65, "Sun Rise/Set");
  int MoonDay, MoonMonth, MoonYear;
  gfx->drawString(x + 20, y + 75, utils::ConvertUnixTime(WxConditions[0].Sunrise,
                                                         UNITS,
                                                         MoonDay,
                                                         MoonMonth,
                                                         MoonYear).substring(0, 5));
  gfx->drawString(x + 20, y + 86, utils::ConvertUnixTime(WxConditions[0].Sunset,
                                                         UNITS,
                                                         MoonDay,
                                                         MoonMonth,
                                                         MoonYear).substring(0, 5));
  gfx->drawString(x + 4, y + 100, "Moon:");
  gfx->drawString(x + 35, y + 100, utils::MoonPhase(MoonDay, MoonMonth, MoonYear, HEMISPHERE));
  DrawMoon(x + 103, y + 51, MoonDay, MoonMonth, MoonYear, HEMISPHERE);
}
//#########################################################################################
void draw_manager::DrawMoon(int x, int y, int dd, int mm, int yy, String hemisphere) {
  int diameter = 38;
  float Xpos, Ypos, Rpos, Xpos1, Xpos2, ip, ag;
  gfx->setColor(EPD_BLACK);
  for (Ypos = 0; Ypos <= 45; Ypos++) {
    Xpos = sqrt(45 * 45 - Ypos * Ypos);
    // Draw dark part of moon
    double pB1x = (90   - Xpos) / 90 * diameter + x;
    double pB1y = (Ypos + 90) / 90   * diameter + y;
    double pB2x = (Xpos + 90) / 90   * diameter + x;
    double pB2y = (Ypos + 90) / 90   * diameter + y;
    double pB3x = (90   - Xpos) / 90 * diameter + x;
    double pB3y = (90   - Ypos) / 90 * diameter + y;
    double pB4x = (Xpos + 90) / 90   * diameter + x;
    double pB4y = (90   - Ypos) / 90 * diameter + y;
    gfx->setColor(EPD_BLACK);
    gfx->drawLine(pB1x, pB1y, pB2x, pB2y);
    gfx->drawLine(pB3x, pB3y, pB4x, pB4y);
    // Determine the edges of the lighted part of the moon
    int j = utils::JulianDate(dd, mm, yy);
    //Calculate the approximate phase of the moon
    double Phase = (j + 4.867) / 29.53059;
    Phase = Phase - (int)Phase;
    if (Phase < 0.5) ag = Phase * 29.53059 + 29.53059 / 2; else ag = Phase * 29.53059 - 29.53059 / 2; // Moon's age in days
    if (hemisphere == "south") Phase = 1 - Phase;
    Rpos = 2 * Xpos;
    if (Phase < 0.5) {
      Xpos1 = - Xpos;
      Xpos2 = (Rpos - 2 * Phase * Rpos - Xpos);
    }
    else {
      Xpos1 = Xpos;
      Xpos2 = (Xpos - 2 * Phase * Rpos + Rpos);
    }
    // Draw light part of moon
    double pW1x = (Xpos1 + 90) / 90 * diameter + x;
    double pW1y = (90 - Ypos) / 90  * diameter + y;
    double pW2x = (Xpos2 + 90) / 90 * diameter + x;
    double pW2y = (90 - Ypos) / 90  * diameter + y;
    double pW3x = (Xpos1 + 90) / 90 * diameter + x;
    double pW3y = (Ypos + 90) / 90  * diameter + y;
    double pW4x = (Xpos2 + 90) / 90 * diameter + x;
    double pW4y = (Ypos + 90) / 90  * diameter + y;
    gfx->setColor(EPD_WHITE);
    gfx->drawLine(pW1x, pW1y, pW2x, pW2y);
    gfx->drawLine(pW3x, pW3y, pW4x, pW4y);
  }
  gfx->setColor(EPD_BLACK);
  gfx->drawCircle(x + diameter - 1, y + diameter, diameter / 2 + 1);
}
//#########################################################################################
void draw_manager::DrawCircle(int x, int y, int Cstart, int Cend, int Cradius, int Coffset_radius, int Coffset) {
  gfx->setColor(EPD_BLACK);
  float dx, dy;
  for (int i = Cstart; i < Cend; i++) {
    dx = (Cradius + Coffset_radius) * cos((i - 90) * PI / 180) + x + Coffset / 2; // calculate X position
    dy = (Cradius + Coffset_radius) * sin((i - 90) * PI / 180) + y; // calculate Y position
    gfx->setPixel(dx, dy);
  }
}
//#########################################################################################
void draw_manager::arrow(int x, int y, int asize, float aangle, int pwidth, int plength) {
  float dx = (asize - 10) * cos((aangle - 90) * PI / 180) + x; // calculate X position
  float dy = (asize - 10) * sin((aangle - 90) * PI / 180) + y; // calculate Y position
  float x1 = 0;         float y1 = plength;
  float x2 = pwidth / 2;  float y2 = pwidth / 2;
  float x3 = -pwidth / 2; float y3 = pwidth / 2;
  float angle = aangle * PI / 180 - 135;
  float xx1 = x1 * cos(angle) - y1 * sin(angle) + dx;
  float yy1 = y1 * cos(angle) + x1 * sin(angle) + dy;
  float xx2 = x2 * cos(angle) - y2 * sin(angle) + dx;
  float yy2 = y2 * cos(angle) + x2 * sin(angle) + dy;
  float xx3 = x3 * cos(angle) - y3 * sin(angle) + dx;
  float yy3 = y3 * cos(angle) + x3 * sin(angle) + dy;
  gfx->fillTriangle(xx1, yy1, xx3, yy3, xx2, yy2);
}
//#########################################################################################
void draw_manager::DisplayWXicon(int x, int y, String IconName, bool LargeSize) {
  gfx->setTextAlignment(TEXT_ALIGN_CENTER);
  Serial.println(IconName);
    if      (IconName == "01d" || IconName == "01n")  Sunny(x, y, LargeSize, IconName);
    else if (IconName == "02d" || IconName == "02n")  MostlySunny(x, y, LargeSize, IconName);
    else if (IconName == "03d" || IconName == "03n")  Cloudy(x, y, LargeSize, IconName);
    else if (IconName == "04d" || IconName == "04n")  MostlySunny(x, y, LargeSize, IconName);
    else if (IconName == "09d" || IconName == "09n")  ChanceRain(x, y, LargeSize, IconName);
    else if (IconName == "10d" || IconName == "10n")  Rain(x, y, LargeSize, IconName);
    else if (IconName == "11d" || IconName == "11n")  Tstorms(x, y, LargeSize, IconName); 
    else if (IconName == "13d" || IconName == "13n")  Snow(x, y, LargeSize, IconName);
    else if (IconName == "50d")                       Haze(x, y - 5, LargeSize, IconName);
    else if (IconName == "50n")                       Fog(x, y - 5, LargeSize, IconName);
    else                                              Nodata(x, y, LargeSize);
}
//#########################################################################################
// Symbols are drawn on a relative 10x10grid and 1 scale unit = 1 drawing unit
void draw_manager::addcloud(int x, int y, int scale, int linesize) {
  //Draw cloud outer
  gfx->fillCircle(x - scale * 3, y, scale);                       // Left most circle
  gfx->fillCircle(x + scale * 3, y, scale);                       // Right most circle
  gfx->fillCircle(x - scale, y - scale, scale * 1.4);             // left middle upper circle
  gfx->fillCircle(x + scale * 1.5, y - scale * 1.3, scale * 1.75); // Right middle upper circle
  gfx->fillRect(x - scale * 3 - 1, y - scale, scale * 6, scale * 2 + 1); // Upper and lower lines
  //Clear cloud inner
  gfx->setColor(EPD_WHITE);
  gfx->fillCircle(x - scale * 3, y, scale - linesize);            // Clear left most circle
  gfx->fillCircle(x + scale * 3, y, scale - linesize);            // Clear right most circle
  gfx->fillCircle(x - scale, y - scale, scale * 1.4 - linesize);  // left middle upper circle
  gfx->fillCircle(x + scale * 1.5, y - scale * 1.3, scale * 1.75 - linesize); // Right middle upper circle
  gfx->fillRect(x - scale * 3 + 2, y - scale + linesize - 1, scale * 5.9, scale * 2 - linesize * 2 + 2); // Upper and lower lines
  gfx->setColor(EPD_BLACK);
}
//#########################################################################################
void draw_manager::addrain(int x, int y, int scale) {
  y = y + scale / 2;
  for (int i = 0; i < 6; i++) {
    gfx->drawLine(x - scale * 4 + scale * i * 1.3 + 0, y + scale * 1.9, x - scale * 3.5 + scale * i * 1.3 + 0, y + scale);
    if (scale != SMALL) {
      gfx->drawLine(x - scale * 4 + scale * i * 1.3 + 1, y + scale * 1.9, x - scale * 3.5 + scale * i * 1.3 + 1, y + scale);
      gfx->drawLine(x - scale * 4 + scale * i * 1.3 + 2, y + scale * 1.9, x - scale * 3.5 + scale * i * 1.3 + 2, y + scale);
    }
  }
}
//#########################################################################################
void draw_manager::addsnow(int x, int y, int scale) {
  int dxo, dyo, dxi, dyi;
  for (int flakes = 0; flakes < 5; flakes++) {
    for (int i = 0; i < 360; i = i + 45) {
      dxo = 0.5 * scale * cos((i - 90) * 3.14 / 180); dxi = dxo * 0.1;
      dyo = 0.5 * scale * sin((i - 90) * 3.14 / 180); dyi = dyo * 0.1;
      gfx->drawLine(dxo + x + 0 + flakes * 1.5 * scale - scale * 3, dyo + y + scale * 2, dxi + x + 0 + flakes * 1.5 * scale - scale * 3, dyi + y + scale * 2);
    }
  }
}
//#########################################################################################
void draw_manager::addtstorm(int x, int y, int scale) {
  y = y + scale / 2;
  for (int i = 0; i < 5; i++) {
    gfx->drawLine(x - scale * 4 + scale * i * 1.5 + 0, y + scale * 1.5, x - scale * 3.5 + scale * i * 1.5 + 0, y + scale);
    if (scale != SMALL) {
      gfx->drawLine(x - scale * 4 + scale * i * 1.5 + 1, y + scale * 1.5, x - scale * 3.5 + scale * i * 1.5 + 1, y + scale);
      gfx->drawLine(x - scale * 4 + scale * i * 1.5 + 2, y + scale * 1.5, x - scale * 3.5 + scale * i * 1.5 + 2, y + scale);
    }
    gfx->drawLine(x - scale * 4 + scale * i * 1.5, y + scale * 1.5 + 0, x - scale * 3 + scale * i * 1.5 + 0, y + scale * 1.5 + 0);
    if (scale != SMALL) {
      gfx->drawLine(x - scale * 4 + scale * i * 1.5, y + scale * 1.5 + 1, x - scale * 3 + scale * i * 1.5 + 0, y + scale * 1.5 + 1);
      gfx->drawLine(x - scale * 4 + scale * i * 1.5, y + scale * 1.5 + 2, x - scale * 3 + scale * i * 1.5 + 0, y + scale * 1.5 + 2);
    }
    gfx->drawLine(x - scale * 3.5 + scale * i * 1.4 + 0, y + scale * 2.5, x - scale * 3 + scale * i * 1.5 + 0, y + scale * 1.5);
    if (scale != SMALL) {
      gfx->drawLine(x - scale * 3.5 + scale * i * 1.4 + 1, y + scale * 2.5, x - scale * 3 + scale * i * 1.5 + 1, y + scale * 1.5);
      gfx->drawLine(x - scale * 3.5 + scale * i * 1.4 + 2, y + scale * 2.5, x - scale * 3 + scale * i * 1.5 + 2, y + scale * 1.5);
    }
  }
}
//#########################################################################################
void draw_manager::addsun(int x, int y, int scale) {
  int linesize = 3;
  if (scale == SMALL) linesize = 1;
  int dxo, dyo, dxi, dyi;
  gfx->fillCircle(x, y, scale);
  gfx->setColor(EPD_WHITE);
  gfx->fillCircle(x, y, scale - linesize);
  gfx->setColor(EPD_BLACK);
  for (float i = 0; i < 360; i = i + 45) {
    dxo = 2.2 * scale * cos((i - 90) * 3.14 / 180); dxi = dxo * 0.6;
    dyo = 2.2 * scale * sin((i - 90) * 3.14 / 180); dyi = dyo * 0.6;
    if (i == 0   || i == 180) {
      gfx->drawLine(dxo + x - 1, dyo + y, dxi + x - 1, dyi + y);
      if (scale != SMALL) {
        gfx->drawLine(dxo + x + 0, dyo + y, dxi + x + 0, dyi + y);
        gfx->drawLine(dxo + x + 1, dyo + y, dxi + x + 1, dyi + y);
      }
    }
    if (i == 90  || i == 270) {
      gfx->drawLine(dxo + x, dyo + y - 1, dxi + x, dyi + y - 1);
      if (scale != SMALL) {
        gfx->drawLine(dxo + x, dyo + y + 0, dxi + x, dyi + y + 0);
        gfx->drawLine(dxo + x, dyo + y + 1, dxi + x, dyi + y + 1);
      }
    }
    if (i == 45  || i == 135 || i == 225 || i == 315) {
      gfx->drawLine(dxo + x - 1, dyo + y, dxi + x - 1, dyi + y);
      if (scale != SMALL) {
        gfx->drawLine(dxo + x + 0, dyo + y, dxi + x + 0, dyi + y);
        gfx->drawLine(dxo + x + 1, dyo + y, dxi + x + 1, dyi + y);
      }
    }
  }
}
//#########################################################################################
void draw_manager::addfog(int x, int y, int scale, int linesize) {
  if (scale == SMALL) y -= 10;
  if (scale == SMALL) linesize = 1;
  for (int i = 0; i < 6; i++) {
    gfx->fillRect(x - scale * 3, y + scale * 1.5, scale * 6, linesize);
    gfx->fillRect(x - scale * 3, y + scale * 2.0, scale * 6, linesize);
    gfx->fillRect(x - scale * 3, y + scale * 2.7, scale * 6, linesize);
  }
}
//#########################################################################################
void draw_manager::MostlyCloudy(int x, int y, bool LargeSize, String IconName) {
  int scale = SMALL;
  if (LargeSize) scale = LARGE;
  int linesize = 3;
  if (scale == SMALL) linesize = 1;
  if (IconName.endsWith("n")) addmoon(x,y,scale);
  addcloud(x, y, scale, linesize);
  addsun(x - scale * 1.8, y - scale * 1.8, scale);
  addcloud(x, y, scale, linesize);
}
//#########################################################################################
void draw_manager::MostlySunny(int x, int y, bool LargeSize, String IconName) {
  int scale = SMALL;
  if (LargeSize) scale = LARGE;
  int linesize = 3;
  if (scale == SMALL) linesize = 1;
  if (IconName.endsWith("n")) addmoon(x,y,scale);
  addcloud(x, y, scale, linesize);
  addsun(x - scale * 1.8, y - scale * 1.8, scale);
}
//#########################################################################################
void draw_manager::Rain(int x, int y, bool LargeSize, String IconName) {
  int scale = SMALL;
  if (LargeSize) scale = LARGE;
  int linesize = 3;
  if (scale == SMALL) linesize = 1;
  if (IconName.endsWith("n")) addmoon(x,y,scale);
  addcloud(x, y, scale, linesize);
  addrain(x, y, scale);
}
//#########################################################################################
void draw_manager::Cloudy(int x, int y, bool LargeSize, String IconName) {
  int scale = SMALL;
  if (LargeSize) scale = LARGE;
  int linesize = 3;
  if (scale == SMALL) {
    if (IconName.endsWith("n")) addmoon(x,y,scale);
    linesize = 1;
    addcloud(x, y, scale, linesize);
  }
  else {
    y += 25;
    if (IconName.endsWith("n")) addmoon(x,y-15,scale);
    addcloud(x+30, y-35, 4, linesize); // Cloud top right
    addcloud(x-20, y-25, 6, linesize); // Cloud top left
    addcloud(x, y, scale, linesize);   // Main cloud
  }
}
//#########################################################################################
void draw_manager::Sunny(int x, int y, bool LargeSize, String IconName) {
  int scale = SMALL;
  if (LargeSize) scale = LARGE;
  if (IconName.endsWith("n")) addmoon(x,y,scale);
  scale = scale * 1.5;
  addsun(x, y, scale);
}
//#########################################################################################
void draw_manager::ExpectRain(int x, int y, bool LargeSize, String IconName) {
  int scale = SMALL;
  if (LargeSize) scale = LARGE;
  int linesize = 3;
  if (scale == SMALL) linesize = 1;
  if (IconName.endsWith("n")) addmoon(x,y,scale);
  addsun(x - scale * 1.8, y - scale * 1.8, scale);
  addcloud(x, y, scale, linesize);
  addrain(x, y, scale);
}
//#########################################################################################
void draw_manager::ChanceRain(int x, int y, bool LargeSize, String IconName) {
  int scale = SMALL;
  if (LargeSize) scale = LARGE;
  int linesize = 3;
  if (scale == SMALL) linesize = 1;
  if (IconName.endsWith("n")) addmoon(x,y,scale);
  addsun(x - scale * 1.8, y - scale * 1.8, scale);
  addcloud(x, y, scale, linesize);
  addrain(x, y, scale);
}
//#########################################################################################
void draw_manager::Tstorms(int x, int y, bool LargeSize, String IconName) {
  int scale = SMALL;
  if (LargeSize) scale = LARGE;
  int linesize = 3;
  if (scale == SMALL) linesize = 1;
  if (IconName.endsWith("n")) addmoon(x,y,scale);
  addcloud(x, y, scale, linesize);
  addtstorm(x, y, scale);
}
//#########################################################################################
void draw_manager::Snow(int x, int y, bool LargeSize, String IconName) {
  int scale = SMALL;
  if (LargeSize) scale = LARGE;
  int linesize = 3;
  if (scale == SMALL) linesize = 1;
  if (IconName.endsWith("n")) addmoon(x,y,scale);
  addcloud(x, y, scale, linesize);
  addsnow(x, y, scale);
}
//#########################################################################################
void draw_manager::Fog(int x, int y, bool LargeSize, String IconName) {
  int scale = SMALL;
  if (LargeSize) scale = LARGE;
  int linesize = 3;
  if (scale == SMALL) linesize = 1;
  if (IconName.endsWith("n")) addmoon(x,y,scale);
  addcloud(x, y, scale, linesize);
  addfog(x, y, scale, linesize);
}
//#########################################################################################
void draw_manager::Haze(int x, int y, bool LargeSize, String IconName) {
  int scale = SMALL;
  if (LargeSize) scale = LARGE;
  int linesize = 3;
  if (scale == SMALL) linesize = 1;
  if (IconName.endsWith("n")) addmoon(x,y,scale);
  addsun(x, y, scale*1.4);
  addfog(x, y, scale*1.4, linesize);
}
//#########################################################################################
void draw_manager::addmoon (int x, int y, int scale){
  if (scale == LARGE) {
    gfx->fillCircle(x-37,y-33,scale);
    gfx->setColor(EPD_WHITE);
    gfx->fillCircle(x-27,y-33,scale*1.6);
    gfx->setColor(EPD_BLACK);
  }
  else
  {
    gfx->fillCircle(x-20,y-15,scale);
    gfx->setColor(EPD_WHITE);
    gfx->fillCircle(x-15,y-15,scale*1.6);
    gfx->setColor(EPD_BLACK);
  }
}
//#########################################################################################
void draw_manager::Nodata(int x, int y, bool LargeSize) {
  int scale = SMALL;
  if (LargeSize) scale = LARGE;
  if (scale == LARGE) gfx->setFont(ArialMT_Plain_24); else gfx->setFont(ArialMT_Plain_16);
  gfx->drawString(x, y-10, "N/A");
}
//#########################################################################################
void draw_manager::DrawBattery(int x, int y) {
  uint8_t percentage = 100;
  float voltage = analogRead(35) / 4096.0 * 9.22;
  if (voltage > 1) {
    if (voltage >= 4.20) percentage = 100;
    else if (voltage < 3.20) percentage = 0;
    else percentage = (voltage - 3.20) * 100 / (4.20 - 3.20);
    gfx->setColor(EPD_BLACK);
    gfx->setFont(ArialMT_Plain_10);
    gfx->setTextAlignment(TEXT_ALIGN_RIGHT);
    gfx->drawString(x - 25, y, String(voltage, 2) + "V");
    gfx->drawRect(x - 22, y + 2, 19, 10);
    gfx->fillRect(x - 2, y + 4, 3, 5);
    gfx->fillRect(x - 20, y + 4, 17 * percentage / 100.0, 6);
  }
}

//#########################################################################################
/* (C) D L BIRD
    This function will draw a graph on a ePaper/TFT/LCD display using data from an array containing data to be graphed.
    The variable 'MAX_READINGS' determines the maximum number of data elements for each array. Call it with the following parametric data:
    x_pos - the x axis top-left position of the graph
    y_pos - the y-axis top-left position of the graph, e.g. 100, 200 would draw the graph 100 pixels along and 200 pixels down from the top-left of the screen
    width - the width of the graph in pixels
    height - height of the graph in pixels
    Y1_Max - sets the scale of plotted data, for example 5000 would scale all data to a Y-axis of 5000 maximum
    data_array1 is parsed by value, externally they can be called anything else, e.g. within the routine it is called data_array1, but externally could be temperature_readings
    auto_scale - a logical value (TRUE or FALSE) that switches the Y-axis autoscale On or Off
    barchart_on - a logical value (TRUE or FALSE) that switches the drawing mode between barhcart and line graph
    barchart_colour - a sets the title and graph plotting colour
    If called with Y!_Max value of 500 and the data never goes above 500, then autoscale will retain a 0-500 Y scale, if on, the scale increases/decreases to match the data.
    auto_scale_margin, e.g. if set to 1000 then autoscale increments the scale by 1000 steps.
*/
void draw_manager::DrawGraph(int x_pos, int y_pos, int gwidth, int gheight,
                             float Y1Min, float Y1Max, String title, float *DataArray,
                             int readings, bool auto_scale, bool barchart_mode) {
#define auto_scale_margin 0 // Sets the autoscale increment, so axis steps up in units of e.g. 3
#define y_minor_axis 5      // 5 y-axis division markers
  int maxYscale = -10000;
  int minYscale =  10000;
  int last_x, last_y;
  float x1, y1, x2, y2;
  if (auto_scale == true) {
    for (int i = 1; i < readings; i++ ) {
      if (DataArray[i] >= maxYscale) maxYscale = DataArray[i];
      if (DataArray[i] <= minYscale) minYscale = DataArray[i];
    }
    maxYscale = round(maxYscale + auto_scale_margin); // Auto scale the graph and round to the nearest value defined, default was Y1Max
    Y1Max = round(maxYscale+0.5);
    if (minYscale != 0) minYscale = round(minYscale - auto_scale_margin); // Auto scale the graph and round to the nearest value defined, default was Y1Min
    Y1Min = round(minYscale);
  }
  // Draw the graph
  last_x = x_pos + 1;
  last_y = y_pos + (Y1Max - constrain(DataArray[1], Y1Min, Y1Max)) / (Y1Max - Y1Min) * gheight;
  gfx->setColor(EPD_BLACK);
  gfx->drawRect(x_pos, y_pos, gwidth + 3, gheight + 2);
  gfx->setFont(ArialMT_Plain_10);
  //gfx->setFont(ArialRoundedMTBold_14);
  gfx->setTextAlignment(TEXT_ALIGN_CENTER);
  gfx->drawString(x_pos + gwidth / 2, y_pos - 17, title);
  gfx->setFont(ArialMT_Plain_10);
  gfx->setTextAlignment(TEXT_ALIGN_RIGHT);
  // Draw the data
  for (int gx = 1; gx < readings; gx++) {
      x1 = last_x;
      y1 = last_y;
      x2 = x_pos + gx * gwidth/(readings-1)-1 ; // MAX_READINGS is the global variable that sets the maximum data that can be plotted
      y2 = y_pos + (Y1Max - constrain(DataArray[gx], Y1Min, Y1Max)) / (Y1Max - Y1Min) * gheight + 1;
      if (barchart_mode) {
        gfx->fillRect(x2, y2, (gwidth/readings)-1, y_pos + gheight - y2 + 1);
      } else {
        gfx->drawLine(last_x, last_y, x2, y2);
      }
      last_x = x2;
      last_y = y2;
  }
  //Draw the Y-axis scale
  for (int spacing = 0; spacing <= y_minor_axis; spacing++) {
  #define number_of_dashes 20
    for (int j = 0; j < number_of_dashes; j++) { // Draw dashed graph grid lines
      if (spacing < y_minor_axis) gfx->drawHorizontalLine((x_pos + 3 + j * gwidth / number_of_dashes), y_pos + (gheight * spacing / y_minor_axis), gwidth / (2 * number_of_dashes));
    }
    if ( (Y1Max-(float)(Y1Max-Y1Min)/y_minor_axis*spacing) < 10) {gfx->drawString(x_pos-2, y_pos+gheight*spacing/y_minor_axis-5, String((Y1Max-(float)(Y1Max-Y1Min)/y_minor_axis*spacing+0.01), 1));}
    else {
      if (Y1Min < 1) gfx->drawString(x_pos - 2, y_pos + gheight * spacing / y_minor_axis - 5, String((Y1Max - (float)(Y1Max - Y1Min) / y_minor_axis * spacing+0.01), 1));
      else gfx->drawString(x_pos - 2, y_pos + gheight * spacing / y_minor_axis - 5, String((Y1Max - (float)(Y1Max - Y1Min) / y_minor_axis * spacing + 0.01), 0)); // +0.01 prevents -0.00 occurring
    }
  }
  for (int i = 0; i <= 3; i++) {
    gfx->drawString(5 + x_pos + gwidth / 3 * i, y_pos + gheight + 3, String(i));
  }
  gfx->drawString(x_pos+gwidth/2+12,y_pos+gheight+5,"Days");
}
//#########################################################################################