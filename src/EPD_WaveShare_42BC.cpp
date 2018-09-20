/**
The MIT License (MIT)
Copyright (c) 2017 by Daniel Eichhorn
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

Please note: I am spending a lot of my free time in developing Software and Hardware
for these projects. Please consider supporting me by
a) Buying my hardware kits from https://blog.squix.org/shop
b) Send a donation: https://www.paypal.me/squix/5USD
c) Or using this affiliate link while shopping: https://www.banggood.com/?p=6R31122484684201508S

See more at https://blog.squix.org

This code is based on a driver from http://waveshare.com
*/

#include "EPD_WaveShare_42BC.h"

EPD_WaveShare42BC::EPD_WaveShare42BC(uint8_t csPin, uint8_t rstPin, uint8_t dcPin, uint8_t busyPin) : DisplayDriver(400, 300)  {
  this->csPin = csPin;
  this->rstPin = rstPin;
  this->dcPin = dcPin;
  this->busyPin = busyPin;
  width = 400;
  height = 300;

}

int EPD_WaveShare42BC::getWidth() {
  return 400;
}

int EPD_WaveShare42BC::getHeight() {
  return 300;
}

void EPD_WaveShare42BC::setRotation(uint8_t r) {
  this->rotation = r;
  switch(r) {
    case 0:
      bufferWidth = width;
      bufferHeight = height;
      break;
    case 1:
      bufferWidth = height;
      bufferHeight = width;
      break;
    case 2:
      bufferWidth = width;
      bufferHeight = height;
      break;
    case 3:
      bufferWidth = height;
      bufferHeight = width;
      break;
  }
}

void EPD_WaveShare42BC::init() {
  /* this calls the peripheral hardware interface, see epdif */
  if (IfInit() != 0) {
      return;
  }
  /* EPD hardware init start */
  Reset();
  SendCommand(BOOSTER_SOFT_START);
  SendData(0x17);
  SendData(0x17);
  SendData(0x17);                  //07 0f 17 1f 27 2F 37 2f
  SendCommand(POWER_ON);
  WaitUntilIdle();
  SendCommand(PANEL_SETTING);
  SendData(0x0f);
  /* EPD hardware init end */
  return;

}

void EPD_WaveShare42BC::setFastRefresh(boolean isFastRefreshEnabled) {
  // Not enabled at the moment
}

void EPD_WaveShare42BC::DigitalWrite(int pin, int value) {
    digitalWrite(pin, value);
}

int EPD_WaveShare42BC::DigitalRead(int pin) {
    return digitalRead(pin);
}

void EPD_WaveShare42BC::DelayMs(unsigned int delaytime) {
    delay(delaytime);
}

void EPD_WaveShare42BC::SpiTransfer(unsigned char data) {
    digitalWrite(this->csPin, LOW);
    SPI.transfer(data);
    digitalWrite(this->csPin, HIGH);
}

int EPD_WaveShare42BC::IfInit(void) {
    Serial.println("Setting pin modes");
    digitalWrite(this->csPin, HIGH);
    pinMode(this->csPin, OUTPUT);
    digitalWrite(this->rstPin, HIGH);
    pinMode(this->rstPin, OUTPUT);
    digitalWrite(this->dcPin, HIGH);
    pinMode(this->dcPin, OUTPUT);
    digitalWrite(this->busyPin, HIGH);
    pinMode(this->busyPin, OUTPUT);
    Serial.println("Starting SPI transaction");
    SPI.begin();
    SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));
    Serial.println("Beginning SPI");

    return 0;
}



void EPD_WaveShare42BC::writeBuffer(uint8_t *buffer, uint8_t bitsPerPixel, uint16_t *palette, uint16_t x, uint16_t y, uint16_t bufferWidth, uint16_t bufferHeight) {
  DisplayFrame(buffer);
}

/**
 *  @brief: basic function for sending commands
 */
