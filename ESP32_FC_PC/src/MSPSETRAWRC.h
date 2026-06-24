#pragma once
#include <Arduino.h>

// Canaux RC
#define FC_SERIAL Serial2
extern uint16_t channels[16];
extern bool droneArmed;

// Timing
extern unsigned long lastSendTime;
extern const unsigned long sendInterval;

// Constantes
extern const uint16_t STEP;
extern const uint16_t NEUTRAL;
extern const uint16_t MIN_THROTTLE;
extern const uint16_t MAX_THROTTLE;
extern const uint16_t MIN_RC;
extern const uint16_t MAX_RC;

// Fonctions
void sendMSP(uint8_t code, uint8_t* payload, uint8_t size);
void sendRawRC();
void printChannels();
void processCommandRC(String cmd);

void requestMSPStatus();
void readMSPResponse();