#include <Arduino.h>

#pragma once

#define FC_SERIAL Serial2
#define RX_PIN 16
#define TX_PIN 17
#define FC_BAUD 115200


extern uint16_t motors[8];
extern bool motorsEnabled;
extern unsigned long lastSendTime;
extern const unsigned long sendInterval;
extern const uint16_t STEP;
extern const uint16_t MIN_THROTTLE;
extern const uint16_t MAX_THROTTLE;

void sendMSP2(uint8_t code, uint8_t* payload, uint8_t size);
void modifval();
void sendAllMotorCommand();
void processCommand2(String cmd);