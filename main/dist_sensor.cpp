#include <Arduino.h>
#include "dist_sensor.h"
#include "Configuration.h"
#include "algo.h"

void DistSensor::initialise() {
  distance = 0;
  pinMode(GPIODISTSENSORXSHUT, OUTPUT);
  distance_sensor = Adafruit_VL53L0X();
  digitalWrite(GPIODISTSENSORXSHUT, LOW);
  delay(200);
  digitalWrite(GPIODISTSENSORXSHUT, HIGH);
  delay(200);
  present = sensor.begin(VL53L0X_I2C_ADDR);
};

void DistSensor::measure() {
  if (present) {
    VL53L0X_RangingMeasurementData_t measure;
    if (sensor.rangingTest(&measure, false) != VL53L0X_ERROR_NONE) {
      distance = 0;                                        // SENSOR FAIL
      debug("Reset distance sensor");
      initialise();
    }
    else {
      int16_t rawDistance = measure.RangeMilliMeter;
      if (measure.RangeStatus == 4 || rawDistance > 8190) {   // MEASURE FAIL
        distance = 0;
      }
      else {
        distance = distanceFilter(rawDistance) - DISTANCEOFFSET;
      }
    }
  }
}
