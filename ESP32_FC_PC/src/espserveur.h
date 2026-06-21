#include <WiFi.h>
#include <WiFiUdp.h>

extern WiFiServer server;
extern WiFiUDP udp;
extern int udpPort;

void handleUdpDiscovery();
void handleTcpClient();