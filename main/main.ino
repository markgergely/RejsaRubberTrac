#include <Arduino.h>
#include <Wire.h>
#include <Tasker.h>
#include "Configuration.h"
#include "temp_sensor.h"
#include "dist_sensor.h"
#include "ble.h"
#include "algo.h"
  
TempSensor tempSensor;
DistSensor distSensor;
BLEDevice bleDevice;
Tasker tasker;

uint8_t mirrorTire = 0;
int vBattery = 0;
int lipoPercentage = 0;
int measurementCycles = 0; // Counts how many measurement cycles were completed. printStatus() uses it to roughly calculate refresh rate.

// Function declarations
uint8_t getWheelPosCoding(void);
void printStatus(void);
void blinkOnTempChange(int16_t);
void blinkOnDistChange(uint16_t);
int getVbat(void);
void updateBattery(void);

#ifdef DUMMYDATA
  #include "dummydata.h"
#endif  


// ----------------------------------------

void setup(){
  static char bleName[32] = "RejsaRubber";
  Serial.begin(115200);
  while (!Serial); // Wait for Serial
  Serial.println("\nBegin startup");
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(GPIOLEFT, INPUT_PULLUP);
  pinMode(GPIOFRONT, INPUT_PULLUP);
  pinMode(GPIOCAR, INPUT_PULLUP);
  pinMode(GPIOMIRR, INPUT_PULLUP);
  updateBattery();

  // TIRE MIRRORED?
  if ((MIRRORTIRE == 1 || digitalRead(GPIOMIRR) == 0)) {
    mirrorTire = 1;
    Serial.println("Temperature sensor orientation is mirrored");
  }

  // Figure out wheel position coding.
  uint8_t wheelPosCode = getWheelPosCoding();
  if (wheelPosCode >= 7) wheelPosCode = WHEELPOS; // set from configuration

  // Set up periodic functions
  tasker.setInterval(printStatus, SERIAL_UPDATERATE*1000); // Print status every second
  tasker.setInterval(updateBattery, BATTERY_UPDATERATE*1000); // Update battery status every minute

  Serial.println("Starting distance sensor");
  distSensor.initialise();

  Serial.println("Starting temperature sensor");
  tempSensor.initialise(FIS_REFRESHRATE);

  Serial.println("Starting BLE device");
  bleDevice.setupDevice(bleName, wheelPosCode);
  
  Serial.println("Running!");
}

void loop() {
  distSensor.measure();
  tempSensor.measure();
  if (Bluefruit.connected()) {
    bleDevice.transmit(tempSensor.measurement_16, mirrorTire, distSensor.distance, vBattery, lipoPercentage);
  }
  blinkOnTempChange(tempSensor.measurement_16[8]/20);    // Use one single temp in the middle of the array
  blinkOnDistChange(distSensor.distance/20);    // value/nn -> Ignore smaller changes to prevent noise triggering blinks
  measurementCycles++;
  tasker.loop();
}

uint8_t getWheelPosCoding(void) {
  return digitalRead(GPIOLEFT) + (digitalRead(GPIOFRONT) << 1) + (digitalRead(GPIOCAR) << 2);
} 

void printStatus(void) { 
  debug("Rate: %.1fHz\tV: %dmV (%d%%)\tD: %dmm\tT: ", (float)measurementCycles/SERIAL_UPDATERATE, vBattery, lipoPercentage, distSensor.distance);
  for (uint8_t i=0; i<FIS_X; i++) {
    debug("%.1f\t",(float)tempSensor.measurement[i]/10);
  }
  debug("\n");
  measurementCycles = 0;
}

void blinkOnDistChange(uint16_t distnew) {
  static uint16_t distold = 0;

  if (!Bluefruit.connected()) {
    digitalWrite(LED_RED, HIGH);
    return;
  }
  
  if (distold != distnew) {
    digitalWrite(LED_RED, HIGH);
    delay(3);
    digitalWrite(LED_RED, LOW);
  }
  distold = distnew;
}

void blinkOnTempChange(int16_t tempnew) {
  static int16_t tempold = 0;
  if (tempold != tempnew) {
    digitalWrite(LED_BLUE, HIGH);
    delay(3);
    digitalWrite(LED_BLUE, LOW);
  }
  tempold = tempnew;
}

int getVbat(void) {
  return analogRead(VBAT_PIN) * MILLIVOLTFULLSCALE * BATRESISTORCOMP / STEPSFULLSCALE;
}

void updateBattery(void) {
  vBattery = getVbat();
  lipoPercentage = lipoPercent(vBattery);
}
