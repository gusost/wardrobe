typedef struct {
	uint8_t pin;
	uint16_t baseValue;
} Sensor;


typedef struct {
	Sensor sensor;
	uint8_t ledPin; // 4,7,8 is SoftPWM
} Drawer;
 