void EPD_WaveShare42BC::SendCommand(unsigned char command) {
    DigitalWrite(this->dcPin, LOW);
    SpiTransfer(command);
}

/**
 *  @brief: basic function for sending data
 */
void EPD_WaveShare42BC::SendData(unsigned char data) {
    DigitalWrite(this->dcPin, HIGH);
    SpiTransfer(data);
}

/**
 *  @brief: Wait until the busy_pin goes HIGH
 */
void EPD_WaveShare42BC::WaitUntilIdle(void) {
    while(DigitalRead(this->busyPin) == 0) {      //0: busy, 1: idle
        DelayMs(100);
    }
}

/**
 *  @brief: module reset.
 *          often used to awaken the module in deep sleep,
 *          see Epd::Sleep();
 */
void EPD_WaveShare42BC::Reset(void) {
    DigitalWrite(this->rstPin, LOW);
    DelayMs(200);
    DigitalWrite(this->rstPin, HIGH);
    DelayMs(200);
}

/**
 *  @brief: transmit partial data to the SRAM
 */
void EPD_WaveShare42BC::SetPartialWindow(const unsigned char* buffer_black, const unsigned char* buffer_red, int x, int y, int w, int l) {
    SendCommand(PARTIAL_IN);
    SendCommand(PARTIAL_WINDOW);
    SendData(x >> 8);
    SendData(x & 0xf8);     // x should be the multiple of 8, the last 3 bit will always be ignored
    SendData(((x & 0xf8) + w  - 1) >> 8);
    SendData(((x & 0xf8) + w  - 1) | 0x07);
    SendData(y >> 8);
    SendData(y & 0xff);
    SendData((y + l - 1) >> 8);
    SendData((y + l - 1) & 0xff);
    SendData(0x01);         // Gates scan both inside and outside of the partial window. (default)
    DelayMs(2);
    SendCommand(DATA_START_TRANSMISSION_1);
    if (buffer_black != NULL) {
        for(int i = 0; i < w  / 8 * l; i++) {
            SendData(buffer_black[i]);  
        }  
    } else {
        for(int i = 0; i < w  / 8 * l; i++) {
            SendData(0x00);
        }
    }
    DelayMs(2);
    SendCommand(DATA_START_TRANSMISSION_2);
    if (buffer_red != NULL) {
        for(int i = 0; i < w  / 8 * l; i++) {
            SendData(reverse(buffer_red[i]));
        }
    } else {
        for(int i = 0; i < w  / 8 * l; i++) {
            SendData(0x00);
        }
    }
    DelayMs(2);
    SendCommand(PARTIAL_OUT);
}

/**
 * @brief: refresh and displays the frame
 */
void EPD_WaveShare42BC::DisplayFrame(const unsigned char* frame_buffer) {
    uint16_t x = 0;
    uint16_t y = 0;
    int x_end;
    int y_end;
    uint16_t image_width = width;
    uint16_t image_height = height;
    uint16_t bufferSize = width * height / 8;
    uint16_t xDot = bufferWidth;
    uint16_t yDot = bufferHeight;
    uint8_t data;

    if (frame_buffer != NULL) {
        /*SendCommand(DATA_START_TRANSMISSION_1);
        for(int i = 0; i < width / 8 * height; i++) {
            SendData(0xFF);      // bit set: white, bit reset: black
        }
        DelayMs(2);*/
        /*SendCommand(DATA_START_TRANSMISSION_2);
        for(int i = 0; i < width / 8 * height; i++) {
            SendData(reverse(frame_buffer[i]));
        }
        DelayMs(2);*/
        SendCommand(DATA_START_TRANSMISSION_1);
        DelayMs(2);
        for(int i = 0; i < width / 8 * height; i++) {
            SendData(0xFF);      // bit set: white, bit reset: black
        }
        DelayMs(2);
        SendCommand(DATA_START_TRANSMISSION_2);
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < (width) / 8; j++) {

                data = 0;
                for (int b = 0; b < 8; b++) {
                  data = data << 1;
                  switch (rotation) {
                    case 0:
                      x = (j * 8 + b);
                      y = i;
                      break;
                    case 1:
                      x = bufferWidth - i;
                      y = (j * 8 + b);
                      break;
                    case 2:
                      x = xDot - (j * 8 + b);
                      y = yDot - i;
                      break;
                    case 3:
                      x = i;
                      y = bufferHeight - (j * 8 + b);
                      break;
                  }
                  data = data | (getPixel(frame_buffer, x, y) & 1);

                }
                SendData(data);
                //SendData(reverse(buffer[(i + j * (image_width / 8))]));
                //SendData(i);
                yield();
            }
        }
    }

    SetLut();

    SendCommand(DISPLAY_REFRESH);
    DelayMs(100);
    WaitUntilIdle();
}

