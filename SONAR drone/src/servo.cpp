#include <Arduino.h>
#include <Servo.h>
#include "servo_module.h"

Servo myservo;  // create servo object to control a servo
// twelve servo objects can be created on most boards

static const int servoPositions[] = {0, 90, 180};
static const unsigned long moveInterval = 1000;
static int currentPositionIndex = 0;
static unsigned long lastMoveMillis = 0;
static bool servoStopped = false;

void initServo() {
  myservo.attach(9);  // attaches the servo on pin 9 to the servo object
  currentPositionIndex = 0;
  lastMoveMillis = millis();
  servoStopped = false;
  myservo.write(servoPositions[currentPositionIndex]);
}

void stopServo() {
  if (myservo.attached()) {
    myservo.detach();
  }
  servoStopped = true;
}

bool isServoStopped() {
  return servoStopped;
}

void moveServo() {
  if (!myservo.attached()) {
    return;
  }

  unsigned long currentMillis = millis();
  if (currentMillis - lastMoveMillis < moveInterval) {
    return;
  }

  lastMoveMillis = currentMillis;
  currentPositionIndex = (currentPositionIndex + 1) % (sizeof(servoPositions) / sizeof(servoPositions[0]));
  myservo.write(servoPositions[currentPositionIndex]);
}
