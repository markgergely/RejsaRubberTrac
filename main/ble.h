#include <bluefruit.h>
#include <Arduino.h>
#include "protocol.h"
#include "Configuration.h"

class BLEDevice {
private:
  BLEService        mainService;
  BLECharacteristic GATTone;
  BLECharacteristic GATTtwo;
  BLECharacteristic GATTthr;
  one_t             datapackOne;
  two_t             datapackTwo;
  thr_t             datapackThr;
  void setupMainService(void);
  void startAdvertising(void);
  void renderPacketTemperature(int16_t measurements[], uint8_t mirrorTire, one_t &FirstPacket, two_t &SecondPacket, thr_t &ThirdPacket);
  void renderPacketBattery(int vbattery, int percentage, two_t &SecondPacket);
  void sendPackets(one_t &FirstPacket, two_t &SecondPacket, thr_t &ThirdPacket);
  void parseBLEname(uint8_t wheelPos, char *blename);
public:
  void setupDevice(char bleName[], uint8_t wheelPosCode);
  void transmit(int16_t tempMeasurements[], uint8_t mirrorTire, int16_t distance, int vBattery, int lipoPercentage);
  BLEDevice();
};
