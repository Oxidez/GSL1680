/*****************************************************************************************
  RA8875 display and GSL1680 capacitive touch controller 
  Simple code to display the X1 and Y1 coordinates for a touch event, on the serial port
  Tested on Arduino DUE, Arduino MEGA2560, ESP32 S3 Dev Module, ESP32 BPI Leaf S3
******************************************************************************************/
#include <SPI.h>
#include <GSL1680.h>
// Touch Screen Pins - modify acording to connections
#define WAKE 6
#define INTRPT 7
GSL1680 TS = GSL1680(true,true); // Serial debug active
// GSL1680 TS = GSL1680();      No serial debug
void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println(""); 
  Serial.println(" Arduino DUE + GSL1680 library ");
  Serial.println("");
  TS.begin(WAKE, INTRPT);  // Initialize GSL1680
}

void loop() {
  if (digitalRead(INTRPT) == HIGH) {
    TS.dataread();
    int X1 = TS.ts_event.x1 & 0x0fff;
    int Y1 = TS.ts_event.y1 & 0x0fff;
    if (TS.ts_event.fingers >= 1) {
        Serial.print("Fingers : ");
        Serial.print(TS.ts_event.fingers); // Fingers counter
        Serial.print(" - ");
        Serial.print(X1);                  // X1 coordinate
        Serial.print(" - ");
        Serial.print(Y1);                  // Y1 coordinate
        Serial.println("");
    }
  }
}