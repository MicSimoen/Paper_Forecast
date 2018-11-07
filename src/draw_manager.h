#ifndef DRAW_MANAGER_H
#define DRAW_MANAGER_H

#include <Arduino.h>

#include "Forecast_record_type.h"
#include "EPD_WaveShare_42BC.h"  // Copyright (c) 2017 by Daniel Eichhorn https://github.com/ThingPulse/minigrafx
#include "MiniGrafx.h"         // Copyright (c) 2017 by Daniel Eichhorn https://github.com/ThingPulse/minigrafx
#include "DisplayDriver.h"     // Copyright (c) 2017 by Daniel Eichhorn https://github.com/ThingPulse/minigrafx
#include "ArialRounded.h"      // Copyright (c) 2017 by Daniel Eichhorn https://github.com/ThingPulse/minigrafx

class draw_manager{
    private:
        float screen_width;    // Set for landscape mode, don't remove the decimal place!
        float screen_height;
        uint8_t bits_per_pixel;
        uint16_t *palette;

        EPD_WaveShare42BC *EPaperDisplay;
        MiniGrafx *gfx;
    
    public:
        draw_manager(float screen_width_in,
                     float screen_height_in,
                     uint8_t bits_per_pixel_in,
                     uint16_t *palette_in);
        
        void Commit(void);

        void DrawTest(void);

        void Draw_Heading_Section(String Day_time_str, String time_str);
        void Draw_Forecast_Weather(int x,
                                   int y,
                                   int index,
                                   bool draw_frame,
                                   Forecast_record_type *WxForecast);
        void Draw_Weather(Forecast_record_type *WxConditions, Forecast_record_type *WxForecast);
        void Draw_Condition_Section(int x, int y, Forecast_record_type *WxConditions, Forecast_record_type *WxForecast);
        void Draw_Wind_Section(int x, int y, float angle, float windspeed);
        void Draw_Rain_Section(int x, int y, float rain);
        void Draw_Pressure_Section(int x, int y, float pressure, String trend);
        void Draw_Description_Section(int x, int y, Forecast_record_type *WxConditions);
        void Draw_NextDay_Section(int x, int y, Forecast_record_type *WxForeCast);
        void Draw_Astronomy_Section(int x, int y, Forecast_record_type *WxConditions);
        void DrawMoon(int x, int y, int dd, int mm, int yy, String hemisphere);
        void DrawCircle(int x,
                        int y,
                        int Cstart,
                        int Cend,
                        int Cradius,
                        int Coffset_radius,
                        int Coffset);
        void arrow(int x, int y, int asize, float aangle, int pwidth, int plength);
        void DisplayWXicon(int x, int y, String IconName, bool LargeSize, bool invert_color);
        void addcloud(int x, int y, int scale, int linesize);
        void addrain(int x, int y, int scale, bool invert_color);
        void addsnow(int x, int y, int scale, bool invert_color);
        void addtstorm(int x, int y, int scale);
        void addsun(int x, int y, int scale);
        void addfog(int x, int y, int scale, int linesize, bool invert_color);
        void MostlyCloudy(int x, int y, bool LargeSize, bool invert_color, String IconName);
        void MostlySunny(int x, int y, bool LargeSize, bool invert_color, String IconName);
        void Rain(int x, int y, bool LargeSize, bool invert_color, String IconName);
        void Cloudy(int x, int y, bool LargeSize, bool invert_color, String IconName);
        void Sunny(int x, int y, bool LargeSize, bool invert_color, String IconName);
        void ExpectRain(int x, int y, bool LargeSize, bool invert_color, String IconName);
        void ChanceRain(int x, int y, bool LargeSize, bool invert_color, String IconName);
        void Tstorms(int x, int y, bool LargeSize, bool invert_color, String IconName);
        void Snow(int x, int y, bool LargeSize, bool invert_color, String IconName);
        void Fog(int x, int y, bool LargeSize, bool invert_color, String IconName);
        void Haze(int x, int y, bool LargeSize, bool invert_color, String IconName);
        void addmoon (int x, int y, int scale, bool invert_color);
        void Nodata(int x, int y, bool LargeSize, bool invert_color);
        void DrawBattery(int x, int y);
        void DrawGraph(int x_pos,
                       int y_pos,
                        int gwidth,
                        int gheight,
                        float Y1Min,
                        float Y1Max,
                        String title,
                        float *DataArray,
                        int readings,
                        bool auto_scale,
                        bool barchart_mode);
};
#endif // DRAW_MANAGER_H