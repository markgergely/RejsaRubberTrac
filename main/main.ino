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
TempSensor tempSensor2;
DistSensor distSensor2;

BLDevice bleDevice;
Tasker tasker;
config_t configuration;
status_t status = {0,0,0.0,0};

void updateBleName(void);
void printStatus(void);
int getVbat(void);
void updateBattery(void);
void updateRefreshRate(void);

void setup(){
  Serial.begin(115200);
  while (!Serial); // Wait for Serial
  Serial.printf("\nBegin startup. Arduino version: %d\n",ARDUINO);
  readConfiguration(&configuration);
  if (!configuration.initialized || configuration.config_version != CONFIG_V) {
    Serial.printf("Initialized is %d\nSize is %d\n", configuration.initialized, configuration.config_version);
    initConfiguration(&configuration);
  }
  dumpConfiguration(&configuration);
#if BOARD == BOARD_ESP32 || BOARD == BOARD_LOLIND32
  analogReadResolution(12); //12 bits
  analogSetAttenuation(ADC_11db);  //For all pins
#endif

#if BOARD == BOARD_LOLIND32
  pinMode(GPIOMIRR2, INPUT_PULLUP);
  pinMode(GPIOUNUSEDA2, INPUT);
  pinMode(GPIOUNUSEDA2, INPUT);
#endif

  updateBattery();
  updateBleName();

  if (configuration.sensor_1.enabled) { 
    Wire.begin(GPIOSDA,GPIOSCL); // initialize I2C w/ I2C pins from config
    if (configuration.sensor_1.dist_type) {
      debug("Starting first distance sensor...\n");
      if (distSensor.initialise(&Wire)) {
        debug("First distance sensor present.\n");
      }
      else {
        debug("ERROR: First distance sensor NOT present.\n");
      }
    }
    debug("Starting first temperature sensor...\n");
    tempSensor.initialise(&configuration.sensor_1.fis, &status, &Wire);
  }

  if (configuration.sensor_2.enabled) { 
    Wire1.begin(GPIOSDA2,GPIOSCL2); // initialize I2C w/ I2C pins from config
    if (configuration.sensor_2.dist_type) {
      debug("Starting seconddistance sensor...\n");
      if (distSensor2.initialise(&Wire1)) {
        debug("Second distance sensor present.\n");
      }
      else {
        debug("ERROR: Second distance sensor not present.\n");
      }
    }
    debug("Starting second temperature sensor for...\n");
    tempSensor2.initialise(&configuration.sensor_2.fis, &status, &Wire1);
  }

  debug("Starting BLE device: %s\n", status.bleName);
  bleDevice.setupDevice(configuration, status);

  debug("Running!\n");

// Set up periodic functions
#ifdef _DEBUG
  tasker.setInterval(printStatus, SERIAL_UPDATERATE*1000); // Print status every second
#endif
  tasker.setInterval(updateBattery, BATTERY_UPDATERATE*1000); // Update battery status every minute
  tasker.setInterval(updateRefreshRate, 2000);
}

void loop() {
  if (configuration.sensor_1.enabled) {
    if (configuration.sensor_1.dist_type) {
      distSensor.measure();
    }
    tempSensor.measure();
  }
  if (configuration.sensor_2.enabled) {
    if (configuration.sensor_2.dist_type) {
      distSensor2.measure();
    }
    tempSensor2.measure();
  }

  if (bleDevice.isConnected()) {
    if (bleDevice.isConfigReceived()) {
      Serial.printf("Config received\n");
      memcpy(&configuration, bleDevice.getBleConfig(), sizeof(config_t));
      tempSensor.initialise(&configuration.sensor_1.fis, &status, &Wire);
      updateBleName();
      bleDevice.setDeviceName(status);
      dumpConfiguration(&configuration);
      writeConfiguration(&configuration);
    }
    bleDevice.transmit(&tempSensor, &distSensor, &status, configuration);
  }
  
// 2do: integrate tempSensor2 & distSensor2 into BLE transmission
// 2do: integrate Wheelpost into BLE transmission

  status.measurementCycles++;
  tasker.loop();
}

