#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>

namespace utils{
    int JulianDate(int d, int m, int y);
    String MoonPhase(int d, int m, int y, String hemisphere);
    String TitleCase(String text);
    String WindDegToDirection(float winddirection);
    String ConvertUnixTime(int unix_time, String Units, int &MoonDay, int &MoonMonth, int &MoonYear);
}
#endif // UTILS_H