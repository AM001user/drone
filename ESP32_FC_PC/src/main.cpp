#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include "espserveur.h"
#include "comFCESP.h"

// const char* ssid     = "ADAMPCDELL 0791";
// const char* password = "2_112o8W";

// void setup() {
//   Serial.begin(115200);
//   delay(10);

//   Serial.print("Connexion à ");
//   Serial.println(ssid);
//   WiFi.begin(ssid, password);

//   while (WiFi.status() != WL_CONNECTED) {
//     delay(500);
//     Serial.print(".");
//   }

//   Serial.println("\nWi-Fi connecté !");
//   Serial.print("Adresse IP : ");
//   Serial.println(WiFi.localIP());

//   udp.begin(udpPort);
//   Serial.printf("UDP sur le port %d prêt pour la découverte\n", udpPort);

//   server.begin();
//   Serial.println("Serveur TCP démarré. En attente du PC...");
// }

// void loop() {
//   handleUdpDiscovery();
//   handleTcpClient();
// }

void setup() {
  Serial.begin(115200);  // vers le PC (moniteur série USB)
  mspSerial.begin(MSP_BAUD, SERIAL_8N1, MSP_RX_PIN, MSP_TX_PIN);
  msp.begin(mspSerial);

  delay(500);
  Serial.println("=== Test de connexion MSP ESP32 <-> FC INAV ===");
}

void loop() {
  if (millis() - lastPoll >= POLL_PERIOD_MS) {
    lastPoll = millis();
    testAttitude();
    testAnalog();
    testRc();
    Serial.println("---");
  }
}