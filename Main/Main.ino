#include <EEPROM.h>
//#include "SoftPWM.h"
#include "structs.h"
#include <U8x8lib.h>
U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);

/*
	Use sensors and LEDs.
*/

Drawer drawers[5];
Lighting lighting[5];
volatile uint8_t on = 0;

void setup() {
	Serial.begin(115200);

	u8x8.begin();
	u8x8.setPowerSave(0);
	u8x8.setFont(u8x8_font_chroma48medium8_r);
	
	for(uint8_t i = 0; i < 54; i++) Serial.print("=");
	Serial.println("=");

	for(uint8_t i = 0; i < 5; i++) {
		drawers[i].sensor.pin = EEPROM.read(i*2);
		drawers[i].sensor.baseValue = getDrawerSensorValue(drawers[i]); 
		drawers[i].ledPin = EEPROM.read(1 + i*2);
		pinMode(drawers[i].sensor.pin,INPUT);
		pinMode(drawers[i].ledPin, OUTPUT);
		lighting[i].ledPin = drawers[i].ledPin;

		Serial.print("Drawer ");
		Serial.print(i + 1);
		Serial.print(" has sensor pin ");
		Serial.print(drawers[i].sensor.pin);
		Serial.print(" with value ");
		Serial.print(drawers[i].sensor.baseValue);
		Serial.print(" has LED pin ");
		Serial.println(drawers[i].ledPin);
	}
//	setupPWMpins();
//	setupInterruptTimer(0,0);
}
uint16_t updateTimer = millis();
int8_t lowestOpenDrawer = -1;
const uint32_t fadeDuration = 1e7; // 10 sec
const uint32_t fadeSeparation = 2e6; // 2 sec
void loop() {
	withPwm();
	//noPwm();
	updatePwm();
}
void updatePwm() {
	for (uint8_t i = 0; i < 5; i++)	{
		if (lighting[i].currentValue != lighting[i].endValue) {
			int32_t deltaTime = lighting[i].endTime - lighting[i].startTime; //  Should be fadeDuration if no overflow
			if(deltaTime > 0) { // Most cases. Except for when there is an overflow of micros()
				int8_t previousValue = lighting[i].currentValue;
				int32_t timeLeft = lighting[i].endTime - micros();
				if( timeLeft > deltaTime ) {
					lighting[i].currentValue = lighting[i].startValue;
				}
				else if(timeLeft > 0) {
					int16_t valueDiff = (lighting[i].endValue - lighting[i].startValue); // can be positive or negative
					// Progress can easily be made with fixed point if resources are scarse
					float progress = (deltaTime - timeLeft) / deltaTime; // Between 0.0 and 1.0
					// Enter lighting curve function here.
					float normy = pow(progress, 2.5 /*gamma*/);
					//(unsigned int) (0.5 + normy * maxLevel);
					lighting[i].currentValue = lighting[i].startValue + normy * valueDiff + 0.5;
					// Linear
					//lighting[i].currentValue = lighting[i].startValue + progress * valueDiff;
				} else { // Times up for updating
					lighting[i].currentValue = lighting[i].endValue;
				}
				if(previousValue != lighting[i].currentValue){
					analogWrite(lighting[i].ledPin, lighting[i].currentValue);
				}
			}
		}
	}
}

