#include <EEPROM.h>
/*
	Setup sensors and LEDs.
*/

typedef struct {
	uint8_t pin;
	uint16_t baseValue;
} Sensor;


typedef struct {
	Sensor sensor;
	uint8_t ledPin;
} Drawer;

Sensor sensors[5] = {{A1,0},{A4,0},{A5,0},{A6,0},{A7,0}};
Drawer drawers[5];


void setup() {

	pinMode(13, OUTPUT); // GG . Visual indicator to close drawer

	// initialize serial communication at 9600 bits per second:
	Serial.begin(115200);
	Serial.println("============");
	
	// Input. 5 sensors
	for(uint8_t i = 0; i < 5; i++) {
		pinMode(sensors[i].pin, INPUT);
		for(uint8_t j = 0; j < 64; j++) {
			sensors[i].baseValue += analogRead(sensors[i].pin);
		}
		sensors[i].baseValue /= 64;
	}

	for(uint8_t i = 0; i < 5; i++) {
		printbaseValue(sensors[i].pin);
	}
	Serial.println("============");

	// Get sensor for one drawer at a time.
	for(uint8_t drawer = 0; drawer < 5; drawer++) {
		findSensors(drawer);
	}
	Serial.println("Done with sensors.");

	// Output
	for(uint8_t LED = 4; LED < 9; LED++) {
		pinMode(LED, OUTPUT);
		// Get sensor for one drawer at a time.
		delay(500);
		findLEDs(LED);
	}

	Serial.println("Done with LEDs.");

	// GG. Persistant storage
	for(uint8_t i = 0; i < 5; i++) {
		EEPROM.write(i*2, drawers[i].sensor.pin);
		EEPROM.write(1 + i*2, drawers[i].ledPin);
	}
}
int baseValue = 0;
// the loop routine runs over and over again forever:
void loop() {
	Serial.println("===============================================");
	for(uint8_t i = 0; i < 5; i++) { 
		Serial.print("Drawer ");
		Serial.print(i + 1);
		Serial.print(" has sensor pin ");
		Serial.print(drawers[i].sensor.pin);
		Serial.print(" with value ");
		Serial.print(drawers[i].sensor.baseValue);
		Serial.print(" has LED pin ");
		Serial.println(drawers[i].ledPin);
	}
	delay(60e3);
}
// GG. Nicer to make this generic.
void findSensors(uint8_t drawer) {
	Serial.print("Please open drawer ");
	Serial.println(drawer + 1);
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
	Serial.print("Found drawer ");
	Serial.print(drawer);
	Serial.print(" for LED ");
	Serial.println(LED - 3);
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

void printbaseValue(int i){
		Serial.print("Pin ");
		Serial.print(i);
		Serial.print(": ");
		baseValue = analogRead(i);
		Serial.println( baseValue );
}