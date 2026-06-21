#include <Arduino.h>
#include "comFCESP.h"
#include <MSP.h>

// ============================================================
// Test de connexion MSP : FC -> ESP32 -> PC (lecture seule)
// Aucune commande envoyee a la FC, juste des requetes de lecture
// ============================================================

// ---- Câblage : adapte selon l'UART utilisé sur la FC ----

HardwareSerial mspSerial(2); // UART2 matériel de l'ESP32
MSP msp;

unsigned long lastPoll = 0;
const unsigned long POLL_PERIOD_MS = 500; // 2 Hz suffit pour un test



void testAttitude() {
  msp_attitude_t att;
  if (msp.request(MSP_ATTITUDE, &att, sizeof(att))) {
    Serial.printf("[OK]  ATTITUDE   roll=%.1f deg  pitch=%.1f deg  yaw=%d deg\n",
                  att.roll / 10.0, att.pitch / 10.0, att.yaw);
  } else {
    Serial.println("[FAIL] ATTITUDE  pas de reponse de la FC");
  }
}

void testAnalog() {
  msp_analog_t analog;
  if (msp.request(MSP_ANALOG, &analog, sizeof(analog))) {
    Serial.printf("[OK]  ANALOG     vbat=%.1fV  rssi=%d  amperage=%.2fA\n",
                  analog.vbat / 10.0, analog.rssi, analog.amperage / 100.0);
  } else {
    Serial.println("[FAIL] ANALOG    pas de reponse de la FC");
  }
}

void testRc() {
  msp_rc_t rc;
  if (msp.request(MSP_RC, &rc, sizeof(rc))) {
    Serial.print("[OK]  RC         channels: ");
    for (int i = 0; i < 8; i++) {
      Serial.printf("%d ", rc.channelValue[i]);
    }
    Serial.println();
  } else {
    Serial.println("[FAIL] RC        pas de reponse de la FC");
  }
}