uint8_t EPD_WaveShare42BC::getPixel(const unsigned char *buffer, uint16_t x, uint16_t y) {
  uint8_t bitsPerPixel = 1;
  uint8_t bitMask = (1 << bitsPerPixel) - 1;
  uint8_t pixelsPerByte = 8 / bitsPerPixel;
  uint8_t bitShift = 3;

  if (x >= bufferWidth || y >= bufferHeight) return 0;
  // bitsPerPixel: 8, pixPerByte: 1, 0  1 = 2^0
  // bitsPerPixel: 4, pixPerByte: 2, 1  2 = 2^1
  // bitsPerPixel  2, pixPerByte: 4, 2  4 = 2^2
  // bitsPerPixel  1, pixPerByte: 8, 3  8 = 2^3
  uint16_t pos = (y * bufferWidth + x) >> bitShift;

  uint8_t shift = (x & (pixelsPerByte - 1)) * bitsPerPixel;

  return (buffer[pos] >> shift) & bitMask;
}

uint8_t EPD_WaveShare42BC::reverse(uint8_t in)
{
  uint8_t out;
  out = 0;
  if (in & 0x01) out |= 0x80;
  if (in & 0x02) out |= 0x40;
  if (in & 0x04) out |= 0x20;
  if (in & 0x08) out |= 0x10;
  if (in & 0x10) out |= 0x08;
  if (in & 0x20) out |= 0x04;
  if (in & 0x40) out |= 0x02;
  if (in & 0x80) out |= 0x01;

  return(out);
}

/**
 * @brief: clear the frame data from the SRAM, this won't refresh the display
 */
void EPD_WaveShare42BC::ClearFrame(void) {
    SendCommand(DATA_START_TRANSMISSION_1);
    DelayMs(2);
    for(int i = 0; i < width / 8 * height; i++) {
        SendData(0xFF);
    }
    DelayMs(2);
    SendCommand(DATA_START_TRANSMISSION_2);
    DelayMs(2);
    for(int i = 0; i < width / 8 * height; i++) {
        SendData(0xFF);
    }
    DelayMs(2);
}

/**
 * @brief: This displays the frame data from SRAM
 */
void EPD_WaveShare42BC::DisplayFrame(void) {
    SendCommand(DISPLAY_REFRESH);
    DelayMs(100);
    WaitUntilIdle();
}

/**
 * @brief: After this command is transmitted, the chip would enter the deep-sleep mode to save power.
 *         The deep sleep mode would return to standby by hardware reset. The only one parameter is a
 *         check code, the command would be executed if check code = 0xA5.
 *         You can use Epd::Reset() to awaken and use Epd::Init() to initialize.
 */
void EPD_WaveShare42BC::Sleep() {
    SendCommand(VCOM_AND_DATA_INTERVAL_SETTING);
    SendData(0xF7);     // border floating
    SendCommand(POWER_OFF);
    WaitUntilIdle();
    SendCommand(DEEP_SLEEP);
    SendData(0xA5);     // check code
}
