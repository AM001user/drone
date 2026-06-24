#include <Arduino.h>
#include "controle_moteur.h"

const uint16_t STEP = 20;          // increment en us par commande UP/DOWN
const uint16_t MIN_THROTTLE = 1000;
const uint16_t MAX_THROTTLE = 2000;

uint16_t motors[8] = {1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000};

bool motorsEnabled = false;

unsigned long lastSendTime = 0;
const unsigned long sendInterval = 20; // 50Hz, au-dessus du minimum 10Hz requis par INAV

void sendMSP2(uint8_t code, uint8_t* payload, uint8_t size) {
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

void modifval(){
  uint8_t payload[16];
  for (int i = 0; i < 8; i++) {
    payload[i * 2]     = motors[i] & 0xFF;
    payload[i * 2 + 1] = (motors[i] >> 8) & 0xFF;
  }
  sendMSP2(214, payload, sizeof(payload));}

void printAll() {
  Serial.print("Motors: ");
  for (int i = 0; i < 4; i ++) {
    Serial.print(motors[i]);
    if (i < 3) Serial.print(", ");
  }
  Serial.println();
}

void processCommand2(String cmd) {
  cmd.trim();

  if (cmd == "x" && !motorsEnabled) {        // ARM
    motorsEnabled = true;
    Serial.println("STATUS:ARMED");

  } else if (cmd == "x" && motorsEnabled) {  // DISARM
    motorsEnabled = false;
    for (int i = 0; i < 4; i++) motors[i] = MIN_THROTTLE;
    Serial.println("STATUS:DISARMED");

  } else if (cmd == "a" && motorsEnabled) {  // UP tous
    for (int i = 0; i < 4; i++)
      motors[i] = constrain(motors[i] + STEP, MIN_THROTTLE, MAX_THROTTLE);
    printAll();

  } else if (cmd == "e" && motorsEnabled) {  // DOWN tous
    for (int i = 0; i < 4; i++)
      motors[i] = constrain(motors[i] - STEP, MIN_THROTTLE, MAX_THROTTLE);
    printAll();

  } else if (cmd == "d" && motorsEnabled) {  // RIGHT
    motors[0] = constrain(motors[0] - STEP, MIN_THROTTLE, MAX_THROTTLE);
    motors[1] = constrain(motors[1] - STEP, MIN_THROTTLE, MAX_THROTTLE);
    motors[2] = constrain(motors[2] + STEP, MIN_THROTTLE, MAX_THROTTLE);
    motors[3] = constrain(motors[3] + STEP, MIN_THROTTLE, MAX_THROTTLE);
    printAll();

  } else if (cmd == "q" && motorsEnabled) {  // LEFT
    motors[0] = constrain(motors[0] + STEP, MIN_THROTTLE, MAX_THROTTLE);
    motors[1] = constrain(motors[1] + STEP, MIN_THROTTLE, MAX_THROTTLE);
    motors[2] = constrain(motors[2] - STEP, MIN_THROTTLE, MAX_THROTTLE);
    motors[3] = constrain(motors[3] - STEP, MIN_THROTTLE, MAX_THROTTLE);
    printAll();

   } else if (cmd == "z" && motorsEnabled) {  // front
    motors[0] = constrain(motors[0] + STEP, MIN_THROTTLE, MAX_THROTTLE);
    motors[1] = constrain(motors[1] - STEP, MIN_THROTTLE, MAX_THROTTLE);
    motors[2] = constrain(motors[2] + STEP, MIN_THROTTLE, MAX_THROTTLE);
    motors[3] = constrain(motors[3] - STEP, MIN_THROTTLE, MAX_THROTTLE);
    printAll();

   } else if (cmd == "s" && motorsEnabled) {  // back
    motors[0] = constrain(motors[0] - STEP, MIN_THROTTLE, MAX_THROTTLE);
    motors[1] = constrain(motors[1] + STEP, MIN_THROTTLE, MAX_THROTTLE);
    motors[2] = constrain(motors[2] - STEP, MIN_THROTTLE, MAX_THROTTLE);
    motors[3] = constrain(motors[3] + STEP, MIN_THROTTLE, MAX_THROTTLE);
    printAll();

  } else if (cmd == "x") {                   // STOP → 1000 (toujours actif)
    for (int i = 0; i < 8; i++) motors[i] = MIN_THROTTLE;
    Serial.println("STATUS:DOWN TO 1000");
  }
}