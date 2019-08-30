#include <VL53L0X.h>

class DistSensor {
private:
  boolean present;
  VL53L0X sensor;
public:
  int16_t distance;
  void initialise();
  void measure(void);
};
