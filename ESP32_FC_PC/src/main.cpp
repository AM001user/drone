#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>

const char* ssid     = "PC_ADAM 8769";
const char* password = "donaldtrump2023";

WiFiServer server(80);
WiFiUDP udp;
const int udpPort = 12345;
const char* discoveryRequest = "ESP32_DISCOVER";
const char* discoveryReply = "ESP32_IP";

void setup() {
  Serial.begin(115200);
  delay(10);

  Serial.print("Connexion à ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWi-Fi connecté !");
  Serial.print("Adresse IP : ");
  Serial.println(WiFi.localIP());

  udp.begin(udpPort);
  Serial.printf("UDP sur le port %d prêt pour la découverte\n", udpPort);

  server.begin();
  Serial.println("Serveur TCP démarré. En attente du PC...");
}

void handleUdpDiscovery() {
  int packetSize = udp.parsePacket();
  if (packetSize > 0) {
    char buffer[128] = {0};
    int len = udp.read(buffer, sizeof(buffer) - 1);
    if (len > 0) {
      buffer[len] = '\0';
    }

    String request = String(buffer);
    Serial.print("UDP reçu de ");
    Serial.print(udp.remoteIP());
    Serial.print(":");
    Serial.print(udp.remotePort());
    Serial.print(" -> ");
    Serial.println(request);

    if (request == discoveryRequest) {
      String reply = String(discoveryReply) + ":" + WiFi.localIP().toString();
      udp.beginPacket(udp.remoteIP(), udp.remotePort());
      udp.print(reply);
      udp.endPacket();
      Serial.println("Réponse de découverte envoyée à l’ordinateur");
    }
  }
}

void handleTcpClient() {
  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  Serial.println("\nNouveau client TCP connecté !");
  client.setTimeout(10000);

  // Étape 1 : handshake
  client.println("ESP32:READY");
  Serial.println("Handshake envoyé : ESP32:READY");

  unsigned long start = millis();
  bool handshakeReceived = false;
  while (client.connected() && millis() - start < 10000) {
    if (client.available()) {
      String line = client.readStringUntil('\n');
      line.trim();
      if (line.length() == 0) {
        continue;
      }
      Serial.print("TCP reçu : ");
      Serial.println(line);
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
    Serial.println("Aucun handshake du PC reçu, fermeture du client");
    client.stop();
    return;
  }

  // Étape 2 : échange de messages multiples
  for (int i = 0; i < 5 && client.connected(); i++) {
    unsigned long startMsg = millis();
    bool received = false;

    while (client.connected() && millis() - startMsg < 10000) {
      if (client.available()) {
        String line = client.readStringUntil('\n');
        line.trim();
        if (line.length() == 0) {
          continue;
        }

        Serial.print("Message reçu du PC : ");
        Serial.println(line);
        client.println("ESP32:ACK:" + String(i + 1));
        Serial.print("Réponse envoyée : ESP32:ACK:");
        Serial.println(i + 1);
        received = true;
        break;
      }
      delay(10);
    }

    if (!received) {
      Serial.println("Timeout d'attente du message PC, fermeture du client");
      break;
    }
  }

  client.println("ESP32:BYE");
  Serial.println("Fin de session : ESP32:BYE");
  client.stop();
  Serial.println("Client déconnecté.");
}

void loop() {
  handleUdpDiscovery();
  handleTcpClient();
}