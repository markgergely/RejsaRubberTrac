#include <Arduino.h>
#include <Wire.h>
#include <Tasker.h>
#include "Configuration.h"
#include "temp_sensor.h"
#include "dist_sensor.h"
#include "display.h"
#include "ble.h"
#include "algo.h"
  
TempSensor tempSensor;
DistSensor distSensor;
BLDevice bleDevice;
Display display;
Tasker tasker;

uint8_t mirrorTire = 0;
int vBattery = 0;          // Current battery voltage in mV
int lipoPercentage = 0;    // Current battery percentage
int measurementCycles = 0; // Counts how many measurement cycles were completed. printStatus() uses it to roughly calculate refresh rate.
float updateRate = 0.0;    // Reflects the actual update rate
// Function declarations
uint8_t getWheelPosCoding(void);
void printStatus(void);
void blinkOnTempChange(int16_t);
void blinkOnDistChange(uint16_t);
int getVbat(void);
void updateBattery(void);
void updateRefreshRate(void);

#ifdef DUMMYDATA
  #include "dummydata.h"
#endif  


// ----------------------------------------

void setup(){
  static char bleName[32] = "RejsaRubber";
  Serial.begin(115200);
  while (!Serial); // Wait for Serial
  Serial.printf("\nBegin startup. Arduino version: %d\n",ARDUINO);

#if BOARD == BOARD_NRF52
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
#elif BOARD == BOARD_ESP32
  analogReadResolution(12); //12 bits
  analogSetAttenuation(ADC_11db);  //For all pins
#endif

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
  tasker.setInterval(updateRefreshRate, 2000);

#if DISP_DEVICE != DIST_NONE
  tasker.setInterval(updateDisplay,200);
#endif

#if DIST_SENSOR != DIST_NONE
  Serial.println("Starting distance sensor");
  distSensor.initialise();
#endif

#if DISP_DEVICE != DISP_NONE
  display.setup();
#endif

  Serial.println("Starting temperature sensor");
  tempSensor.initialise(FIS_REFRESHRATE);

  Serial.println("Starting BLE device");
  bleDevice.setupDevice(bleName, wheelPosCode);

  Serial.println("Running!");
}

void loop() {
#if DIST_SENSOR != DIST_NONE
  distSensor.measure();
#endif
  tempSensor.measure();
  if (bleDevice.isConnected()) {
    bleDevice.transmit(tempSensor.measurement_16, mirrorTire, distSensor.distance, vBattery, lipoPercentage);
  }

#if DISP_DEVICE == DISP_NONE && BOARD == BOARD_NRF52 // Only use the LEDs for nRF52 w/o display
  blinkOnTempChange(tempSensor.measurement_16[8]/20);    // Use one single temp in the middle of the array
  blinkOnDistChange(distSensor.distance/20);    // value/nn -> Ignore smaller changes to prevent noise triggering blinks
#endif

  measurementCycles++;
  tasker.loop();
}

void updateDisplay(void) {
  display.refreshDisplay(tempSensor.measurement, tempSensor.leftEdgePosition, tempSensor.rightEdgePosition, tempSensor.validAutorangeFrame, updateRate, distSensor.distance, lipoPercentage, bleDevice.isConnected());
}

uint8_t getWheelPosCoding(void) {
  return digitalRead(GPIOLEFT) + (digitalRead(GPIOFRONT) << 1) + (digitalRead(GPIOCAR) << 2);
} 

void printStatus(void) {
  debug("Rate: %.1fHz\tV: %dmV (%d%%)\tD: %dmm\tT: ", (float)updateRate, vBattery, lipoPercentage, distSensor.distance);
  for (uint8_t i=0; i<FIS_X; i++) {
    debug("%.1f\t",(float)tempSensor.measurement[i]/10);
  }
  debug("\n");
}

void updateRefreshRate(void) {
  static long lastUpdate = 0;
  updateRate = (float)measurementCycles / (millis()-lastUpdate) * 1000;
  lastUpdate = millis();
  measurementCycles = 0;
}

#if DISP_DEVICE == DISP_NONE && BOARD == BOARD_NRF52
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
#endif

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

void updateBattery(void) {
  vBattery = getVbat();
  lipoPercentage = lipoPercent(vBattery);
}
