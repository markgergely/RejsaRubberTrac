#include "Configuration.h"
#if BOARD == BOARD_NRF52
  #include <bluefruit.h>
#elif BOARD == BOARD_ESP32
  #include <BLEDevice.h>
  #include <BLEServer.h>
  #include <BLEUtils.h>
  #include <BLE2902.h>
  #include <esp_bt_device.h>
#endif
#include <Arduino.h>
#include "config.h"
#include "protocol.h"
#include "temp_sensor.h"
#include "dist_sensor.h"

class BLDevice {
private:
#if BOARD == BOARD_NRF52
  BLEService         mainService;
  BLECharacteristic  GATTone;
  BLECharacteristic  GATTtwo;
  BLECharacteristic  GATTthr;
#elif BOARD == BOARD_ESP32
  BLEService*        mainService;
  BLEServer*         mainServer;
  BLEAdvertising*    mainAdvertising;
  BLECharacteristic* GATTone;
  BLECharacteristic* GATTtwo;
  BLECharacteristic* GATTthr;
  BLECharacteristic* GATTfour;
  BLECharacteristic* GATTfive;
  BLECharacteristic* GATTconfig;
#endif
  one_t             datapackOne;
  two_t             datapackTwo;
  thr_t             datapackThr;
  preview_t            datapackFour;
  five_t            datapackFive;
  config_t          datapackConfig;
  void setupMainService(void);
  void startAdvertising(void);
  void renderPacketTemperature(int16_t measurements[], one_t &FirstPacket, two_t &SecondPacket, thr_t &ThirdPacket);
  void renderPacketTemperature32(int16_t measurements[], preview_t &Packet, status_t* status);
  void renderPacketPicture(int16_t picture[], uint16_t pictureOffset, five_t &Packet);
  void renderPacketBattery(int vbattery, int percentage, two_t &SecondPacket);
  void sendPackets(one_t &FirstPacket, two_t &SecondPacket, thr_t &ThirdPacket);
public:
  boolean isConnected();
  boolean isConfigReceived(void);
  void setupDevice(config_t config, status_t status);
  void transmit(TempSensor* tempsensor, DistSensor* distsensor, status_t* status, config_t config);
  void writeConfig(config_t config);
  void setDeviceName(status_t status);
  config_t * getBleConfig();
  BLDevice();
};
