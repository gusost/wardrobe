/*
 Fade

 This example shows how to fade an LED on pin 9
 using the analogWrite() function.

 The analogWrite() function uses PWM, so if
 you want to change the pin you're using, be
 sure to use another PWM capable pin. On most
 Arduino, the PWM pins are identified with
 a "~" sign, like ~3, ~5, ~6, ~9, ~10 and ~11.

 This example code is in the public domain.
 */

int led = 3;           // the PWM pin the LED is attached to
int brightness = 0;    // how bright the LED is
int fadeAmount = 1;    // how many points to fade the LED by

const unsigned int maxStepsConst = 1000;
const unsigned int maxLevelConst = 255;
const float gammaConst = 2.5;

// the setup routine runs once when you press reset:
void setup() {
	// declare pin 9 to be an output:
	pinMode(led, OUTPUT);
	Serial.begin(115200);
	exponentialCurve();
}

// the loop routine runs over and over again forever:
void loop() {
	// set the brightness of pin 9:
	analogWrite( led, getLevel(brightness) );

	// change the brightness for next time through the loop:
	brightness = brightness + fadeAmount;

	// reverse the direction of the fading at the ends of the fade:
	if (brightness <= 0 || brightness >= 1000) {
		fadeAmount = -fadeAmount;
		delay(1000);
	}
	// wait for 30 milliseconds to see the dimming effect
	//delay(30);
}
unsigned int getLevel(unsigned int level) {
	return calculateLevel(level, maxStepsConst, maxLevelConst, gammaConst);
}

void exponentialCurve() {
	exponentialCurve(3.0);
}

void exponentialCurve(float gamma) {
	exponentialCurve(gamma, 1000, 255);
}

void exponentialCurve(float gamma, unsigned int steps, unsigned int levels) {
	Serial.print("Exponential curve with ");
	Serial.print(steps);
	Serial.print(" steps and ");
	Serial.print(levels);
	Serial.print(" levels with a gamma of ");
	Serial.println(gamma);
	for(uint8_t i = 69; i>0; i--) Serial.print("=");
	Serial.println();
	Serial.print("[");
	for (unsigned int x = 0; x < steps; x++) {
		Serial.print( calculateLevel(x, steps, levels, gamma), DEC);
		Serial.print(",");
	}
	Serial.println("]");
	
	int illustrationRows = 41;
	int illustrationColumns = 178;
	for (unsigned int x = 0; x < illustrationRows; x++) {
		illustrateLevel(x, illustrationRows, illustrationColumns, gamma);
	}
/*
	int y;
	for (unsigned int x = 0; x < steps; x++) {
		y = calculateLevel(x, steps, levels, gamma);
		printLevel(x,y);
	}
*/
}

unsigned int calculateLevel(unsigned int level, unsigned int maxSteps, unsigned int maxLevel, float gamma) {
	float normx = level / (float) (maxSteps - 1);
	float normy = pow(normx, gamma);
	return (unsigned int) (0.5 + normy * maxLevel);
}

void illustrateLevel(unsigned int level, unsigned int illustrationRows, unsigned int illustrationColumns, float gamma) {
	for(unsigned int i = calculateLevel(level, illustrationRows, illustrationColumns, gamma); i>0; i--) { 
		Serial.print("#");
	}
	Serial.println();
}

void printLevel(unsigned int x, unsigned int y) {
		Serial.print("x: ");
		Serial.print(x, DEC);
		Serial.print(" y: ");
		Serial.println(y, DEC);
}