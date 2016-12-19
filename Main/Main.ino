#include <EEPROM.h>
#include "SoftPWM.h"
#include "structs.h"
/*
	Use sensors and LEDs.
*/

Drawer drawers[5];
volatile uint8_t on = 0;
// PORTD has pinbits 4-7 for pins 4-7
// pin 8 has pinbit 0 for port c
// uint8_t pinBit = _BV(4); //digitalPinToBitMask(4);
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
	setupInterruptTimer(0,0);
}
uint16_t updateTimer = millis();
void loop() {
//	noPwm();
	withPwm();
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
	if (drawer.sensor.baseValue > getSensorValue(drawer.sensor.pin) + 50 ) {
		return true;
	}
	return false;
}

void withPwm(){
	uint32_t doneMillis = millis() + 125;
	for(int8_t i = 4; i >= 0; i--) {
		Drawer drawer = drawers[i];
		if( drawerIsOpen(drawer) ) {
			// GG. This RESETS the doneMillis from turning drawers off. If this is not done a long delay will be felt when opening a drawer far up
			for(int8_t j = i; j >= 0; j--){
				doneMillis += 125;
				setPwmLevelForDrawer(j, 255, doneMillis);
			}
			break;
		} else {
			doneMillis = millis() + 125;
			setPwmLevelForDrawer(i, 0, doneMillis);
		}
	}
	int16_t doneDelay = doneMillis - millis();
	if (doneDelay > 0) {
		delay(doneDelay + 5); // { // Wait until done for now. Events should do stuff.	
	}
/*		delay(10);
		Serial.println("millis doneMillis");
		Serial.println(millis()); // = millis();
		Serial.println(doneMillis); // = millis();
*/
}

void noPwm() {
	for(int8_t i = 4; i >= 0; i--) {
		Drawer drawer = drawers[i];
		if( drawerIsOpen(drawer) ) {
			// GG. This RESETS the doneMillis from turning drawers off. If this is not done a long delay will be felt when opening a drawer far up
			for(int8_t j = i; j >= 0; j--){
				digitalWrite(drawers[j].ledPin, HIGH);
				delay(250);
			}
			break;
		} else {
			if (digitalRead(drawer.ledPin) == HIGH) {
				digitalWrite(drawer.ledPin, LOW);		
				delay(250);
			} 
		}
	}
}

ISR(TIMER1_COMPA_vect){  //change the 0 to 1 for timer1 and 2 for timer2
   //interrupt commands here
	updatePwmValues();
}

ISR(TIMER2_COMPA_vect){
	handlePWM();
}

void setupInterruptTimer(uint32_t us, uint8_t timer){
	
	/*
	uint16_t f = 10^6 / us;
	uint16_t compareReg = (16*10^6) / (f*64);

	// 1, 8, 64, 256, 1024 are the prescalers for 0b001, 0b010, 0b011, 0b100, 0b101
	*/ 
	// GG. Setup the 1000Hz update brightness value
	cli();
/*	TCCR0A = 0;// set entire TCCR0A register to 0
	TCCR0B = 0;// same for TCCR0B
	TCNT0  = 0;// initialize counter value to 0
	// set compare match register for 2khz increments
	OCR0A = 249;// = (16*10^6) / (1000*64) - 1 (must be <256)
	// turn on CTC mode
	TCCR0A |= (1 << WGM01);
	// Set CS01 and CS00 bits for 64 prescaler
	TCCR0B |= (1 << CS01) | (1 << CS00);   
	// enable timer compare interrupt
	TIMSK0 |= (1 << OCIE0A);
*/
	TCCR1A = 0;// set entire TCCR1A register to 0
	TCCR1B = 0;// same for TCCR1B
	TCNT1  = 0;//initialize counter value to 0
	// set compare match register for 1hz increments
	OCR1A = 249; // = (16*10^6) / (1000*64) - 1 (must be <65536)
	// turn on CTC mode
	TCCR1B |= (1 << WGM12);
	// Set CS10 and CS12 bits for 1024 prescaler
	TCCR1B |= (1 << CS12) | (1 << CS10);  
	// enable timer compare interrupt
	TIMSK1 |= (1 << OCIE1A);


	// GG. Setup the 100KHz SoftPWM handler. (100kHz / 8) now
	TCCR2A = 0;// set entire TCCR2A register to 0
	TCCR2B = 0;// same for TCCR2B
	TCNT2  = 0;// initialize counter value to 0
	// set compare match register for 8khz increments
	// Ska vara 159 med prescaler 1 för 100KHz OCR2A = 159;// = (16*10^6) / (8000*8) - 1 (must be <256)
	OCR2A = 79;
	// turn on CTC mode
	TCCR2A |= (1 << WGM21);
	// Set CS21 bit for 8 prescaler
	TCCR2B |= (1 << CS21);   
	// enable timer compare interrupt
	TIMSK2 |= (1 << OCIE2A);
	sei();
}