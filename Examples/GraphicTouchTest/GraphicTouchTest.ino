/*****************************************************************************************
  RA8875 display and GSL1680 capacitive touch controller 
  Graphical touchscreen test
  Tested on Arduino DUE, Arduino MEGA2560, ESP32 S3 Dev Module, ESP32 BPI Leaf S3
******************************************************************************************/
#include <SPI.h>
#include <RA8875.h>
#include <GSL1680.h>
// RA8875 connection
#define RA8875_INT 4
#define RA8875_CS 10
#define RA8875_RESET 9
// GSL1680 connections
#define WAKE 6
#define INTRPT 7
// Create TS touchscreen object. Use GSL1680(); if not needed serial debug
GSL1680 TS = GSL1680(true, true);
// Create the tft display object
RA8875 tft = RA8875(RA8875_CS, RA8875_RESET);
// Function to display the X and Y coordinates for touch events
void inttostr(uint16_t value, uint8_t *str) {
  str[0] = value / 1000 + 0x30;
  str[1] = value % 1000 / 100 + 0x30;
  str[2] = value % 1000 % 100 / 10 + 0x30;
  str[3] = value % 1000 % 100 % 10 + 0x30;
}

void setup() {
  Serial.begin(115200);
  // Initialize RA8875 display at a resolution 800x480 pixels
  tft.begin(RA8875_800x480);
  tft.brightness(255); // Set brightness to maximum
  tft.fillWindow(RA8875_BLACK);
  tft.setCursor(400, 40, true);
  tft.setTextColor(RA8875_CYAN);
  tft.setFontScale(1);
  tft.print(" Touchscreen initialization. ");
  // Initialize GSL1680 touchscreen driver
  TS.begin(WAKE, INTRPT);
  // Start to play with brightness then some graphics
  tft.fillWindow(RA8875_BLACK);
  tft.setCursor(400, 40, true);
  tft.setTextColor(RA8875_CYAN);
  tft.setFontScale(1);
  tft.print(" Backlight adjustment test. ");
  // PWM brightness from 255 to 0 by a step of 5
  for (uint8_t i = 255; i != 0; i -= 5 ) {
    tft.brightness(i);
    delay(10);
  }
  // PWM brightness from 0 to 255 by a step of 5
  for (uint8_t i = 0; i != 255; i += 5 ) {
    tft.brightness(i);
    delay(10);
  }
  tft.brightness(255); // Set brightness to maximum
  tft.fillWindow(RA8875_RED);
  delay(500);
  tft.fillWindow(RA8875_YELLOW);
  delay(500);
  tft.fillWindow(RA8875_GREEN);
  delay(500);
  tft.fillWindow(RA8875_CYAN);
  delay(500);
  tft.fillWindow(RA8875_MAGENTA);
  delay(500);
  tft.fillWindow(RA8875_BLACK);
  // Start some graphic acceleration
  tft.drawCircle(100, 100, 50, RA8875_BLACK);
  tft.fillCircle(100, 100, 49, RA8875_GREEN);
  tft.fillRect(11, 11, 700, 400, RA8875_BLUE);
  tft.drawRect(10, 10, 400, 200, RA8875_GREEN);
  tft.fillRoundRect(200, 10, 200, 100, 10, RA8875_RED);
  tft.fillRoundRect(480, 100, 300, 200, 10, RA8875_RED);
  tft.drawPixel(10, 10, RA8875_BLACK);
  tft.drawPixel(11, 11, RA8875_BLACK);
  tft.drawTriangle(200, 15, 250, 100, 150, 125, RA8875_BLACK);
  tft.drawEllipse(300, 100, 100, 40, RA8875_BLACK);
  tft.fillEllipse(300, 100, 98, 38, RA8875_GREEN);
  tft.drawCurve(50, 100, 80, 40, 2, RA8875_BLACK);
  tft.fillCurve(50, 100, 78, 38, 2, RA8875_WHITE);
}

