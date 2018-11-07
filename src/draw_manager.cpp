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
    Serial.println("Committing buffer to screen");
    gfx->commit();
}

void draw_manager::DrawTest(void){
    gfx->setColor(EPD_BLACK);
    gfx->fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    gfx->setColor(EPD_WHITE);
    gfx->fillRect(10, 10, SCREEN_WIDTH-20, SCREEN_HEIGHT-20);
    gfx->setColor(EPD_BLACK);
    gfx->fillRect(30, 30, SCREEN_WIDTH-60, SCREEN_HEIGHT-60);
    gfx->setColor(EPD_YELLOW);
    gfx->fillCircle(SCREEN_WIDTH/2, SCREEN_HEIGHT/2, 100);
}

void draw_manager::Draw_Heading_Section(String Day_time_str, String time_str) {
    gfx->setColor(EPD_WHITE);
    gfx->fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    gfx->setColor(EPD_BLACK);
    gfx->setFont(ArialRoundedMTBold_14);
    gfx->setTextAlignment(TEXT_ALIGN_CENTER);
    gfx->drawString(screen_width / 2, -3, CITY);
    gfx->setFont(ArialMT_Plain_10);
    gfx->setTextAlignment(TEXT_ALIGN_RIGHT);
    gfx->drawString(screen_width-3, 0, time_str);
    gfx->setTextAlignment(TEXT_ALIGN_LEFT);
    gfx->drawString(5, 0, Day_time_str);
    gfx->drawLine(0, 15, screen_width, 15);
}
void draw_manager::Draw_Weather(Forecast_record_type *WxConditions, Forecast_record_type *WxForecast) {
    gfx->fillRect(0, 15, SCREEN_WIDTH, SCREEN_HEIGHT);
    Draw_Condition_Section(0, 15, WxConditions, WxForecast);
    Draw_Wind_Section(130, 18, WxConditions[0].Winddir, WxConditions[0].Windspeed);
    Draw_Rain_Section(261, 18, WxConditions[0].Rainfall);
    Draw_Pressure_Section(330, 18, WxConditions[0].Pressure, WxForecast[0].Trend);
    Draw_Description_Section(261, 90, WxConditions);
    Draw_NextDay_Section(130, 160, WxForecast);
}
//#########################################################################################
void draw_manager::Draw_Condition_Section(int x, int y, Forecast_record_type *WxConditions, Forecast_record_type *WxForecast) {
    gfx->fillRect(x, y, 130, 285);
    DisplayWXicon(x + 65, y + 90, WxConditions[0].Icon, MAIN_WEATHER_LARGE_ICON, true);

    int max_index = 8;
    // if(sizeof(WxForecast) < 8) {
    //     max_index = sizeof(WxForecast);
    // }

    float min_temp = WxConditions[0].Temperature;
    float max_temp = WxConditions[0].Temperature;
    for(int i = 0; i <= max_index; i++) {
        if(WxForecast[i].Temperature<min_temp) {
            min_temp = WxForecast[i].Temperature;
        }
        if(WxForecast[i].Temperature>max_temp) {
            max_temp = WxForecast[i].Temperature;
        }
    }

    gfx->setColor(EPD_WHITE);
    gfx->setTextAlignment(TEXT_ALIGN_CENTER);
    gfx->setFont(ArialRoundedMTBold_36);
    gfx->drawString(x + 65, y + 145, String(WxConditions[0].Temperature,1) + "°" + (UNITS=="M"?"C":"F")); // Show current Temperature
    gfx->setFont(ArialMT_Plain_24);
    gfx->drawString(x + 65, y + 190, String(min_temp,0) + "° | " + String(max_temp,0) + "°"); // Show forecast high and Low
    gfx->setFont(ArialRoundedMTBold_14);
    float sun_radius = 5;
    float x_start, x_end, y_start, y_end;
    for(int i=0; i < 2; i++){
        gfx->setColor(EPD_YELLOW);
        gfx->fillCircle(x + 40 + (i * 50), y + 255, sun_radius);
        for(float angle=0; angle<360; angle+=30) {
            x_start = (sun_radius + 3) * cos((angle - 90) * PI / 180) + x + 40 + (i * 50); // calculate X position
            y_start = (sun_radius + 3) * sin((angle - 90) * PI / 180) + y + 255; // calculate Y position
            x_end = (sun_radius + 8) * cos((angle - 90) * PI / 180) + x + 40 + (i * 50); // calculate X position
            y_end = (sun_radius + 8) * sin((angle - 90) * PI / 180) + y + 255; // calculate Y position
            gfx->drawLine(x_start, y_start, x_end, y_end);
        }
        gfx->setColor(EPD_BLACK);
        gfx->fillRect(x + 25 + (i * 50), y + 257, 30, 15);
        gfx->setColor(EPD_WHITE);
        gfx->drawLine(x + 20 + (i * 50), y + 258, x + 60 + (i * 50), y + 258);
    }
   
    int MoonDay, MoonMonth, MoonYear;
    arrow(x + 40, y + 236, 0, 0, 5, 5);
    gfx->drawString(x + 40, y + 260, utils::ConvertUnixTime(WxConditions[0].Sunrise,
                                                            UNITS,
                                                            MoonDay,
                                                            MoonMonth,
                                                            MoonYear).substring(0, 5));
    arrow(x + 90, y + 230, 0, 0, 6, -6);
    gfx->drawString(x + 90, y + 260, utils::ConvertUnixTime(WxConditions[0].Sunset,
                                                            UNITS,
                                                            MoonDay,
                                                            MoonMonth,
                                                            MoonYear).substring(0, 5));
    gfx->setTextAlignment(TEXT_ALIGN_LEFT);
    gfx->setColor(EPD_BLACK);
    
}
//#########################################################################################
void draw_manager::Draw_Wind_Section(int x, int y, float angle, float windspeed) {
    int Cradius = 44;

    gfx->setColor(EPD_WHITE);
    gfx->fillRect(x, y, 129, 140);
    gfx->setColor(EPD_BLACK);

    gfx->setTextAlignment(TEXT_ALIGN_CENTER);
    gfx->setFont(ArialRoundedMTBold_14);
    gfx->drawString(x + 65, y + 1, "Wind");
    gfx->drawLine(x , y + 20, x + 128, y + 20);
    
    gfx->drawCircle(x + 65, y + 80, Cradius -1);
    gfx->drawCircle(x + 65, y + 80, Cradius);
    float i;
    float x_pos, y_pos, x_start, y_start, x_end, y_end;
    gfx->setTextAlignment(TEXT_ALIGN_CENTER);
    gfx->setFont(ArialMT_Plain_10);
    for(i=0; i<360; i+=45) {
        x_pos = (Cradius + 9) * cos((i - 90) * PI / 180) + x + 65; // calculate X position
        y_pos = (Cradius + 9) * sin((i - 90) * PI / 180) + y + 74; // calculate Y position
        gfx->drawString(x_pos, y_pos, utils::WindDegToDirection(i));
        x_start = (Cradius - 7) * cos((i - 90) * PI / 180) + x + 65; // calculate X position
        y_start = (Cradius - 7) * sin((i - 90) * PI / 180) + y + 80; // calculate Y position
        x_end = (Cradius - 12) * cos((i - 90) * PI / 180) + x +65; // calculate X position
        y_end = (Cradius - 12) * sin((i - 90) * PI / 180) + y + 80; // calculate Y position
        gfx->drawLine(x_start, y_start, x_end, y_end);
    }

    arrow(x + 65, y + 80, Cradius - 12, angle, 5, 10); // Show wind direction on outer circle
    gfx->setTextAlignment(TEXT_ALIGN_CENTER);
    gfx->setFont(ArialRoundedMTBold_14);
    gfx->drawString(x + 65, y + 56, String(round(angle))+"°");
    gfx->drawString(x + 65, y  + 71, String(windspeed,1) + (UNITS == "M" ? " m/s" : " mph"));
    gfx->drawString(x + 65, y + 86, utils::WindDegToDirection(angle));
    
}
//#########################################################################################
void draw_manager::Draw_Rain_Section(int x, int y, float rain){
    gfx->setColor(EPD_WHITE);
    gfx->fillRect(x, y, 67, 70);
    gfx->setColor(EPD_BLACK);

    gfx->setTextAlignment(TEXT_ALIGN_CENTER);
    gfx->setFont(ArialRoundedMTBold_14);
    gfx->drawString(x + 33, y + 1, "Rain");
    gfx->drawLine(x, y + 20, x + 66, y + 20);

    gfx->setTextAlignment(TEXT_ALIGN_CENTER);
    if (rain > 0) {
        gfx->drawString(x + 33, y + 35, String(rain, (UNITS == "M" ? 1 : 3)) + (UNITS == "M" ? "mm" : "in"));
    } else {
        gfx->drawString(x + 33, y + 35, "--");
    }
    gfx->setFont(ArialMT_Plain_10);

}
//#########################################################################################
void draw_manager::Draw_Pressure_Section(int x, int y, float pressure, String trend){
    gfx->setColor(EPD_WHITE);
    gfx->fillRect(x, y, 67, 70);
    gfx->setColor(EPD_BLACK);

    gfx->setTextAlignment(TEXT_ALIGN_CENTER);
    gfx->setFont(ArialRoundedMTBold_14);
    gfx->drawString(x + 33, y + 1, "Pressure");
    gfx->drawLine(x, y + 20, x + 66, y + 20);

    gfx->setTextAlignment(TEXT_ALIGN_CENTER);
    gfx->drawString(x + 33, y + 25, String(pressure,1) + (UNITS == "M" ? "mb" : "in"));

    if (trend == "+") {
        arrow(x + 33, y + 60, 0, 0.0, 10, 10);
    } else if (trend == "0") {
        arrow(x + 28, y + 55, 0, 90.0, 10, 10);
   } else if (trend == "-") {
        arrow(x + 33, y + 50, 0, 180.0, 10, 10);
   }

    gfx->setFont(ArialMT_Plain_10);

}
//#########################################################################################
void draw_manager::Draw_Description_Section(int x, int y, Forecast_record_type *WxConditions){
    gfx->setColor(EPD_WHITE);
    gfx->fillRect(x, y, 136, 68);
    gfx->setColor(EPD_BLACK);

    String Wx_Description;
    if (WxConditions[0].Forecast0 != "") {
        gfx->setFont(ArialMT_Plain_24);
        gfx->drawString(x + 67, y + 10, WxConditions[0].Main0);
        Wx_Description = WxConditions[0].Forecast0;
        if (WxConditions[0].Forecast1 != "" && WxConditions[0].Forecast1 != WxConditions[0].Forecast2) {
            Wx_Description += " & " +  WxConditions[0].Forecast1;
        }
        gfx->setFont(ArialRoundedMTBold_14);
        gfx->drawString(x + 67, y + 40, Wx_Description);
    } else {
        gfx->setFont(ArialMT_Plain_24);
        gfx->drawString(x + 67, y + 30, WxConditions[0].Main0);
    }
    gfx->setFont(ArialMT_Plain_10);

}
void draw_manager::Draw_NextDay_Section(int x, int y, Forecast_record_type *WxForeCast) {
    gfx->setColor(EPD_WHITE);
    gfx->fillRect(x, y, 267, 137);
    gfx->setColor(EPD_BLACK);

    gfx->drawLine(x + 8, y + 68, x + 258, y + 68);
    gfx->drawLine(x + 67, y + 6, x + 67, y + 62);
    gfx->drawLine(x + 133, y + 6, x + 133, y + 62);
    gfx->drawLine(x + 199, y + 6, x + 199, y + 62);
    gfx->drawLine(x + 67, y + 75, x + 67, y + 130);
    gfx->drawLine(x + 133, y + 75, x + 133, y + 130);
    gfx->drawLine(x + 199, y + 75, x + 199, y + 130);

    int x_index, y_index, index, x_pos, y_pos;
    for (y_index = 0; y_index < 2; y_index++){
        for(x_index = 0; x_index < 4; x_index++){
            x_pos = x + 6 + x_index * 66;
            y_pos = y + 2 + y_index * 70;
            index = x_index + 4 * y_index;
            Draw_Forecast_Weather(x_pos, y_pos, index, false, WxForeCast);
        }
    }
}
//#########################################################################################
void draw_manager::Draw_Forecast_Weather(int x,
                                         int y,
                                         int index,
                                         bool draw_frame,
                                         Forecast_record_type *WxForecast) {
  gfx->setFont(ArialMT_Plain_10);
  gfx->setColor(EPD_BLACK); // Sometimes gets set to WHITE, so change back
  if (draw_frame) {
      gfx->drawRect(x, y, 55, 65);
  }
  gfx->drawLine(x + 1, y + 13, x + 55, y + 13);
  DisplayWXicon(x + 28, y + 35, WxForecast[index].Icon, FORECAST_LARGE_ICON, false);
  gfx->setTextAlignment(TEXT_ALIGN_CENTER);
  gfx->setFont(ArialMT_Plain_10);
  gfx->drawString(x + 28, y, String(WxForecast[index].Period.substring(11,16)));
  //gfx->drawString(x + 28, y + 50, String(WxForecast[index].High,0) + "° / " + String(WxForecast[index].Low,0) + "°");
  String temperature = String(round(WxForecast[index].Temperature), 0);
  String rain = String(round(10 * WxForecast[index].Rainfall)/10, 1);
  gfx->drawString(x + 28, y + 50, temperature + "° / " + rain + (UNITS == "M" ? "mm" : "in"));
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
    float dx = asize * cos((aangle - 90) * PI / 180) + x; // calculate X position
    float dy = asize * sin((aangle - 90) * PI / 180) + y; // calculate Y position
    float xx1 = plength * cos((aangle - 90) * PI / 180) + dx;
    float yy1 = plength * sin((aangle - 90) * PI / 180) + dy;
    float xx2 = pwidth * cos(aangle * PI / 180) + dx;
    float yy2 = pwidth * sin(aangle * PI / 180) + dy;
    float xx3 = pwidth * cos((aangle - 180) * PI / 180) + dx;
    float yy3 = pwidth * sin((aangle - 180) * PI / 180) + dy;
    gfx->fillTriangle(xx1, yy1, xx3, yy3, xx2, yy2);
}
//#########################################################################################
void draw_manager::DisplayWXicon(int x, int y, String IconName, bool LargeSize, bool invert_color) {
  gfx->setTextAlignment(TEXT_ALIGN_CENTER);
  // Serial.println(IconName);
    if      (IconName == "01d" || IconName == "01n")  Sunny(x, y, LargeSize, invert_color, IconName);
    else if (IconName == "02d" || IconName == "02n")  MostlySunny(x, y, LargeSize, invert_color, IconName);
    else if (IconName == "03d" || IconName == "03n")  Cloudy(x, y, LargeSize, invert_color, IconName);
    else if (IconName == "04d" || IconName == "04n")  MostlySunny(x, y, LargeSize, invert_color, IconName);
    else if (IconName == "09d" || IconName == "09n")  ChanceRain(x, y, LargeSize, invert_color, IconName);
    else if (IconName == "10d" || IconName == "10n")  Rain(x, y, LargeSize, invert_color, IconName);
    else if (IconName == "11d" || IconName == "11n")  Tstorms(x, y, LargeSize, invert_color, IconName); 
    else if (IconName == "13d" || IconName == "13n")  Snow(x, y, LargeSize, invert_color, IconName);
    else if (IconName == "50d")                       Haze(x, y - 5, LargeSize, invert_color, IconName);
    else if (IconName == "50n")                       Fog(x, y - 5, LargeSize, invert_color, IconName);
    else                                              Nodata(x, y, LargeSize, invert_color);
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
void draw_manager::addrain(int x, int y, int scale, bool invert_color) {
    if (invert_color) {
        gfx->setColor(EPD_WHITE);
    }
    y = y + scale / 2;
    for (int i = 0; i < 6; i++) {
        gfx->drawLine(x - scale * 4 + scale * i * 1.3 + 0, y + scale * 1.9, x - scale * 3.5 + scale * i * 1.3 + 0, y + scale);
        if (scale != SMALL) {
        gfx->drawLine(x - scale * 4 + scale * i * 1.3 + 1, y + scale * 1.9, x - scale * 3.5 + scale * i * 1.3 + 1, y + scale);
        gfx->drawLine(x - scale * 4 + scale * i * 1.3 + 2, y + scale * 1.9, x - scale * 3.5 + scale * i * 1.3 + 2, y + scale);
        }
    }
    if (invert_color) {
        gfx->setColor(EPD_BLACK);
    }
}
//#########################################################################################
void draw_manager::addsnow(int x, int y, int scale, bool invert_color) {
    if (invert_color) {
        gfx->setColor(EPD_WHITE);
    }
    int dxo, dyo, dxi, dyi;
    for (int flakes = 0; flakes < 5; flakes++) {
        for (int i = 0; i < 360; i = i + 45) {
            dxo = 0.5 * scale * cos((i - 90) * 3.14 / 180); dxi = dxo * 0.1;
            dyo = 0.5 * scale * sin((i - 90) * 3.14 / 180); dyi = dyo * 0.1;
            gfx->drawLine(dxo + x + 0 + flakes * 1.5 * scale - scale * 3, dyo + y + scale * 2, dxi + x + 0 + flakes * 1.5 * scale - scale * 3, dyi + y + scale * 2);
        }
    }
    if (invert_color) {
        gfx->setColor(EPD_BLACK);
    }
}
//#########################################################################################
void draw_manager::addtstorm(int x, int y, int scale) {
    gfx->setColor(EPD_YELLOW);
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
    gfx->setColor(EPD_BLACK);
}
//#########################################################################################
void draw_manager::addsun(int x, int y, int scale) {
    gfx->setColor(EPD_YELLOW);
    int linesize = 3;
    if (scale == SMALL) linesize = 1;
    int dxo, dyo, dxi, dyi;
    gfx->fillCircle(x, y, scale);
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
    gfx->setColor(EPD_BLACK);
}
//#########################################################################################
void draw_manager::addfog(int x, int y, int scale, int linesize, bool invert_color) {
    if (invert_color) {
        gfx->setColor(EPD_WHITE);
    }
    if (scale == SMALL) y -= 10;
    if (scale == SMALL) linesize = 1;
    for (int i = 0; i < 6; i++) {
        gfx->fillRect(x - scale * 3, y + scale * 1.5, scale * 6, linesize);
        gfx->fillRect(x - scale * 3, y + scale * 2.0, scale * 6, linesize);
        gfx->fillRect(x - scale * 3, y + scale * 2.7, scale * 6, linesize);
    }
    if (invert_color) {
        gfx->setColor(EPD_BLACK);
    }
}
//#########################################################################################
void draw_manager::MostlyCloudy(int x, int y, bool LargeSize, bool invert_color, String IconName) {
  int scale = SMALL;
  if (LargeSize) scale = LARGE;
  int linesize = 3;
  if (scale == SMALL) linesize = 1;
  if (IconName.endsWith("n")) addmoon(x,y,scale, invert_color);
  addcloud(x, y, scale, linesize);
  addsun(x - scale * 1.8, y - scale * 1.8, scale);
  addcloud(x, y, scale, linesize);
}
//#########################################################################################
void draw_manager::MostlySunny(int x, int y, bool LargeSize, bool invert_color, String IconName) {
  int scale = SMALL;
  if (LargeSize) scale = LARGE;
  int linesize = 3;
  if (scale == SMALL) linesize = 1;
  if (IconName.endsWith("n")) addmoon(x,y,scale, invert_color);
  addcloud(x, y, scale, linesize);
  addsun(x - scale * 1.8, y - scale * 1.8, scale);
}
//#########################################################################################
void draw_manager::Rain(int x, int y, bool LargeSize, bool invert_color, String IconName) {
  int scale = SMALL;
  if (LargeSize) scale = LARGE;
  int linesize = 3;
  if (scale == SMALL) linesize = 1;
  if (IconName.endsWith("n")) addmoon(x,y,scale, invert_color);
  addcloud(x, y, scale, linesize);
  addrain(x, y, scale, invert_color);
}
//#########################################################################################
void draw_manager::Cloudy(int x, int y, bool LargeSize, bool invert_color, String IconName) {
  int scale = SMALL;
  if (LargeSize) scale = LARGE;
  int linesize = 3;
  if (scale == SMALL) {
    if (IconName.endsWith("n")) addmoon(x,y,scale, invert_color);
    linesize = 1;
    addcloud(x, y, scale, linesize);
  }
  else {
    y += 25;
    if (IconName.endsWith("n")) addmoon(x,y-15,scale, invert_color);
    addcloud(x+30, y-35, 6, linesize); // Cloud top right
    addcloud(x-20, y-25, 8, linesize); // Cloud top left
    addcloud(x, y, scale, linesize);   // Main cloud
  }
}
//#########################################################################################
void draw_manager::Sunny(int x, int y, bool LargeSize, bool invert_color, String IconName) {
    int scale = SMALL;
    if (LargeSize) scale = LARGE;
    if (IconName.endsWith("n")) addmoon(x,y,scale, invert_color);
    scale = scale * 1.5;
    addsun(x, y, scale);
}
//#########################################################################################
void draw_manager::ExpectRain(int x, int y, bool LargeSize, bool invert_color, String IconName) {
  int scale = SMALL;
  if (LargeSize) scale = LARGE;
  int linesize = 3;
  if (scale == SMALL) linesize = 1;
  if (IconName.endsWith("n")) addmoon(x,y,scale, invert_color);
  addsun(x - scale * 1.8, y - scale * 1.8, scale);
  addcloud(x, y, scale, linesize);
  addrain(x, y, scale, invert_color);
}
//#########################################################################################
void draw_manager::ChanceRain(int x, int y, bool LargeSize, bool invert_color, String IconName) {
  int scale = SMALL;
  if (LargeSize) scale = LARGE;
  int linesize = 3;
  if (scale == SMALL) linesize = 1;
  if (IconName.endsWith("n")) addmoon(x,y,scale, invert_color);
  addsun(x - scale * 1.8, y - scale * 1.8, scale);
  addcloud(x, y, scale, linesize);
  addrain(x, y, scale, invert_color);
}
//#########################################################################################
void draw_manager::Tstorms(int x, int y, bool LargeSize, bool invert_color, String IconName) {
  int scale = SMALL;
  if (LargeSize) scale = LARGE;
  int linesize = 3;
  if (scale == SMALL) linesize = 1;
  if (IconName.endsWith("n")) addmoon(x,y,scale, invert_color);
  addcloud(x, y, scale, linesize);
  addtstorm(x, y, scale);
}
//#########################################################################################
void draw_manager::Snow(int x, int y, bool LargeSize, bool invert_color, String IconName) {
  int scale = SMALL;
  if (LargeSize) scale = LARGE;
  int linesize = 3;
  if (scale == SMALL) linesize = 1;
  if (IconName.endsWith("n")) addmoon(x,y,scale, invert_color);
  addcloud(x, y, scale, linesize);
  addsnow(x, y, scale, invert_color);
}
//#########################################################################################
void draw_manager::Fog(int x, int y, bool LargeSize, bool invert_color, String IconName) {
  int scale = SMALL;
  if (LargeSize) scale = LARGE;
  int linesize = 3;
  if (scale == SMALL) linesize = 1;
  if (IconName.endsWith("n")) addmoon(x,y,scale, invert_color);
  addcloud(x, y, scale, linesize);
  addfog(x, y, scale, linesize, invert_color);
}
//#########################################################################################
void draw_manager::Haze(int x, int y, bool LargeSize, bool invert_color, String IconName) {
  int scale = SMALL;
  if (LargeSize) scale = LARGE;
  int linesize = 3;
  if (scale == SMALL) linesize = 1;
  if (IconName.endsWith("n")) addmoon(x,y,scale, invert_color);
  addsun(x, y, scale*1.4);
  addfog(x, y, scale*1.4, linesize, invert_color);
}
//#########################################################################################
void draw_manager::addmoon (int x, int y, int scale, bool invert_color){
    if (invert_color) {
        gfx->setColor(EPD_WHITE);
        if (scale == LARGE) {
            gfx->fillCircle(x-40,y-70,scale*0.7);
            gfx->setColor(EPD_BLACK);
            gfx->fillCircle(x-30,y-70,scale*1.12);
        } else {
            gfx->fillCircle(x-20,y-15,scale);
            gfx->setColor(EPD_BLACK);
            gfx->fillCircle(x-15,y-15,scale*1.6);
        }
    } else {
        if (scale == LARGE) {
            gfx->fillCircle(x-40,y-70,scale*0.7);
            gfx->setColor(EPD_WHITE);
            gfx->fillCircle(x-30,y-70,scale*1.12);
        } else {
            gfx->fillCircle(x-20,y-15,scale);
            gfx->setColor(EPD_WHITE);
            gfx->fillCircle(x-15,y-15,scale*1.6);
        }
    }
    gfx->setColor(EPD_BLACK);
  
}
//#########################################################################################
void draw_manager::Nodata(int x, int y, bool LargeSize, bool invert_color) {
    if (invert_color) {
        gfx->setColor(EPD_WHITE);
    }
    int scale = SMALL;
    if (LargeSize) scale = LARGE;
    if (scale == LARGE) gfx->setFont(ArialMT_Plain_24); else gfx->setFont(ArialMT_Plain_16);
    gfx->drawString(x, y-10, "N/A");
    if (invert_color) {
        gfx->setColor(EPD_BLACK);
    }
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