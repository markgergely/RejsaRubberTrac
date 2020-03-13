#include <Arduino.h>
#include "dist_sensor.h"
#include "Configuration.h"
#include "algo.h"
#include <Wire.h>

void DistSensor::initialise() {
  distance = 0;
  Wire.begin();
  pinMode(GPIODISTSENSORXSHUT, OUTPUT);
  digitalWrite(GPIODISTSENSORXSHUT, LOW);
  delay(10);
  digitalWrite(GPIODISTSENSORXSHUT, HIGH);
  delay(10);
  uint32_t temporary;
  present = sensor.init();
  sensor.setTimeout(500);
  sensor.setMeasurementTimingBudget(50000);
  temporary = sensor.getMeasurementTimingBudget();
  Serial.printf("wtf: %dmm\n", temporary);
  sensor.startContinuous();
};

void DistSensor::measure() {
  if (present) {
    int16_t rawDistance = sensor.readRangeContinuousMillimeters();
    if (rawDistance < 500) {
       distance = distanceFilter(rawDistance) - DISTANCEOFFSET; // Only update distance if its less than 500mm
    }
    if (sensor.timeoutOccurred()) { Serial.println("Distance sensor TIMEOUT!"); }
  }
}
