#include "ble.h"

boolean didReceiveConfig;
config_t bleConfig;

BLDevice::BLDevice() { // We initialize a couple things in constructor
  datapackOne.distance = 0;
  datapackOne.protocol = PROTOCOL;
  datapackTwo.protocol = PROTOCOL;
  datapackThr.distance = 0;
  datapackThr.protocol = PROTOCOL;
  datapackFour.protocol = PROTOCOL;
  datapackFive.protocol = PROTOCOL;
  datapackFive.offset = 0;
#if BOARD == BOARD_NRF52
  mainService = BLEService(0x00000001000000fd8933990d6f411ff7);
  GATTone = BLECharacteristic(0x01);
  GATTtwo = BLECharacteristic(0x02);
  GATTthr = BLECharacteristic(0x03);
#endif
}

void BLDevice::setupDevice(config_t config, status_t status) {
  uint8_t macaddr[6];

#if BOARD == BOARD_NRF52
  Bluefruit.autoConnLed(false); // DISABLE BLUE BLINK ON CONNECT STATUS
  Bluefruit.begin(); 
  Bluefruit.getAddr(macaddr);
  sprintf(bleName, "%s%02x%02x%02x\0",bleName, macaddr[2], macaddr[1], macaddr[0]); // Extend bleName[] with the last 4 octets of the mac address
  Bluefruit.setName(bleName);
  Serial.print("Starting bluetooth with MAC address ");
//  Serial.printBufferReverse(macaddr, 6, ':');
  Serial.println();
#elif BOARD == BOARD_ESP32
  BLEDevice::init(status.bleName);
  mainServer = BLEDevice::createServer();
  BLEDevice::setPower(ESP_PWR_LVL_P7);
#endif
  Serial.printf("Device name: %s\n", status.bleName);

  setupMainService();
  startAdvertising();
  writeConfig(config);
}

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      BLEDevice::startAdvertising();
    };
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();
      Serial.printf("Received something with a length of %d\n", rxValue.length());
      Serial.printf("Size of config is: %d\n", sizeof(config_t));
      memcpy(&bleConfig, rxValue.c_str(), rxValue.length());
      didReceiveConfig = true;
    }
};

void BLDevice::setupMainService(void) {
#if BOARD == BOARD_NRF52
  mainService.begin();

  GATTone.setProperties(CHR_PROPS_NOTIFY | CHR_PROPS_READ);  // Options: CHR_PROPS_BROADCAST, CHR_PROPS_NOTIFY, CHR_PROPS_INDICATE, CHR_PROPS_READ, CHR_PROPS_WRITE_WO_RESP, CHR_PROPS_WRITE
  GATTone.setPermission(SECMODE_OPEN, SECMODE_NO_ACCESS);
  GATTone.setFixedLen(20);
  GATTone.begin();

  GATTtwo.setProperties(CHR_PROPS_NOTIFY | CHR_PROPS_READ);
  GATTtwo.setPermission(SECMODE_OPEN, SECMODE_NO_ACCESS);
  GATTtwo.setFixedLen(20);
  GATTtwo.begin();

  GATTthr.setProperties(CHR_PROPS_NOTIFY | CHR_PROPS_READ);
  GATTthr.setPermission(SECMODE_OPEN, SECMODE_NO_ACCESS);
  GATTthr.setFixedLen(20);
  GATTthr.begin();
#elif BOARD == BOARD_ESP32
  mainServer->setCallbacks(new MyServerCallbacks());
  mainService = mainServer->createService(BLEUUID((uint16_t)0x1FF7),300);
  GATTone = mainService->createCharacteristic(BLEUUID((uint16_t)0x0001), BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY );
  GATTtwo = mainService->createCharacteristic(BLEUUID((uint16_t)0x0002), BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY );
  GATTthr = mainService->createCharacteristic(BLEUUID((uint16_t)0x0003), BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY );
  GATTfour = mainService->createCharacteristic(BLEUUID((uint16_t)0x0004), BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY );
  GATTfive = mainService->createCharacteristic(BLEUUID((uint16_t)0x0005), BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY );
  GATTconfig = mainService->createCharacteristic(BLEUUID((uint16_t)0x0006), BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_INDICATE);
  GATTone->addDescriptor(new BLE2902());
  GATTtwo->addDescriptor(new BLE2902());
  GATTthr->addDescriptor(new BLE2902());
  GATTfour->addDescriptor(new BLE2902());
  GATTfive->addDescriptor(new BLE2902());
  GATTconfig->addDescriptor(new BLE2902());
  GATTconfig->setCallbacks(new MyCallbacks());
  mainService->start();
#endif
}
boolean BLDevice::isConfigReceived(void) {
  if (didReceiveConfig) {
    didReceiveConfig = false;
    return true;
  }
  return false;
}

