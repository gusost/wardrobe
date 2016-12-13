#define ON true
#define OFF false
#include "Arduino.h"

typedef struct {
	uint8_t pin;
	uint16_t pwmValue;
	bool pinState;
	uint16_t pwmTickCount;
} pwmPin;

void setupPWMpins();
void pwmFadePattern();
void handlePWM();
void setPwmLevelForDrawer(uint8_t drawer, uint8_t level);