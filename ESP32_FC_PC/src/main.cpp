#include <Arduino.h>
#include "controle_moteur.h"
#include "control4motor.h"
#include "MSPSETRAWRC.h"


void setup() {
  Serial.begin(115200);
  FC_SERIAL.begin(FC_BAUD, SERIAL_8N1, RX_PIN, TX_PIN);
  Serial.println("READY");
}

// void loop() {
//   // Lecture des commandes venant du PC
//   if (Serial.available()) {
//     String cmd = Serial.readStringUntil('\n');
//     processCommand(cmd);
//   }

//   // Envoi continu obligatoire vers le FC
//   if (millis() - lastSendTime >= sendInterval) {
//     lastSendTime = millis();
//     sendMotorCommand();
//   }
// }

void loop() {
  // Lecture char par char, exécution immédiate
  while (Serial.available()) {
    char c = (char)Serial.read();
    processCommand2(String(c));  // "a", "z", "s"...
  }

  // Envoi continu 50Hz
  if (millis() - lastSendTime >= sendInterval) {
    lastSendTime = millis();
    modifval();
  }
}

// void setup() {
//   Serial.begin(115200);
//   FC_SERIAL.begin(FC_BAUD, SERIAL_8N1, RX_PIN, TX_PIN);
//   Serial.println("READY - x:ARM/DISARM a:UP e:DOWN z:FRONT s:BACK d:RIGHT q:LEFT r:CENTER");
// }

// unsigned long lastStatusTime = 0;

// void loop() {
//   while (Serial.available()) {
//     char c = (char)Serial.read();
//     processCommandRC(String(c));
//   }

//   // Keepalive 50Hz
//   if (millis() - lastSendTime >= sendInterval) {
//     lastSendTime = millis();
//     sendRawRC();
//   }

//   // Poll statut FC toutes les 500ms
//   if (millis() - lastStatusTime >= 500) {
//     lastStatusTime = millis();
//     requestMSPStatus();
//     readMSPResponse();
//   }
// }