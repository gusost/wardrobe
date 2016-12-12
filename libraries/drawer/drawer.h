/* 
 * File:   drawer.h
 * Author: Gustav Öst
 *
 * Created on April 6, 2015
 */
 
#ifndef DRAWER_H
#define	DRAWER_H

#include <Arduino.h>

#define LEFT 0
#define RIGHT 1

class Drawer {
  
  uint8_t position; // GG. bitfield XXXXlevel{3}left/right{1}
  uint8_t sensorPin;
  uint8_t pwmPin;
  uint8_t startLevel, targetLevel, currentLevel;
  unsigned int sensorMeanValue;
  
  public:
    Drawer();
    Drawer(uint8_t height, bool right, uint8_t ledPin, uint8_t sensor );
    Drawer(uint8_t level, uint8_t ledPin, uint8_t sensorPin );
    uint8_t getPwmPin();
    uint8_t getSensorPin();
    uint16_t lightOn();
    uint16_t lightOff();
    void printName();
    void setLevel(uint8_t level);
    unsigned int getLevel();
    bool extended();
    void updateLevel();

  private:
    unsigned int getSensorValue();
    unsigned int getSensor();
};

#endif
