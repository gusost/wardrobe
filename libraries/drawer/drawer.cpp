/* 
 * File:   drawer.cpp
 * Author: Gustav Öst
 *
 * Created on April 6, 2015
 */
 #define DEBUG
 #include "drawer.h"

/* 
  uint8_t position; // GG. bitfield XXXXlevel{3}left/right{1}
  uint8_t pins;     // GG. Sensorpin{4} and getPwmPin(){4}
  uint8_t startLevel, targetLevel, currentLevel;
  unsigned int sensorMeanValue;
*/

  Drawer::Drawer() {
    
  }
  Drawer::Drawer(uint8_t level, bool right, uint8_t led, uint8_t sensor) {
    position = (level << 1 ) + (right ? 1 : 0);
    pwmPin = led; 
    sensorPin = sensor;
    startLevel = targetLevel = currentLevel = 128;
    sensorMeanValue = this->getSensorValue();
  }

  Drawer::Drawer(uint8_t level, uint8_t ledPin, uint8_t sensor ) {
    position = level;
    pwmPin = ledPin; 
    sensorPin = sensor;
    sensorMeanValue = this->getSensorValue();
 #ifdef DEBUG
    Serial.print("Level ");
    Serial.print(level);
    Serial.print(" initiated with sensor value: ");
    Serial.println(sensorMeanValue);
 #endif
  }
  
  void Drawer::setLevel(uint8_t level) {
    currentLevel = level;
  }
  
  void Drawer::printName() {
    Serial.print("Låda ");
    Serial.print(position);
//    Serial.print(" på ");
//    Serial.print( (position & 0x1) ? "höger" : "vänster");
//    Serial.print(" sida");
  }

  uint16_t Drawer::lightOn() {
    digitalWrite(pwmPin, HIGH);
 #ifdef DEBUG
    this->printName();
    Serial.print(" is turning on. Sensor value: ");
    Serial.print(this->getSensor());
    Serial.print(" calibration value: ");
    Serial.println(sensorMeanValue);
 #endif
  }
  uint16_t Drawer::lightOff() {
    digitalWrite(pwmPin, LOW);
 #ifdef DEBUG
    this->printName();
    Serial.println(" is turning off");
 #endif
  }  
  uint8_t Drawer::getPwmPin() {
    return pwmPin;
  }
  
  uint8_t Drawer::getSensorPin() {
    return sensorPin;
  }
  
  unsigned int Drawer::getLevel() {
    return currentLevel;
  }
  
  bool Drawer::extended() {
    // GG. Do a read sensor function. Power down to ADC mode, avrage, uniq sensor considerations etc.
//    unsigned int sensor = getSensor();
    return ( this->getSensor() < sensorMeanValue - 50 ); // GG. Begin static. Each sensor should have its own value
  }
  void Drawer::updateLevel() {
    return;
  }

//  private:
  unsigned int Drawer::getSensor() {
    uint16_t val = this->getSensorValue();
#ifdef DEBUG
/*
    this->printName();
    Serial.print(" ");
    Serial.print( val );
    Serial.println();
*/
#endif
    return val;
  }
  unsigned int Drawer::getSensorValue() {
    unsigned int sensorValue = 0;
    for(uint8_t i = 0; i < 64; i++) {
      sensorValue += analogRead(sensorPin);
    }
    sensorValue = sensorValue / 64;
    return sensorValue;
  }

