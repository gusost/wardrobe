typedef struct {
	uint8_t pin;
	uint16_t baseValue;
} Sensor;


typedef struct {
	Sensor sensor;
	uint8_t ledPin;
} Drawer;

// All timing is in microSeconds
 typedef struct {
	uint32_t startTime = 0;
	uint32_t endTime = 0;
	uint8_t startValue = 0;
	uint8_t currentValue = 0;
	uint8_t endValue = 0;
	uint8_t ledPin;
} Lighting;
