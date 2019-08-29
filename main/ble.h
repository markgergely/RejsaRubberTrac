#include <bluefruit.h>
#include <Arduino.h>
#include "protocol.h"
#include "Configuration.h"

void setupBluefruit(char bleName[]);
void setupMainService(void);
void startAdvertising(void);
void renderPacketTemperature(int16_t measurements[], uint8_t mirrorTire, one_t &FirstPacket, two_t &SecondPacket, thr_t &ThirdPacket);
void renderPacketBattery(int vbattery, int percentage, two_t &SecondPacket);
void sendPackets(one_t &FirstPacket, two_t &SecondPacket, thr_t &ThirdPacket);
void parseBLEname(uint8_t wheelPos, char *blename);
