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
#ifndef OPENWEATHERMAPCURRENT_H
#define OPENWEATHERMAPCURRENT_H

#include <JsonListener.h>
#include <JsonStreamingParser.h>

#include "Forecast_record_type.h"

class OpenWeatherMapCurrent: public JsonListener {
  private:
    String currentKey;
    String currentParent;
    Forecast_record_type *data;
    uint8_t weatherItemCounter = 0;
    boolean metric = true;
    String language;

    void doUpdate(Forecast_record_type *data, String url);
    String buildUrl(String appId, String locationParameter);

  public:
    OpenWeatherMapCurrent();
    void updateCurrent(Forecast_record_type *data, String appId, String location);
    void updateCurrentById(Forecast_record_type *data, String appId, String locationId);

    void setMetric(boolean metric) {this->metric = metric;}
    boolean isMetric() { return metric; }
    void setLanguage(String language) { this->language = language; }
    String getLanguage() { return language; }
    
    String getMeteoconIcon(String icon);
    virtual void whitespace(char c);

    virtual void startDocument();

    virtual void key(String key);

    virtual void value(String value);

    virtual void endArray();

    virtual void endObject();

    virtual void endDocument();

    virtual void startArray();

    virtual void startObject();
};
#endif // OPENWEATHERMAPCURRENT_H