//#include <Adafruit_PWMServoDriver.h>
//#include "LEDFader.h"
//#include <Wire.h>

#include "drawer.h"

//Drawer(uint8_t level, bool right, uint8_t pwmPin, uint8_t sensorPin );
//Drawer(uint8_t level, uint8_t ledPin, uint8_t sensorPin );

//Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();
Drawer drawers[5];
bool drawerOpen;

void setup() {
  //LEDFader::init(pwm);
  Serial.begin(115200);
  Serial.println();
  
  drawerOpen =false;
  //Drawer drawer1 = Drawer(1, 6, A0 );
  drawers[0] = Drawer(1, 6, A7 );
  drawers[1] = Drawer(2, 7, A6 );
  drawers[2] = Drawer(3, 3, A3 );
  drawers[3] = Drawer(4, 4, A2 );
  drawers[4] = Drawer(5, 5, A1 );
}

void loop() {
  uint8_t lowestExtended = 255;
  for (uint8_t i = 4; i >= 0 && i < 6; i--) {
    if (drawers[i].extended()) {
      drawerOpen = true;
      lowestExtended = i;
      for (uint8_t j = i; j >= 0 && j < 6; j--) {
        drawers[j].lightOn();
        delay(250);
      }
      break;
    }
  }
  if ( drawerOpen ) {
    lowestExtended = lowestExtended < 4 ? lowestExtended : 4;
    for (uint8_t j = 4; j > lowestExtended && j < 6; j--) {
      drawers[j].lightOff();
      delay(250);
    }
    drawerOpen = false;
  }
}
