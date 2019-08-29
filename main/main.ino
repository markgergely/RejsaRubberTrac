#include <Arduino.h>
#include <Wire.h>
#include "Configuration.h"
#include "temp_sensor.h"
#include "dist_sensor.h"
#include "ble.h"
#include "algo.h"

one_t datapackOne;
two_t datapackTwo;
thr_t datapackThr;

char bleName[32];
uint8_t mirrorTire = 0;

TempSensor tempSensor;
DistSensor distSensor;
int vBattery = 0;
int lipoPercentage = 0;
unsigned long previousBatteryUpdate = 0;

// Function declarations
uint8_t getWheelPosCoding(void);
void printStatus(void);
void blinkOnTempChange(int16_t);
void blinkOnDistChange(uint16_t);
int getVbat(void);


#ifdef DUMMYDATA
  #include "dummydata.h"
#endif  


// ----------------------------------------

void setup(){
  Serial.begin(115200);
  while (!Serial); // Wait for Serial
  Serial.println("\nBegin startup");
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(GPIOLEFT, INPUT_PULLUP);
  pinMode(GPIOFRONT, INPUT_PULLUP);
  pinMode(GPIOCAR, INPUT_PULLUP);
  pinMode(GPIOMIRR, INPUT_PULLUP);

  // TIRE MIRRORED?
  if ((MIRRORTIRE == 1 || digitalRead(GPIOMIRR) == 0)) {
    mirrorTire = 1;
    Serial.println("Temperature sensor orientation is mirrored");
  }

  Serial.println("Starting distance sensor");
  distSensor.initialise();

  Serial.println("Starting temperature sensor");
  tempSensor.initialise(4);

  // SET SOME DEFAULT VALUES IN THE DATA PACKETS
  datapackOne.distance = 0;
  datapackOne.protocol = PROTOCOL;
  datapackTwo.protocol = PROTOCOL;
  datapackThr.distance = 0;
  datapackThr.protocol = PROTOCOL;

  // START UP BLUETOOTH
  uint8_t wheelPosCode = getWheelPosCoding();
  if (wheelPosCode >= 7) wheelPosCode = WHEELPOS; // set from configuration
  parseBLEname(wheelPosCode, bleName);  // Pre-fill the correct bleName for the wheel position

  setupBluefruit(bleName);
  
  Serial.println("Running!");
}

void loop() {
  distSensor.measure();
  tempSensor.measure();
  if (Bluefruit.connected()) {
    renderPacketTemperature(tempSensor.measurement_16, mirrorTire, datapackOne, datapackTwo, datapackThr);
    renderPacketBattery(vBattery, lipoPercentage, datapackTwo);
    datapackOne.distance = datapackThr.distance = distSensor.distance;
    sendPackets(datapackOne, datapackTwo, datapackThr);
  }

  blinkOnTempChange(tempSensor.measurement_16[8]/20);    // Use one single temp in the middle of the array
  blinkOnDistChange(distSensor.distance/20);    // value/nn -> Ignore smaller changes to prevent noise triggering blinks
  long now = millis();
  if ((now-previousBatteryUpdate) > 60000) {
    vBattery = 0;
    previousBatteryUpdate = now;
  }
  if (!vBattery) {
    vBattery = getVbat();
    lipoPercentage = lipoPercent(vBattery);
  }

  printStatus();
}

uint8_t getWheelPosCoding(void) {
  return digitalRead(GPIOLEFT) + (digitalRead(GPIOFRONT) << 1) + (digitalRead(GPIOCAR) << 2);
} 

void printStatus(void) {
  static unsigned long then;
  unsigned long now = millis();
  debug("Rate: %.1fHz\tV: %dmV (%d%%)\tD: %dmm\tT: ",(float)1000/(now-then), vBattery, lipoPercentage, distSensor.distance);
  for (uint8_t i=0; i<FIS_X; i++) {
    debug("%.1f\t",(float)tempSensor.measurement_16[i]/10);
  }
  debug("\n");
  then = now;
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

int getVbat() {
  return analogRead(VBAT_PIN) * MILLIVOLTFULLSCALE * BATRESISTORCOMP / STEPSFULLSCALE;
}
