#include <Arduino.h>
#include "MSPSETRAWRC.h"

// ================================================================
// MSP_SET_RAW_RC (code 200)
// Canaux INAV standard : Roll, Pitch, Throttle, Yaw, AUX1...
// ================================================================

const uint16_t STEP        = 20;
const uint16_t NEUTRAL      = 1500;
const uint16_t MIN_THROTTLE = 1000;
const uint16_t MAX_THROTTLE = 2000;
const uint16_t MIN_RC       = 1000;
const uint16_t MAX_RC       = 2000;

// Canaux RC : [0]Roll [1]Pitch [2]Throttle [3]Yaw [4]AUX1(ARM) [5-15]AUX...
uint16_t channels[16] = {
  1500, // Roll     (neutre)
  1500, // Pitch    (neutre)
  1000, // Throttle (bas)
  1500, // Yaw      (neutre)
  1000, // AUX1     (DISARM)
  1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000
};

bool droneArmed = false;

unsigned long lastSendTime = 0;
const unsigned long sendInterval = 20; // 50Hz

void sendMSP(uint8_t code, uint8_t* payload, uint8_t size) {
  uint8_t checksum = size ^ code;
  FC_SERIAL.write('$');
  FC_SERIAL.write('M');
  FC_SERIAL.write('<');
  FC_SERIAL.write(size);
  FC_SERIAL.write(code);
  for (uint8_t i = 0; i < size; i++) {
    FC_SERIAL.write(payload[i]);
    checksum ^= payload[i];
  }
  FC_SERIAL.write(checksum);
}

void sendRawRC() {
  uint8_t payload[32]; // 16 canaux x 2 bytes
  for (int i = 0; i < 16; i++) {
    payload[i * 2]     = channels[i] & 0xFF;
    payload[i * 2 + 1] = (channels[i] >> 8) & 0xFF;
  }
  sendMSP(200, payload, sizeof(payload));
}

void printChannels() {
  Serial.print("R:");   Serial.print(channels[0]);
  Serial.print(" P:");  Serial.print(channels[1]);
  Serial.print(" T:");  Serial.print(channels[2]);
  Serial.print(" Y:");  Serial.print(channels[3]);
  Serial.print(" ARM:"); Serial.println(droneArmed ? "YES" : "NO");
}

void processCommandRC(String cmd) {
  cmd.trim();

  // --- ARM / DISARM (touche x) ---
  if (cmd == "x" && !droneArmed) {  // ARM
    channels[2] = 1000;  // ← force throttle bas AVANT d'armer
    sendRawRC();          // ← envoie immédiatement ce throttle=1000
    delay(100);           // ← laisse le FC voir throttle=1000
    droneArmed = true;
    channels[4] = 1500;
    Serial.println("STATUS:ARMED");

  } else if (cmd == "x" && droneArmed) {  // DISARM
    droneArmed = false;
    channels[4] = 1000;  // hors range → DISARM
    channels[2] = 1000;
    channels[0] = NEUTRAL;
    channels[1] = NEUTRAL;
    channels[3] = NEUTRAL;
    Serial.println("STATUS:DISARMED");

  // --- THROTTLE ---
  } else if (cmd == "a" && droneArmed) {  // UP
    channels[2] = constrain(channels[2] + STEP, MIN_THROTTLE, MAX_THROTTLE);
    printChannels();

  } else if (cmd == "e" && droneArmed) {  // DOWN
    channels[2] = constrain(channels[2] - STEP, MIN_THROTTLE, MAX_THROTTLE);
    printChannels();

  // --- PITCH (avant/arriere) ---
  } else if (cmd == "z" && droneArmed) {  // FRONT
    channels[1] = constrain(channels[1] - STEP, MIN_RC, MAX_RC); // Pitch- = avant
    printChannels();

  } else if (cmd == "s" && droneArmed) {  // BACK
    channels[1] = constrain(channels[1] + STEP, MIN_RC, MAX_RC); // Pitch+ = arriere
    printChannels();

  // --- ROLL (gauche/droite) ---
  } else if (cmd == "d" && droneArmed) {  // RIGHT
    channels[0] = constrain(channels[0] + STEP, MIN_RC, MAX_RC); // Roll+ = droite
    printChannels();

  } else if (cmd == "q" && droneArmed) {  // LEFT
    channels[0] = constrain(channels[0] - STEP, MIN_RC, MAX_RC); // Roll- = gauche
    printChannels();

  // --- RECENTRAGE (relache touche) ---
  } else if (cmd == "r") {  // RESET Roll/Pitch/Yaw au neutre
    channels[0] = NEUTRAL;
    channels[1] = NEUTRAL;
    channels[3] = NEUTRAL;
    Serial.println("STATUS:CENTERED");
  }
}

void requestMSPStatus() {
  // Envoi requete MSP_STATUS (pas de payload)
  uint8_t payload[0];
  sendMSP(101, payload, 0);
}

void readMSPResponse() {
  unsigned long timeout = millis() + 100;
  while (millis() < timeout) {
    if (FC_SERIAL.available() >= 6) {
      uint8_t c = FC_SERIAL.read();
      if (c != '$') continue;
      if (FC_SERIAL.read() != 'M') continue;
      if (FC_SERIAL.read() != '>') continue;

      uint8_t size = FC_SERIAL.read();
      uint8_t code = FC_SERIAL.read();

      Serial.print("CODE:"); Serial.print(code);
      Serial.print(" SIZE:"); Serial.println(size);

      if (size > 0 && size < 64) {
    uint8_t data[64];
    for (int i = 0; i < size; i++) data[i] = FC_SERIAL.read();

    // Affiche tous les bytes en hex
    Serial.print("DATA: ");
    for (int i = 0; i < size; i++) {
      Serial.print("b"); Serial.print(i);
      Serial.print("=0x"); Serial.print(data[i], HEX);
      Serial.print(" ");
    }
    Serial.println();

    // AJOUTE ICI
    if (code == 101 && size >= 6) {
      uint16_t flags = data[4] | (data[5] << 8);
      bool fcArmed = (flags & 0x01);
      Serial.print("FC_ARMED:"); Serial.println(fcArmed ? "YES" : "NO");
      Serial.print("FLAGS:0x"); Serial.println(flags, HEX);
      }
      break;
    }
  }
}
}