#include <Arduino.h>
#include "controle_moteur.h"

void setup() {
  Serial.begin(115200);
  FC_SERIAL.begin(FC_BAUD, SERIAL_8N1, RX_PIN, TX_PIN);
  Serial.println("READY");
}

void loop() {
  // Lecture des commandes venant du PC
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    processCommand(cmd);
  }

  // Envoi continu obligatoire vers le FC
  if (millis() - lastSendTime >= sendInterval) {
    lastSendTime = millis();
    sendMotorCommand();
  }
}
