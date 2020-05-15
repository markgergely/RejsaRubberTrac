#include "temp_sensor.h"
#include "Configuration.h"
#include "algo.h"
#include <Arduino.h>
#include <Wire.h>
#include "spline.h"

void TempSensor::initialise(fis_t* _config, status_t* _status, TwoWire *I2Cpipe) {
  config = _config;
  status = _status;
  rawMinSlopePosition= config->x;
  rawMaxSlopePosition= 0;
  leftEdgePosition = 0.0;
  rightEdgePosition = (float)config->x;
  pictureOffset = 0;
  thisWire = I2Cpipe;
  effective_rows = config->y - config->ignore_bottom - config->ignore_top;
  if ( config->type == FIS_MLX90621 ) {
    FISDevice90621.emissivity = (float)config->emissivity/100;
    FISDevice90621.initialise(config->refresh_rate, thisWire);
  } else {
    FISDevice90640.emissivity = (float)config->emissivity/100;
    FISDevice90640.initialise(config->refresh_rate, thisWire);
  }
};

void TempSensor::measure() {
  uint16_t column_content[effective_rows];
  if ( config->type == FIS_MLX90621 ) {
    FISDevice90621.measure(true);
    status->sensor_stat_1.ambient_t = round(FISDevice90621.getAmbient()*10); //2do: report status to the correct sensor pod
    delay(24);
  } else {
    FISDevice90640.measure(true);
    status->sensor_stat_1.ambient_t = round(FISDevice90640.getAmbient()*10); //2do: report status to the correct sensor pod
  }
  for(uint8_t x=0;x<config->x;x++){
    for (uint8_t y=0;y<effective_rows;y++) { // Read columns first
      column_content[y] = getPixelTemperature(x, y);
    }
    measurement[x]=calculateColumnTemperature(column_content, effective_rows);
    measurement_32[x]=round(calculateColumnTemperature(column_content, effective_rows));
  }
  if (config->type == FIS_MLX90621) {
    for(uint8_t x=0;x<64;x++) picture[x]=round(FISDevice90621.getTemperature(x)*10);
    for(uint8_t x=64;x<128;x++) picture[x]=0;    
  } else {
    for(uint8_t x=0;x<128;x++) picture[x]=round(FISDevice90640.getTemperature(x+pictureOffset)*10);
    if (pictureOffset == 640) {
      pictureOffset = 0;
    } else {
      pictureOffset = pictureOffset + 128;
    }
  }
  if (config->autozoom) {
    calculateSlope(measurement_slope);
    getMinMaxSlopePosition();
    validAutorangeFrame = checkAutorangeValidity();
    status->sensor_stat_1.autozoom_stat.lock = validAutorangeFrame;
    if (validAutorangeFrame) {
      float leftStepSize = abs((rawMaxSlopePosition-leftEdgePosition)/4);
      float rightStepSize = abs((rawMinSlopePosition-rightEdgePosition)/4);
      if (leftEdgePosition < rawMaxSlopePosition) leftEdgePosition += leftStepSize;
      else leftEdgePosition -= leftStepSize;
      if (rightEdgePosition < rawMinSlopePosition) rightEdgePosition += rightStepSize;
      else rightEdgePosition -= rightStepSize;
      if (rightEdgePosition < 0) leftEdgePosition = 0;
      if (leftEdgePosition < 0) rightEdgePosition = 0;
      if (leftEdgePosition > config->x) leftEdgePosition = 32;
      if (rightEdgePosition > config->x) rightEdgePosition = 32;
      status->sensor_stat_1.autozoom_stat.innerEdge = round(rightEdgePosition*10);
      status->sensor_stat_1.autozoom_stat.outerEdge = round(leftEdgePosition*10);
    }
  }
  interpolate((int)leftEdgePosition, (int)rightEdgePosition, measurement_16);
}

float TempSensor::getPixelTemperature(uint8_t x, uint8_t y) {
  if (config->type == FIS_MLX90621) {
    return FISDevice90621.getTemperature((y+config->ignore_top+x*config->y) + config->offset) * 10 * (float)(config->scale/1000); // MLX90621 iterates in columns
  }
  if (config->type == FIS_MLX90640) { 
    return FISDevice90640.getTemperature((y*config->x+config->ignore_top*config->x+x) + config->offset) * 10 * (float)(config->scale/1000); // MLX90640 iterates in rows
  }
  return 0.0;
}

float TempSensor::calculateColumnTemperature(uint16_t column_content[], uint8_t size) {
#if COLUMN_AGGREGATE == COLUMN_AGGREGATE_MAX
  return get_maximum(column_content, size);
#elif COLUMN_AGGREGATE == COLUMN_AGGREGATE_AVG
  return get_average(column_content, size);
#endif
}

void TempSensor::interpolate(uint8_t startColumn, uint8_t endColumn,int16_t result[]) {
  float stepSize = (endColumn-startColumn)/16.0;
  float x[32];
  for (uint8_t i=0;i<config->x;i++) x[i]=i; // Initialize the X axis of an array {0, 1, 2 ... 30, 31}
  Spline linearSpline(x,measurement,config->x,1);
  for (uint8_t i=0;i<16;i++) result[i] = linearSpline.value(startColumn+i*stepSize);
}

void TempSensor::calculateSlope(float result[]) {
  for (uint8_t i=0;i<config->x-1;i++) result[i] = measurement[i+1]-measurement[i];
}

void TempSensor::getMinMaxSlopePosition() {
  float minSlopeValue = 0;
  float maxSlopeValue = 0;
  for (uint8_t i=0;i<config->x-1;i++) {
    if (measurement_slope[i] > maxSlopeValue ) {
      maxSlopeValue = measurement_slope[i];
      rawMaxSlopePosition = i;
    }
    if (measurement_slope[i] < minSlopeValue ) {
      minSlopeValue = measurement_slope[i];
      rawMinSlopePosition = i;
    }
  }
}

boolean TempSensor::checkAutorangeValidity() {
  float avgTireTemp=0;
  float avgAmbientTemp=0;
  if (measurement_slope[rawMinSlopePosition] > -7) return false;
  if (measurement_slope[rawMaxSlopePosition] < 7) return false;
  if (rawMinSlopePosition < rawMaxSlopePosition) return false; // Inner or outer edge of tire out of camera view
  if ((rawMinSlopePosition-rawMaxSlopePosition) < AUTORANGING_MINIMUM_TIRE_WIDTH) return false; // Too thin tire
  for (uint8_t i=0;i<config->x;i++) {
    if (i < rawMaxSlopePosition || i > rawMinSlopePosition ) avgAmbientTemp += measurement[i];
    else avgTireTemp += measurement[i];
  }
  avgTireTemp = avgTireTemp / (rawMinSlopePosition-rawMaxSlopePosition);
  avgAmbientTemp = avgAmbientTemp / (rawMaxSlopePosition + (config->x-rawMinSlopePosition));
  if (avgTireTemp - avgAmbientTemp < 5.0) return false; // Tire is not significantly hotter than ambient
  return true;
}