void BLDevice::startAdvertising(void) {
#if BOARD == BOARD_NRF52
  Bluefruit.setTxPower(4);
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
  Bluefruit.Advertising.addService(mainService);
  Bluefruit.Advertising.addName();
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244); // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);
  Bluefruit.Advertising.start(0); 
#elif BOARD == BOARD_ESP32
  mainAdvertising = BLEDevice::getAdvertising();
  mainAdvertising->addServiceUUID(BLEUUID((uint16_t)0x1FF7));
  mainAdvertising->setScanResponse(false);
  mainAdvertising->setMinPreferred(0x0);
  BLEDevice::startAdvertising();
#endif
}

void BLDevice::renderPacketTemperature(int16_t measurements[], one_t &FirstPacket, two_t &SecondPacket, thr_t &ThirdPacket) {
  for(uint8_t x=0;x<8;x++){
    FirstPacket.temps[x]=measurements[x*2];
    SecondPacket.temps[x]=measurements[x*2 + 1];
    ThirdPacket.temps[x]=max(FirstPacket.temps[x], SecondPacket.temps[x]);
  }
}

void BLDevice::renderPacketTemperature32(int16_t measurements[], preview_t &Packet, status_t* status) {
  memcpy(&Packet.temps, &measurements[0], 64);
  memcpy(&Packet.status, status, sizeof(status_t));
}

void BLDevice::renderPacketPicture(int16_t picture[], uint16_t offset, five_t &Packet) {
  memcpy(&Packet.temps, &picture[0], 256);
  Packet.offset = offset;
}

void BLDevice::renderPacketBattery(int vbattery, int percentage, two_t &SecondPacket) {
  SecondPacket.voltage = vbattery;
  SecondPacket.charge = percentage;
}

void BLDevice::transmit(TempSensor* tempsensor, DistSensor* distsensor, status_t* status, config_t config) {
  renderPacketTemperature(tempsensor->measurement_16, datapackOne, datapackTwo, datapackThr);
  renderPacketTemperature32(tempsensor->measurement_32, datapackFour, status);
  renderPacketPicture(tempsensor->picture, tempsensor->pictureOffset, datapackFive);
  renderPacketBattery(status->mv, status->lipoPercentage, datapackTwo);
  datapackOne.distance = datapackThr.distance = distsensor->distance;
#if BOARD == BOARD_NRF52
  GATTone.notify(&datapackOne, sizeof(datapackOne));
  GATTtwo.notify(&datapackTwo, sizeof(datapackTwo));
  GATTthr.notify(&datapackThr, sizeof(datapackThr));
#elif BOARD == BOARD_ESP32
  GATTone->setValue((uint8_t*)&datapackOne, sizeof(datapackOne));
  GATTtwo->setValue((uint8_t*)&datapackTwo, sizeof(datapackTwo));
  GATTthr->setValue((uint8_t*)&datapackThr, sizeof(datapackThr));
  GATTfour->setValue((uint8_t*)&datapackFour, sizeof(datapackFour));
  GATTfive->setValue((uint8_t*)&datapackFive, sizeof(datapackFive));
  GATTone->notify();
  GATTtwo->notify();
  GATTthr->notify();
  GATTfour->notify();
  GATTfive->notify();
#endif 
}

void BLDevice::setDeviceName(status_t status) {
  //BLEDevice::init(status.bleName);
  esp_bt_dev_set_device_name(status.bleName);
}


void BLDevice::writeConfig(config_t config) {
  GATTconfig->setValue((uint8_t*)&config, sizeof(datapackConfig));
}

boolean BLDevice::isConnected() {
#if BOARD == BOARD_NRF52
  return Bluefruit.connected();
#elif BOARD == BOARD_ESP32
  int32_t connectedCount;
  connectedCount = mainServer->getConnectedCount();
  return (connectedCount > 0);
#endif
}

config_t * BLDevice::getBleConfig() {
  return &bleConfig;
}