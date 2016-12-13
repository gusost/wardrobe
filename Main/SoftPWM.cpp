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
}

void setupPWMpins() {
	for (int index=0; index < pinCount; index++) {
		myPWMpins[index].pin = pins[index];
 
		// mix it up a little bit
		// changes the starting pwmValue for odd and even
		if (index % 2)
			myPWMpins[index].pwmValue = 25;
		else
			myPWMpins[index].pwmValue = 75;
 
		myPWMpins[index].pinState = ON;
		myPWMpins[index].pwmTickCount = 0;
 
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
	currentMicros = micros();
	// check to see if we need to increment our PWM counters yet
	if (currentMicros - previousMicros >= microInterval) {
		// Increment each pin's counter
		for (int index=0; index < pinCount; index++) {
			// each pin has its own tickCounter
			myPWMpins[index].pwmTickCount++;
			// determine if we're counting on or off time
			if(myPWMpins[index].pwmValue == pwmMax) {
				myPWMpins[index].pinState = ON;
			}
			else if(myPWMpins[index].pwmValue == 0) {
				myPWMpins[index].pinState = OFF;
			}
			else if (myPWMpins[index].pinState == ON) {
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
			digitalWrite(myPWMpins[index].pin, myPWMpins[index].pinState);
		}
		// reset the micros() tick counter.
		// digitalWrite(13, !digitalRead(13)); // Why?!
		previousMicros = currentMicros;
	}
}