#include <Arduino.h>
#include "controle_moteur.h"

/*
  ESP32 -> INAV : Controle Moteur 1 via commandes serie USB (pilote par PC)
  ===========================================================================

  Commandes recues sur le port serie USB (envoyees par le script Python) :
    "ARM"       -> active l'envoi reel au moteur (securite)
    "DISARM"    -> coupe immediatement (force a 1000)
    "UP"        -> augmente motor1 de STEP us
    "DOWN"      -> diminue motor1 de STEP us
    "SET:xxxx"  -> fixe directement motor1 a xxxx us (1000-2000)

  L'ESP32 envoie en continu (50Hz) MSP_SET_MOTOR au FC via UART2 (GPIO16/17),
  comme deja configure pour eviter le conflit avec le port de flash USB (UART0).

  /!\ SECURITE /!\
  - RETIRE LES HELICES avant tout test
  - Le FC doit etre DESARME
  - Tant que "ARM" n'a pas ete recu, motor1 reste force a 1000 quoi qu'il arrive
*/

const uint16_t STEP = 20;          // increment en us par commande UP/DOWN
const uint16_t MIN_THROTTLE = 1000;
const uint16_t MAX_THROTTLE = 2000;

uint16_t motor1Value = MIN_THROTTLE;
bool motorsEnabled = false;

unsigned long lastSendTime = 0;
const unsigned long sendInterval = 20; // 50Hz, au-dessus du minimum 10Hz requis par INAV

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

void sendMotorCommand() {
  uint16_t motors[8] = {1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000};
  motors[0] = motorsEnabled ? motor1Value : MIN_THROTTLE;

  uint8_t payload[16];
  for (int i = 0; i < 8; i++) {
    payload[i * 2]     = motors[i] & 0xFF;
    payload[i * 2 + 1] = (motors[i] >> 8) & 0xFF;
  }
  sendMSP(214, payload, sizeof(payload));
}

void processCommand(String cmd) {
  cmd.trim();

  if (cmd == "ARM") {
    motorsEnabled = true;
    Serial.println("STATUS:ARMED");
  } else if (cmd == "DISARM") {
    motorsEnabled = false;
    motor1Value = MIN_THROTTLE;
    Serial.println("STATUS:DISARMED");
  } else if (cmd == "UP") {
    int v = motor1Value + STEP;
    motor1Value = (v > MAX_THROTTLE) ? MAX_THROTTLE : v;
    Serial.println("VAL:" + String(motor1Value));
  } else if (cmd == "DOWN") {
    int v = motor1Value - STEP;
    motor1Value = (v < MIN_THROTTLE) ? MIN_THROTTLE : v;
    Serial.println("VAL:" + String(motor1Value));
  } else if (cmd.startsWith("SET:")) {
    int v = cmd.substring(4).toInt();
    v = constrain(v, MIN_THROTTLE, MAX_THROTTLE);
    motor1Value = v;
    Serial.println("VAL:" + String(motor1Value));
  }
}