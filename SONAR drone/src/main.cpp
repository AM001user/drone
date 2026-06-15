#include <Arduino.h>
#include "servo_module.h"

void setup() {
  Serial.begin(115200);
  Serial.println("Servo ready");
  Serial.println("Send 'q' to stop the servo.");
  initServo();
}

void loop() {
  if (Serial.available()) {
    char incoming = Serial.read();
    if (incoming == 'q' || incoming == 'Q') {
      stopServo();
      Serial.println("Servo stopped.");
    }
  }

  if (!isServoStopped()) {
    moveServo();
  }
}

