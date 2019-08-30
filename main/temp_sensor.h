#include "MLX90621.h"
#include "MLX90640.h"
#include "Configuration.h"
#include <Arduino.h>

class TempSensor {
private:
  float getPixelTemperature(uint8_t x, uint8_t y);
  float calculateColumnTemperature(uint16_t column_content[], uint8_t size);
  void extrapolate(uint8_t startColumn, uint8_t endColumn, int16_t result[]);
#if FIS_SENSOR == FIS_MLX90621
  MLX90621 FISDevice;
#elif FIS_SENSOR == FIS_MLX90640
  MLX90640 FISDevice;
#endif
public:
  float measurement[FIS_X];
  int16_t measurement_16[16];
	void initialise(int);
	void measure();
};
