#include "Adafruit_VL53L0X.h"

int16_t distanceFilter(int16_t distanceIn);

class DistSensor {
private:
  boolean present;
  Adafruit_VL53L0X sensor;
public:
  int16_t distance;
  DistSensor();
  void initialise();
  void measure(void);
};
