#include <EEPROM.h>
#include "SoftPWM.h"
#include "structs.h"
/*
	Use sensors and LEDs.
*/

Drawer drawers[5];
// variables for pattern timing
/*
unsigned long currentMillis = millis();
unsigned long previousMillis = 0;
unsigned long millisInterval = 10;
*/
void setup() {
	Serial.begin(115200);
	for(uint8_t i = 0; i < 54; i++) Serial.print("=");
	Serial.println("=");

	for(uint8_t i = 0; i < 5; i++) {
		drawers[i].sensor.pin = EEPROM.read(i*2);
		drawers[i].sensor.baseValue = getDrawerSensorValue(drawers[i]); 
		drawers[i].ledPin = EEPROM.read(1 + i*2);
		pinMode(drawers[i].sensor.pin,INPUT);
		pinMode(drawers[i].ledPin, OUTPUT);

		Serial.print("Drawer ");
		Serial.print(i + 1);
		Serial.print(" has sensor pin ");
		Serial.print(drawers[i].sensor.pin);
		Serial.print(" with value ");
		Serial.print(drawers[i].sensor.baseValue);
		Serial.print(" has LED pin ");
		Serial.println(drawers[i].ledPin);
	}
	setupPWMpins();
}

void loop() {
	for(int8_t i = 4; i >= 0; i--) {
		Drawer drawer = drawers[i];
		if( drawerIsOpen(drawer) ) {
			for(int8_t j = i; j >= 0; j--){
				setPwmLevelForDrawer(j,255);
//				digitalWrite(drawers[j].ledPin, HIGH);
				handlePWM(); // GG. Temp
				delay(250);
			}
			break;
		} else {
			setPwmLevelForDrawer(i,0);
			//digitalWrite(drawer.ledPin, LOW);
		}
	}
	handlePWM();
}

uint16_t getDrawerSensorValue(Drawer drawer) {
	return getSensorValue(drawer.sensor.pin);
}
uint16_t getSensorValue(uint8_t sensorPin) {
	uint16_t tempValue = 0;
	for(uint8_t j = 0; j < 64; j++) {
		tempValue += analogRead(sensorPin);
	}
	return tempValue / 64;
}
bool drawerIsOpen(Drawer drawer) {
	uint16_t tempValue = getSensorValue(drawer.sensor.pin);
	if (drawer.sensor.baseValue > tempValue + 50) {
		return true;
	}
	return false;
}