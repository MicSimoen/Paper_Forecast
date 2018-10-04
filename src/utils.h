#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>

#include "Forecast_record_type.h"

namespace utils{
    int JulianDate(int d, int m, int y);
    String MoonPhase(int d, int m, int y, String hemisphere);
    String TitleCase(String text);
    String WindDegToDirection(float winddirection);
    void SetupTime(void);
    void UpdateLocalTime(String &Day_time_str, String &time_str);
    String ConvertUnixTime(int unix_time, String Units, int &MoonDay, int &MoonMonth, int &MoonYear);
    void Convert_Readings_to_Imperial(Forecast_record_type *WxConditions, Forecast_record_type *WxForecast);
}
#endif // UTILS_H