void loop() {
  static uint16_t total = 256;
  static uint16_t w = tft.width();
  static uint16_t h = tft.height();
  float xScale = 1024.0F / w;
  float yScale = 1024.0F / h;
  while (total != 0)
  {
    total--;
    int16_t x  = random(w);
    int16_t y  = random(h);
    int16_t x1 = random(w);
    int16_t y1 = random(h);
    int16_t x2 = random(w);
    int16_t y2 = random(h);
    uint16_t c  = (((random(64) << 6) | random(64)) << 6) | random(31);
    uint16_t bg = (((random(64) << 6) | random(64)) << 6) | random(31);
    int16_t th = random(h);
    int16_t tw = random(w);
    int16_t op = random(9);
    int16_t en = random(8);
    switch ( op )
    {
      default:
      case 0:
        tft.fillRect(x, y, tw, th, c);
        break;
      case 1:
        tft.fillEllipse(x, y, tw / 2, th / 2, c);
        break;
      case 2:
        tft.fillCircle(x, y, tw / 2, c);
        break;
      case 3:
        tft.fillTriangle(x, y, x1, y1, x2, y2, c);
        break;
      case 4:
        tft.setCursor(x, y);
        tft.setTextColor(c, bg);
        tft.setFontScale(en);
        tft.print("H E L L O");
        break;
      case 5:
        tft.drawRect(x, y, tw, th, c);
        break;
      case 6:
        tft.drawEllipse(x, y, tw / 2, th / 2, c);
        break;
      case 7:
        tft.drawCircle(x, y, tw / 2, c);
        break;
      case 8:
        tft.drawTriangle(x, y, x1, y1, x2, y2, c);
        break;
    }
    if (total == 0)
    { tft.fillWindow(RA8875_BLACK);
      tft.setCursor(300, 40, true);
      tft.setTextColor(RA8875_CYAN);
      tft.setFontScale(1);
      tft.print(" Touchscreen test. ");
      tft.setCursor(700, 40, true);
      tft.setTextColor(RA8875_RED);
      tft.print("CLEAR");
    }
  }
  uint8_t ss[4];
  while (1)
  {
    // Waiting for touch events
    if (digitalRead(INTRPT) == HIGH)
    {
      TS.dataread();
      if (TS.ts_event.fingers == 1)
      { tft.fillCircle(TS.ts_event.x1, TS.ts_event.y1, 5, RA8875_RED);
      }
      if (TS.ts_event.fingers == 2)
      { tft.fillCircle(TS.ts_event.x1, TS.ts_event.y1, 5, RA8875_RED);
        tft.fillCircle(TS.ts_event.x2, TS.ts_event.y2, 5, RA8875_GREEN);
      }
      if (TS.ts_event.fingers == 3)
      { tft.fillCircle(TS.ts_event.x1, TS.ts_event.y1, 5, RA8875_RED);
        tft.fillCircle(TS.ts_event.x2, TS.ts_event.y2, 5, RA8875_GREEN);
        tft.fillCircle(TS.ts_event.x3, TS.ts_event.y3, 5,  RA8875_BLUE);
      }
      if (TS.ts_event.fingers == 4)
      { tft.fillCircle(TS.ts_event.x1, TS.ts_event.y1, 5, RA8875_RED);
        tft.fillCircle(TS.ts_event.x2, TS.ts_event.y2, 5, RA8875_GREEN);
        tft.fillCircle(TS.ts_event.x3, TS.ts_event.y3, 5,  RA8875_BLUE);
        tft.fillCircle(TS.ts_event.x4, TS.ts_event.y4, 5,  RA8875_CYAN);
      }
      if (TS.ts_event.fingers == 5)
      { tft.fillCircle(TS.ts_event.x1, TS.ts_event.y1, 5, RA8875_RED);
        tft.fillCircle(TS.ts_event.x2, TS.ts_event.y2, 5, RA8875_GREEN);
        tft.fillCircle(TS.ts_event.x3, TS.ts_event.y3, 5,  RA8875_BLUE);
        tft.fillCircle(TS.ts_event.x4, TS.ts_event.y4, 5,  RA8875_CYAN);
        tft.fillCircle(TS.ts_event.x5, TS.ts_event.y5, 5,  RA8875_MAGENTA);
      }
      // Touch event for CLEAR screen button
      if (TS.ts_event.x1 >= 690 && (TS.ts_event.y1 & 0x0fff) <= 60)
      { tft.fillWindow(RA8875_BLACK);
        tft.setCursor(300, 40, true);
        tft.setTextColor(RA8875_CYAN);
        tft.setFontScale(1);
        tft.print(" Touchscreen test. ");
        tft.setCursor(700, 40, true);
        tft.setTextColor(RA8875_RED);
        tft.print("CLEAR");
      }
      tft.setFontScale(0);
      // Display the coordinates for X
      inttostr(TS.ts_event.x1 & 0x0fff, ss);
      tft.setCursor(100, 100);
      tft.setTextColor(RA8875_YELLOW, RA8875_BLACK);
      tft.print("X =  ");
      tft.setCursor(140, 100);
      tft.writeCommand(RA8875_MRWC);
      tft.writeData16(ss[0]);
      delay(1);
      tft.writeData16(ss[1]);
      delay(1);
      tft.writeData16(ss[2]);
      delay(1);
      tft.writeData16(ss[3]);
      // Display the coordinates for Y
      inttostr(TS.ts_event.y1 & 0x0fff, ss);
      tft.setCursor(100, 140);
      tft.print("Y =  ");
      tft.setCursor(140, 140);
      tft.writeCommand(RA8875_MRWC);
      tft.writeData16(ss[0]);
      delay(1);
      tft.writeData16(ss[1]);
      delay(1);
      tft.writeData16(ss[2]);
      delay(1);
      tft.writeData16(ss[3]);
    }
  }
}