#include "temp_sensor.h"
#include "Configuration.h"
#include "algo.h"
#include <Arduino.h>


void TempSensor::initialise(int refrate) {
    FISDevice.initialise(refrate);
};

void TempSensor::measure() {
  uint16_t column_content[EFFECTIVE_ROWS];
  FISDevice.measure(true); 
  for(uint8_t x=0;x<FIS_X;x++){
    for (uint8_t y=0;y<EFFECTIVE_ROWS;y++) { // Read the columns first
#if FIS_SENSOR == FIS_MLX90621
      column_content[y] = (FISDevice.getTemperature(y+IGNORE_TOP_ROWS+x*FIS_Y) + TEMPOFFSET) * 10 * TEMPSCALING; // MLX90621 iterates in columns
#elif FIS_SENSOR == FIS_MLX90640
      column_content[y] = (FISDevice.getTemperature(y*FIS_X+IGNORE_TOP_ROWS*FIS_X+x) + TEMPOFFSET) * 10 * TEMPSCALING; // MLX90640 iterates in rows
#endif
    }
    //Serial.printf("Columns %d %d\n", column_content[1], column_content[2]);
#if COLUMN_AGGREGATE == COLUMN_AGGREGATE_MAX
    measurement[x]=get_maximum(column_content, EFFECTIVE_ROWS);
#elif COLUMN_AGGREGATE == COLUMN_AGGREGATE_AVG
    measurement[x]=get_average(column_content, EFFECTIVE_ROWS);
#endif
  }
#if FIS_SENSOR == FIS_MLX90621
  measurement_16 = measurement;
#elif FIS_SENSOR == FIS_MLX90640
  for (uint8_t i=0;i<16;i++) measurement_16[i] = max(measurement[i*2], measurement[i*2+1]); // Transform the 32 measurements to 16
#endif
}
