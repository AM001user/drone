#include <Arduino.h>

#pragma once

#define FC_SERIAL Serial2
#define RX_PIN 16
#define TX_PIN 17
#define FC_BAUD 115200


extern const uint16_t STEP;          // increment en us par commande UP/DOWN
extern const uint16_t MIN_THROTTLE;
extern const uint16_t MAX_THROTTLE;

extern uint16_t motor1Value;
extern bool motorsEnabled;

extern unsigned long lastSendTime;
extern const unsigned long sendInterval; // 50Hz, au-dessus du minimum 10Hz requis par INAV

void sendMSP(uint8_t code, uint8_t* payload, uint8_t size);
void sendMotorCommand();
void processCommand(String cmd);