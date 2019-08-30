#include "ble.h"

BLEService  mainService   = BLEService        (0x00000001000000fd8933990d6f411ff7);
BLECharacteristic GATTone = BLECharacteristic (0x01);
BLECharacteristic GATTtwo = BLECharacteristic (0x02);
BLECharacteristic GATTthr = BLECharacteristic (0x03);

void setupBluefruit(char bleName[]) {
  uint8_t macaddr[6];
  Bluefruit.autoConnLed(false); // DISABLE BLUE BLINK ON CONNECT STATUS
  Bluefruit.begin(); 
  Bluefruit.getAddr(macaddr);
  sprintf(bleName, "%s%02x%02x%02x\0",bleName, macaddr[2], macaddr[1], macaddr[0]); // Extend bleName[] with the last 4 octets of the mac address
  Serial.print("Starting bluetooth with MAC address ");
  Serial.printBufferReverse(macaddr, 6, ':');
  Serial.println();
  Serial.printf("Device name: %s\n", bleName);
  Bluefruit.setName(bleName);

  setupMainService();
  startAdvertising(); 
}

void setupMainService(void) {
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
}


void startAdvertising(void) {
  
  Bluefruit.setTxPower(4);
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
  Bluefruit.Advertising.addService(mainService);
  Bluefruit.Advertising.addName();
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244); // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);
  Bluefruit.Advertising.start(0); 
}

void parseBLEname(uint8_t wheelPos, char *blename) {
  switch (wheelPos) {
    case 0: sprintf(blename, "RejsaRubberFL"); break;
    case 1: sprintf(blename, "RejsaRubberFR"); break;
    case 2: sprintf(blename, "RejsaRubberRL"); break;
    case 3: sprintf(blename, "RejsaRubberRR"); break;
    case 4: sprintf(blename, "RejsaRubberF "); break;
    case 5: sprintf(blename, "RejsaRubberF "); break;
    case 6: sprintf(blename, "RejsaRubberR "); break;
    case 7: sprintf(blename, "RejsaRubber"); break;
    default: sprintf(blename, "Name Error "); break;
  }
}

void renderPacketTemperature(int16_t measurements[], uint8_t mirrorTire, one_t &FirstPacket, two_t &SecondPacket, thr_t &ThirdPacket) {
  for(uint8_t x=0;x<8;x++){
    uint8_t _x = x;
    if (mirrorTire == 1) {
      _x = 7-x;
    }
    FirstPacket.temps[_x]=measurements[x*2];
    SecondPacket.temps[_x]=measurements[x*2 + 1];
    ThirdPacket.temps[_x]=max(FirstPacket.temps[_x], SecondPacket.temps[_x]);
  }
}

void renderPacketBattery(int vbattery, int percentage, two_t &SecondPacket) {
  SecondPacket.voltage = vbattery;
  SecondPacket.charge = percentage;
}

void sendPackets(one_t &FirstPacket, two_t &SecondPacket, thr_t &ThirdPacket) {
  GATTone.notify(&FirstPacket, sizeof(FirstPacket));
  GATTtwo.notify(&SecondPacket, sizeof(SecondPacket));
  GATTthr.notify(&ThirdPacket, sizeof(ThirdPacket));
}
