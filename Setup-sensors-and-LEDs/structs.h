typedef struct {
	uint8_t pin;
	uint16_t baseValue;
} Sensor;


typedef struct {
	Sensor sensor;
	uint8_t ledPin;
} Drawer;
