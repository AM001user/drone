#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include "espserveur.h"

WiFiServer server(80);
WiFiUDP udp;
int udpPort = 12345;
const char* discoveryRequest = "ESP32_DISCOVER";
// CORRECTION 1 : On intègre directement le séparateur pour correspondre au script Python
const char* discoveryReplyPrefix = "ESP32_IP:"; 

void handleUdpDiscovery() {
  int packetSize = udp.parsePacket();
  if (packetSize > 0) {
    char buffer[128] = {0};
    int len = udp.read(buffer, sizeof(buffer) - 1);
    if (len > 0) {
      buffer[len] = '\0';
    }

    String request = String(buffer);
    request.trim(); // Sécurité : on nettoie les espaces/sauts de ligne cachés

    if (request == discoveryRequest) {
      // CORRECTION 1 (suite) : Formatage propre de la réponse
      String reply = String(discoveryReplyPrefix) + WiFi.localIP().toString();
      
      udp.beginPacket(udp.remoteIP(), udp.remotePort());
      udp.print(reply);
      udp.endPacket();
      Serial.printf("UDP Reçu de %s. Réponse de découverte envoyée.\n", udp.remoteIP().toString().c_str());
    }
  }
}

void handleTcpClient() {
  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  Serial.println("\nNouveau client TCP connecté !");
  client.setTimeout(10000); // 10 secondes max pour les fonctions de lecture natives

  // Étape 1 : Handshake
  client.println("ESP32:READY");
  Serial.println("Handshake envoyé : ESP32:READY");

  unsigned long start = millis();
  bool handshakeReceived = false;
  
  while (client.connected() && millis() - start < 10000) {
    if (client.available()) {
      String line = client.readStringUntil('\n');
      line.trim();
      if (line.length() == 0) continue;

      Serial.println("TCP reçu : " + line);
      if (line == "PC:HELLO") {
        client.println("ESP32:HELLO");
        Serial.println("Handshake confirmé : ESP32:HELLO");
        handshakeReceived = true;
        break;
      }
    }
    delay(10);
  }

  if (!handshakeReceived) {
    Serial.println("Aucun handshake valide du PC reçu, fermeture.");
    client.stop();
    return;
  }

  // Étape 2 : Échange de messages multiples
  for (int i = 0; i < 5 && client.connected(); i++) {
    unsigned long startMsg = millis();
    bool received = false;

    while (client.connected() && millis() - startMsg < 10000) {
      if (client.available()) {
        String line = client.readStringUntil('\n');
        line.trim();
        if (line.length() == 0) continue;

        Serial.println("Message reçu du PC : " + line);
        client.println("ESP32:ACK:" + String(i + 1));
        Serial.println("Réponse envoyée : ESP32:ACK:" + String(i + 1));
        received = true;
        break;
      }
      delay(10);
    }

    if (!received) {
      Serial.println("Timeout d'attente du message PC.");
      break;
    }
  }

  // Étape 3 : Fermeture propre
  client.println("ESP32:BYE");
  Serial.println("Fin de session : ESP32:BYE");
  
  // CORRECTION 2 : Laisse le temps aux paquets TCP de partir avant de détruire le socket
  client.flush(); 
  delay(50); 
  client.stop();
  Serial.println("Client déconnecté proprement.");
}

