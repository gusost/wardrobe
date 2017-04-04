#include <EEPROM.h>
//#include "SoftPWM.h"
#include "structs.h"
#include <U8x8lib.h>
U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);

/*
	Use sensors and LEDs.
*/

// Timing variables
const uint32_t fadeDuration = 589824; //600e3; == 9 << 16
const uint32_t fadeSeparation = fadeDuration / 3;
const uint32_t afterCloseFadeDelay = 5e6; // 5 sec
uint8_t numberOfAnalogValueReads = 64;

// Debug variables
const bool debug = false;
uint32_t pwmUpdateCount = 0;
uint32_t updatePrintTimer = fadeDuration * 2;

Drawer drawers[5];
Lighting lighting[5];
int8_t lowestOpenDrawer = -1;



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
		analogWrite(drawers[i].ledPin, 0);

		Serial.print("Drawer ");
		Serial.print(i + 1);
		Serial.print(" has sensor pin ");
		Serial.print(drawers[i].sensor.pin);
		Serial.print(" with value ");
		Serial.print(drawers[i].sensor.baseValue);
		Serial.print(" has LED pin ");
		Serial.println(drawers[i].ledPin);
	}
	// GG. Debug
	lighting[0].currentValue = 255;
	setLigthing(&lighting[0], micros(), fadeDuration, 0);
//	setupPWMpins();
//	setupInterruptTimer(0,0);
}

void loop() {
	withPwm();
	//noPwm();
	updatePwm();
	if(debug) {
		if ( micros() > updatePrintTimer ) {
			updatePrintTimer += fadeDuration * 2;
			 if(0 != pwmUpdateCount) {
				Serial.print("@ ");
				Serial.print( millis() );
				Serial.print("ms PWM has been updated ");
				Serial.print(pwmUpdateCount);
				Serial.println(" times since last");
				pwmUpdateCount = 0;
			}
		}
	}
}

void updatePwm() {
	//  If no pwm updates are going on do many analog reads
	numberOfAnalogValueReads = 64;
	for (uint8_t i = 0; i < 5; i++)	{
		if (lighting[i].currentValue != lighting[i].endValue) {
			// While PWM updates are going on, only do 2 ADC per drawer.
			numberOfAnalogValueReads = 2;
			
			int32_t fadeTime = lighting[i].endTime - lighting[i].startTime; //  Should be fadeDuration if no overflow
			if(fadeTime > 0) { // Most cases. Except for when there is an overflow of micros()
				uint8_t previousValue = lighting[i].currentValue;
				int32_t timeLeft = lighting[i].endTime - micros();
				if( timeLeft > fadeTime ) {
					// See if everythin is all ready fine before starting the animation
					if(lighting[i].currentValue == lighting[i].startValue) {
						continue;
					}
					lighting[i].currentValue = lighting[i].startValue;
				}
				else if(timeLeft > 0) {
					int16_t valueDiff = (lighting[i].endValue - lighting[i].startValue); // can be positive or negative
					// Progress can easily be made with fixed point if resources are scarse
					float timeProgress = (fadeTime - timeLeft) / (float ) fadeTime; // 1 - timeLeft/fadeTime Between 0.0 and 1.0
					// Enter lighting curve function here.
					// Exponential
					double valueProgress; // Undefined here. Should always be defined later.
					if (valueDiff > 0) {
						// For gamma 2, 3 etc just do multiplications
						//valueProgress = pow(timeProgress, 2.5 /*gamma*/);
						valueProgress = timeProgress * timeProgress;
						lighting[i].currentValue = lighting[i].startValue + valueProgress * valueDiff + 0.5;
					} else {
						//valueProgress = pow(1.0 - timeProgress, 2.5 /*gamma*/);
						valueProgress = 1.0 - timeProgress;
						valueProgress *= valueProgress;
						lighting[i].currentValue = lighting[i].endValue - valueProgress * valueDiff + 0.5;
					}
					// Linear
					//lighting[i].currentValue = lighting[i].startValue + progress * valueDiff;
				} else { // Times up for updating
					lighting[i].currentValue = lighting[i].endValue;
				}
				if(previousValue != lighting[i].currentValue){
					analogWrite(lighting[i].ledPin, lighting[i].currentValue);
					if (debug) {
						pwmUpdateCount++;
					}
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
					if( lighting[j].endValue != 255 ) {
						setLigthing(&lighting[j], startTime, fadeDuration, 255);
						startTime += fadeSeparation;
					}
				}
				lowestOpenDrawer = i;
			}
			// STOP processing after a drawer is found open.
			break;
		} else {
			// GG. if drawer i used to be the lowest one and is now closed reset the lowestOpenDrawer variable
			if (i == lowestOpenDrawer) {
				lowestOpenDrawer = -1;
			}
			if(lighting[i].endValue != 0) {
				setLigthing(&lighting[i], startTime + afterCloseFadeDelay, fadeDuration, 0);
				startTime += fadeSeparation;
			}
		}
	}
}
/* void noPwm() {
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
} */
void setLigthing( Lighting *lighting, uint32_t startTime, uint32_t duration, uint8_t value ){
	lighting->startTime = startTime;
	lighting->endTime = startTime + duration;
	lighting->startValue = lighting->currentValue;
	lighting->endValue = value;
	if(debug) {
		Serial.print("Setting lighting to ");
		Serial.print(value);
		Serial.print(" from ");
		Serial.print(lighting->currentValue);
		Serial.print(" until ");
		Serial.print(lighting->endTime / 1000);
		Serial.println("ms");
	}
}
uint16_t getDrawerSensorValue(Drawer drawer) {
	return getSensorValue(drawer.sensor.pin);
}
uint16_t getSensorValue(uint8_t sensorPin) {
	uint16_t tempValue = 0;
	for(uint8_t j = 0; j < numberOfAnalogValueReads; j++) {
		tempValue += analogRead(sensorPin);
	}
	return tempValue / numberOfAnalogValueReads;
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