void withPwm() {
	uint32_t startTime = micros();
	for(int8_t i = 4; i >= 0 && i >= lowestOpenDrawer; i--) {
		Drawer drawer = drawers[i];
		if( drawerIsOpen(drawer) ) {
			// See if this is all ready handled. Only set thing the first time it is the lowest open drawer
			if(lowestOpenDrawer != i) {
				// Only set for drawers without a started sequence.
				for(int8_t j = i; j >= 0 && j > lowestOpenDrawer; j--){
					if( lighting[i].endValue != 255 ) {
						lighting[i].startTime = startTime;
						lighting[i].endTime = startTime + fadeDuration;
						lighting[i].startValue = lighting[i].currentValue;
						lighting[i].endValue = 255;
						startTime += fadeSeparation;
					}
				}
				lowestOpenDrawer = i;
			}
			// STOP processing after a drawer is found open.
			break;
		} else {
			// GG. if drawer is closed reset the lowestOpenDrawer variable
			if (i == lowestOpenDrawer) {
				lowestOpenDrawer = -1;
			}
			if(lighting[i].endValue != 0) {
				lighting[i].startTime = startTime;
				lighting[i].endTime = startTime + fadeDuration;
				lighting[i].startValue = lighting[i].currentValue;
				lighting[i].endValue = 0;
				startTime += fadeSeparation;
			}
		}
	}
}
void noPwm() {
	for(int8_t i = 4; i >= 0; i--) {
		Drawer drawer = drawers[i];
		if( drawerIsOpen(drawer) ) {
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

// ISR(TIMER1_COMPA_vect){  //change the 0 to 1 for timer1 and 2 for timer2
//    //interrupt commands here
//    //updatePwmValues();
//    //digitalWrite(4, !digitalRead(4));
// /*   if(on == 1) {
// 	   PORTB |= pinBit; //= PORTD | 0b00000000; // toggle D4
// 	   on = 0;
//    } else {
// 	   PORTB &= ~pinBit; //PORTD = PORTD | 0b00000100; // toggle D4
// 	   on = 1;
//    }
// */
// 	updatePwmValues();
// }

// ISR(TIMER2_COMPA_vect){
// 	handlePWM();
// }

// void setupInterruptTimer(uint32_t us, uint8_t timer){
	
// 	/*
// 	uint16_t f = 10^6 / us;
// 	uint16_t compareReg = (16*10^6) / (f*64);

// 	// 1, 8, 64, 256, 1024 are the prescalers for 0b001, 0b010, 0b011, 0b100, 0b101
// 	*/ 
// 	// GG. Setup the 1000Hz update brightness value
// 	cli();
// /*	TCCR0A = 0;// set entire TCCR0A register to 0
// 	TCCR0B = 0;// same for TCCR0B
// 	TCNT0  = 0;// initialize counter value to 0
// 	// set compare match register for 2khz increments
// 	OCR0A = 249;// = (16*10^6) / (1000*64) - 1 (must be <256)
// 	// turn on CTC mode
// 	TCCR0A |= (1 << WGM01);
// 	// Set CS01 and CS00 bits for 64 prescaler
// 	TCCR0B |= (1 << CS01) | (1 << CS00);   
// 	// enable timer compare interrupt
// 	TIMSK0 |= (1 << OCIE0A);
// */
// 	TCCR1A = 0;// set entire TCCR1A register to 0
// 	TCCR1B = 0;// same for TCCR1B
// 	TCNT1  = 0;//initialize counter value to 0
// 	// set compare match register for 1hz increments
// 	OCR1A = 249; // = (16*10^6) / (1000*64) - 1 (must be <65536)
// 	// turn on CTC mode
// 	TCCR1B |= (1 << WGM12);
// 	// Set CS10 and CS12 bits for 1024 prescaler
// 	TCCR1B |= (1 << CS12) | (1 << CS10);  
// 	// enable timer compare interrupt
// 	TIMSK1 |= (1 << OCIE1A);


// 	// GG. Setup the 100KHz SoftPWM handler. (100kHz / 8) now
// 	TCCR2A = 0;// set entire TCCR2A register to 0
// 	TCCR2B = 0;// same for TCCR2B
// 	TCNT2  = 0;// initialize counter value to 0
// 	// set compare match register for 8khz increments
// 	// Ska vara 159 med prescaler 1 för 100KHz OCR2A = 159;// = (16*10^6) / (8000*8) - 1 (must be <256)
// 	OCR2A = 59;
// 	// turn on CTC mode
// 	TCCR2A |= (1 << WGM21);
// 	// Set CS21 bit for 8 prescaler
// 	TCCR2B |= (1 << CS21);   
// 	// enable timer compare interrupt
// 	TIMSK2 |= (1 << OCIE2A);
// 	sei();
// }