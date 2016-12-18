#include "Arduino.h"
#include "SoftPWM.h"

// variables for software PWM
unsigned long currentMicros = micros();
unsigned long previousMicros = 0;
// this is the frequency of the sw PWM
// frequency = 1/(2 * microInterval)
unsigned long microInterval = 25;
 
const uint16_t pwmMax = 255;
 
// fading (for the timing)
int fadeIncrement = 1;
// create the sw pwm pins
// these can be any I/O pin
// that can be set to output!
const int pinCount = 5;
const byte pins[pinCount] = {8,7,6,5,4}; // GG Rigth for now. Make dynamic
 
pwmPin myPWMpins[pinCount];
 
void setPwmLevelForDrawer(uint8_t drawer, uint8_t level) {
	myPWMpins[drawer].pwmValue = level;
	myPWMpins[drawer].doneTime = millis();
}
void setPwmLevelForDrawer(uint8_t drawer, uint8_t level, uint32_t doneTime) {
	myPWMpins[drawer].finalPwmValue = level;
	myPWMpins[drawer].doneTime = doneTime;
}
void updatePwmValues() {
	for (int index=0; index < pinCount; index++) {
		int16_t pwmDiff =  myPWMpins[index].finalPwmValue - myPWMpins[index].pwmValue;
		// Should it change?
		if( pwmDiff != 0) {
			int32_t timeDiff = myPWMpins[index].doneTime - millis();
			if(timeDiff < 0 || myPWMpins[index].doneTime == 0) {
				myPWMpins[index].pwmValue = myPWMpins[index].finalPwmValue;
				myPWMpins[index].doneTime = 0;
			}
			// Is it increasing and should it change now?
			else if (pwmDiff > 0 && timeDiff < pwmDiff) { // Looks wierd but am using one pwm step per ms.
				myPWMpins[index].pwmValue = myPWMpins[index].finalPwmValue - timeDiff;
			} 
			else if ( pwmDiff < 0 && timeDiff < 0 - pwmDiff ) {
				myPWMpins[index].pwmValue = myPWMpins[index].finalPwmValue + timeDiff;
			}
			// Needs the case where timeDone
		}
	}
}

void setupPWMpins() {
	for (int index=0; index < pinCount; index++) {
		myPWMpins[index].pin = pins[index];
 
		// mix it up a little bit
		// changes the starting pwmValue for odd and even
		myPWMpins[index].doneTime = 0;
		myPWMpins[index].pwmValue = 0;
		myPWMpins[index].pinState = OFF;
		myPWMpins[index].pwmTickCount = 0;
		myPWMpins[index].finalPwmValue = 0;
 
		// unlike analogWrite(), this is necessary
		pinMode(pins[index], OUTPUT);
	}
}
 
void pwmFadePattern() {
	// go through each sw pwm pin, and increase
	// the pwm value. this would be like
	// calling analogWrite() on each hw pwm pin
	for (int index=0; index < pinCount; index++) {
//		myPWMpins[index].pwmValue += fadeIncrement;
//		if (myPWMpins[index].pwmValue > pwmMax - 1 || myPWMpins[index].pwmValue < fadeIncrement)
			myPWMpins[index].pwmValue = 0;
	}
}
 
void handlePWM() {
	// GG. Timer event driven now. no need to check manually.
//	currentMicros = micros();
	// check to see if we need to increment our PWM counters yet
//	if (currentMicros - previousMicros >= microInterval) {
		// Increment each pin's counter
		for (int index=0; index < pinCount; index++) {
			// determine if we're counting on or off time
			if(myPWMpins[index].pwmValue == pwmMax) {
				if(myPWMpins[index].pinState == ON) {
					continue;
				}
				myPWMpins[index].pinState = ON;
			}
			else if(myPWMpins[index].pwmValue == 0) {
				if(myPWMpins[index].pinState == OFF) {
					continue;
				}
				myPWMpins[index].pinState = OFF;
			}
						// each pin has its own tickCounter
			myPWMpins[index].pwmTickCount++;
			if (myPWMpins[index].pinState == ON) {
				// see if we hit the desired on percentage
				// not as precise as 255 or 1024, but easier to do math
				if (myPWMpins[index].pwmTickCount >= myPWMpins[index].pwmValue) {
					myPWMpins[index].pinState = OFF;
				}
			} else {
				// if it isn't on, it is off
				if (myPWMpins[index].pwmTickCount >= pwmMax) {
					myPWMpins[index].pinState = ON;
					myPWMpins[index].pwmTickCount = 0;
				}
			}
			// could probably use some bitwise optimization here, digitalWrite()
			// really slows things down after 10 pins.
			if (myPWMpins[index].pin < 8) {
				if(myPWMpins[index].pinState){
					PORTD |= _BV(myPWMpins[index].pin); //= PORTD | 0b00000000;
				} else {
					PORTD &= ~(_BV(myPWMpins[index].pin)); //PORTD = PORTD | 0b00000100;
				}
			} else {
				if(myPWMpins[index].pinState){
					PORTB |= _BV(myPWMpins[index].pin - 8); 
				} else {
					PORTB &= ~(_BV(myPWMpins[index].pin - 8));
				}
			}
			//digitalWrite(myPWMpins[index].pin, myPWMpins[index].pinState);
		}
		// reset the micros() tick counter.
		// digitalWrite(13, !digitalRead(13)); // Why?!
//		previousMicros = currentMicros;
//	}
}