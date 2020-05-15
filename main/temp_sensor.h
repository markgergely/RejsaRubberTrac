#include "MLX90621.h"
#include "MLX90640.h"
#include "Configuration.h"
#include <Arduino.h>
#include <Wire.h>
#include "config.h"

#ifndef TEMPSENSOR_H
#define TEMPSENSOR_H
class TempSensor {
private:
  float calculateColumnTemperature(uint16_t column_content[], uint8_t size);
  void interpolate(uint8_t startColumn, uint8_t endColumn, int16_t result[]);
  void calculateSlope(float result[]);
  void getMinMaxSlopePosition();
  boolean checkAutorangeValidity();
  MLX90621 FISDevice90621;
  MLX90640 FISDevice90640;
  TwoWire *thisWire;
public:
  int16_t   measurement_32[32];
  int16_t   picture[128];
  uint16_t  pictureOffset;
  float     measurement[32];
  float     measurement_slope[32];
  boolean   validAutorangeFrame;
  int16_t   measurement_16[16];
  uint8_t   rawMinSlopePosition;
  uint8_t   rawMaxSlopePosition;
  float     leftEdgePosition;
  float     rightEdgePosition;
  fis_t*    config;
  status_t* status;
  uint8_t   effective_rows;
	void  initialise(fis_t* _config, status_t* _status, TwoWire *I2Cpipe = &Wire);
  float getPixelTemperature(uint8_t x, uint8_t y);
	void  measure();
};
#endif