// Figure out final BleName based on the first sensor's position
// 2do: second sensor pod support
void updateBleName(void) {
  sprintf(status.bleName, "%s%s%s",
          configuration.bleNamePrefix,
          configuration.sensor_1.position.rear ? "R": "F",
          configuration.sensor_1.position.bike ? "": configuration.sensor_1.position.right ? "R" : "L");
}

void printStatus(void) {
#if DIST_SENSOR != DIST_NONE
  String distSensor_str = String(distSensor.distance) + "mm ";
#else
  String distSensor_str = "N/A  ";
#endif
#if DIST_SENSOR2 != DIST_NONE
  String distSensor2_str = String(distSensor2.distance) + "mm ";
#else
  String distSensor2_str = "N/A  ";
#endif
  debug("Rate: %.1fHz\tV: %dmV (%d%%) \tD: %s\tT: ", (float)status.updateRate, status.mv, status.lipoPercentage, distSensor_str);
  for (uint8_t i=0; i<FIS_X; i++) {
    debug("%.1f\t",(float)tempSensor.measurement[i]/10);
  }
  debug("\n");
#if FIS_SENSOR2_PRESENT == 1
  debug("\t\t\t\t\tWheel: %s\tD: %s\tT: ", wheelPos2, distSensor2_str);
  for (uint8_t i=0; i<FIS_X; i++) {
    debug("%.1f\t",(float)tempSensor2.measurement[i]/10);
  }
  debug("\n");
#endif
}

void updateRefreshRate(void) {
  static long lastUpdate = 0;
  status.updateRate = (float)status.measurementCycles / (millis()-lastUpdate) * 1000;
  lastUpdate = millis();
  status.measurementCycles = 0;
}

int getVbat(void) {
  double adcRead=0;
#if BOARD == BOARD_ESP32 // Compensation for ESP32's crappy ADC -> https://bitbucket.org/Blackneron/esp32_adc/src/master/
  const double f1 = 1.7111361460487501e+001;
  const double f2 = 4.2319467860421662e+000;
  const double f3 = -1.9077375643188468e-002;
  const double f4 = 5.4338055402459246e-005;
  const double f5 = -8.7712931081088873e-008;
  const double f6 = 8.7526709101221588e-011;
  const double f7 = -5.6536248553232152e-014;
  const double f8 = 2.4073049082147032e-017;
  const double f9 = -6.7106284580950781e-021;
  const double f10 = 1.1781963823253708e-024;
  const double f11 = -1.1818752813719799e-028;
  const double f12 = 5.1642864552256602e-033;

  const int loops = 5;
  const int loopDelay = 1;

  int counter = 1;
  int inputValue = 0;
  double totalInputValue = 0;
  double averageInputValue = 0;
  for (counter = 1; counter <= loops; counter++) {
    inputValue = analogRead(VBAT_PIN);
    totalInputValue += inputValue;
    delay(loopDelay);
  }
  averageInputValue = totalInputValue / loops;
  adcRead = f1 + f2 * pow(averageInputValue, 1) + f3 * pow(averageInputValue, 2) + f4 * pow(averageInputValue, 3) + f5 * pow(averageInputValue, 4) + f6 * pow(averageInputValue, 5) + f7 * pow(averageInputValue, 6) + f8 * pow(averageInputValue, 7) + f9 * pow(averageInputValue, 8) + f10 * pow(averageInputValue, 9) + f11 * pow(averageInputValue, 10) + f12 * pow(averageInputValue, 11);
#elif BOARD == BOARD_NRF52
  adcRead = analogRead(VBAT_PIN);
#endif
  return adcRead * MILLIVOLTFULLSCALE * BATRESISTORCOMP / STEPSFULLSCALE;
}

void updateBattery() {
  status.mv = getVbat() + configuration.voltage_offset;
  status.lipoPercentage = lipoPercent(status.mv);
}
