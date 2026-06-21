#include <MSP.h>

#define MSP_RX_PIN 16   // ESP32 RX2  <- TXn de la FC
#define MSP_TX_PIN 17   // ESP32 TX2  -> RXn de la FC
#define MSP_BAUD   115200

extern HardwareSerial mspSerial; // UART2 matériel de l'ESP32
extern MSP msp;

extern unsigned long lastPoll;
extern const unsigned long POLL_PERIOD_MS; // 2 Hz suffit pour un test

void testAttitude();
void testAnalog();
void testRc();