#define ON true
#define OFF false
#include "Arduino.h"

typedef struct {
	uint8_t pin;
	uint16_t pwmValue;
	uint16_t finalPwmValue;
	uint32_t doneTime;
	bool pinState;
	uint16_t pwmTickCount;
} pwmPin;

void setupPWMpins();
void updatePwmValues();
void pwmFadePattern();
void handlePWM();
void setPwmLevelForDrawer(uint8_t drawer, uint8_t level);
void setPwmLevelForDrawer(uint8_t drawer, uint8_t level, uint32_t doneTime);