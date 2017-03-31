#include <EEPROM.h>
#include "structs.h"

#include <U8x8lib.h>
U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);

/*
	Setup sensors and LEDs.
*/

const uint8_t ledArray[] = {5,6,9,10,11};
Sensor sensors[5] = {{A0,0},{A1,0},{A2,0},{A3,0},{A6,0}};
Drawer drawers[5];


void setup() {
	for(uint8_t i = 0; i < 5; i++) {
		uint8_t LED = ledArray[i];
		pinMode(LED, OUTPUT);
		digitalWrite(LED,LOW);
	}
	pinMode(13, OUTPUT); // GG . Visual indicator to close drawer

	// initialize serial communication at 115200 bits per second:
	Serial.begin(115200);
	Serial.println("============");
	u8x8.begin();
  	u8x8.setPowerSave(0);
	u8x8.setFont(u8x8_font_chroma48medium8_r);
	
	// Input. 5 sensors
	for(uint8_t i = 0; i < 5; i++) {
		pinMode(sensors[i].pin, INPUT);
		for(uint8_t j = 0; j < 64; j++) {
			sensors[i].baseValue += analogRead(sensors[i].pin);
		}
		sensors[i].baseValue /= 64;
	}

	for(uint8_t i = 0; i < 5; i++) {
		String printString = printBaseValue(sensors[i].pin);
		u8x8.drawString(0,i, printString.c_str() );
		Serial.println( printString );
	}
	Serial.println("============");

	// Get sensor for one drawer at a time.
	for(uint8_t drawer = 0; drawer < 5; drawer++) {
		findSensors(drawer);
		delay(500);
	}
	Serial.println("Done with sensors.");
	u8x8.clear();

	
	// Output
	//for(uint8_t LED = 4; LED < 9; LED++) {
	for(uint8_t i = 0; i < 5; i++) {
		uint8_t LED = ledArray[i];
		pinMode(LED, OUTPUT);
		// Get sensor for one drawer at a time.
		delay(500);
		findLEDs(LED);
	}

	Serial.println("Done with LEDs.");

	u8x8.clear();
	u8x8.draw2x2UTF8(0,0, " Config");

	// GG. Persistant storage
	for(uint8_t i = 0; i < 5; i++) {
		EEPROM.write(i*2, drawers[i].sensor.pin);
		EEPROM.write(1 + i*2, drawers[i].ledPin);
		u8x8.setCursor(0, i + 2);
		u8x8.print("Drawer ");
		u8x8.print(i + 1);
		u8x8.print(":");
	}
}
int baseValue = 0;
// the loop routine runs over and over again forever:
void loop() {
	// Serial.println("===============================================");
	for(uint8_t i = 0; i < 5; i++) { 
//		uint8_t LED = ledArray[i];
//		digitalWrite(LED,LOW);

		u8x8.setCursor(10, i + 2);
		u8x8.print(getSensorValue(drawers[i].sensor.pin));
		u8x8.print("   ");
		float something = millis()/1000.0;
 		int value = 128.0 + 128 * sin( something * PI + 1.256 * i);
 		analogWrite(drawers[i].ledPin, value);
/*		u8x8.print(" with value ");
		u8x8.print(drawers[i].sensor.baseValue);
		u8x8.print(" has LED pin ");
		u8x8.println(drawers[i].ledPin);
		Serial.print("Drawer ");
		Serial.print(i + 1);
		Serial.print(" has sensor pin ");
		Serial.print(drawers[i].sensor.pin);
		Serial.print(" with value ");
		Serial.print(drawers[i].sensor.baseValue);
		Serial.print(" has LED pin ");
		Serial.println(drawers[i].ledPin);
*/
	}
//	delay(1e3);
}
// GG. Nicer to make this generic.
void findSensors(uint8_t drawer) {
	Serial.print("Please open drawer ");
	Serial.println(drawer + 1);
	// u8x8.drawString(0,5, "Open drawer ");
	// u8x8.setCursor(12, 5);
	// u8x8.print( String(drawer + 1).c_str() );
	// u8x8.print(" ");
	u8x8.draw2x2UTF8(4,0, "Open");
	u8x8.draw2x2UTF8(2,2, "drawer");
	u8x8.draw2x2UTF8(7,4, String(drawer + 1).c_str());
	bool allClosed = true;
	uint8_t foundSensor = 0;
	while (allClosed) { 
		for(uint8_t i = 0; i < 5; i++) {
			uint16_t tempValue = 0;
			for(uint8_t j = 0; j < 64; j++) {
				tempValue += analogRead(sensors[i].pin);
			}
			tempValue /= 64;
			if (sensors[i].baseValue > tempValue + 50) {
				allClosed = false;
				foundSensor = sensors[i].pin;
				drawers[drawer].sensor = sensors[i];
				break;
			}
		}
	}
	digitalWrite(13, HIGH);
	u8x8.clear();
//	u8x8.drawString(0,5, "Close drawer");
//	u8x8.setCursor(13, 5);
//	u8x8.print( drawer + 1 );
	u8x8.draw2x2UTF8(3,0, "Close");
	u8x8.draw2x2UTF8(2,2, "drawer");
	u8x8.draw2x2UTF8(7,4, String(drawer + 1).c_str());
	u8x8.setCursor(0, 6);
	u8x8.print( "Pin" );
	u8x8.setCursor(4, 6);
	u8x8.print( foundSensor );
	u8x8.setCursor(7, 6);
	u8x8.print( "for d");
	u8x8.setCursor(13, 6);
	u8x8.print( drawer + 1 );

	Serial.print("Found sensor pin ");
	Serial.print(foundSensor);
	Serial.print(" for drawer ");
	Serial.println(drawer + 1);
	Serial.print("Please close drawer ");
	Serial.println(drawer + 1);
	while (allClosed == false) {
		uint16_t tempValue = 0;
		for(uint8_t j = 0; j < 64; j++) {
			tempValue += analogRead(foundSensor);
		}
		tempValue /= 64;
		if (drawers[drawer].sensor.baseValue < tempValue + 50) {
			allClosed = true;
		}
	}
	digitalWrite(13, LOW);
}
// GG. Nicer to make this generic.
void findLEDs(uint8_t LED) {
	digitalWrite(LED, HIGH);
	Serial.println("Please open the drawer that is lit");
	u8x8.draw2x2UTF8(0,0, "Open lit");
	u8x8.draw2x2UTF8(2,2, "drawer");
	delay(1000);
	bool allClosed = true;
	uint8_t drawer = 0;
	while( allClosed ) {
		for(uint8_t i = 0; i < 5; i++) {
			uint16_t tempValue = getSensorValue(drawers[i].sensor.pin);
			if (drawers[i].sensor.baseValue > tempValue + 50) {
				allClosed = false;
				drawer = i;
				drawers[i].ledPin = LED;
				break;
			}
		}
	}
	digitalWrite(13, HIGH);
	u8x8.clear();
	u8x8.draw2x2UTF8(3,0, "Close");
	u8x8.draw2x2UTF8(2,2, "drawer");
	u8x8.draw2x2UTF8(7,4, String(drawer + 1).c_str());

	u8x8.setCursor(0, 6);
	u8x8.print("Found drawer ");
	u8x8.print(drawer + 1);
	u8x8.setCursor(0, 7);
	u8x8.print("for LED pin ");
	u8x8.print(LED);
	
	Serial.print("Found drawer ");
	Serial.print(drawer);
	Serial.print(" for LED ");
	Serial.println(LED);
	Serial.print("Please close drawer ");
	Serial.println(drawer);
	while (allClosed == false) {
		uint16_t tempValue = getSensorValue(drawers[drawer].sensor.pin);
		if (drawers[drawer].sensor.baseValue < tempValue + 50) {
			allClosed = true;
		}
	}
	digitalWrite(13, LOW);
	digitalWrite(LED, LOW);
}

uint16_t getSensorValue(uint8_t sensorPin) {
	uint16_t tempValue = 0;
	for(uint8_t j = 0; j < 64; j++) {
		tempValue += analogRead(sensorPin);
	}
	return tempValue / 64;
}

String printBaseValue(int i){
	String printString = "Pin ";
	printString += i;
	printString += ": ";
	printString += analogRead(i);
//	Serial.print("Pin ");
//	Serial.print(i);
//	Serial.print(": ");
//	baseValue = analogRead(i);
	return printString;
}