#include "VL53L0X-2xI2C.h"
#include <Wire.h>
#ifndef DISTSENSOR_H
#define DISTSENSOR_H

class DistSensor {
private:
  boolean present;
  VL53L0X sensor;
  TwoWire *thisWire;
public:
  int16_t distance;
  bool initialise(TwoWire *I2Cpipe = &Wire);
  void measure(void);
};
#endif