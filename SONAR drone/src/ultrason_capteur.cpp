#include "ultrason_capteur.h"

static const unsigned long ULTRASON_MAX_DURATION = 30000UL; // 30 ms, ~5 m max

void initUltrason() {
  pinMode(ULTRASON_TRIGGER_PIN, OUTPUT);
  pinMode(ULTRASON_ECHO_PIN, INPUT);
  digitalWrite(ULTRASON_TRIGGER_PIN, LOW);
}

unsigned long readUltrasonDuration() {
  // Envoie une impulsion de 10 µs sur la broche trigger
  digitalWrite(ULTRASON_TRIGGER_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(ULTRASON_TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(ULTRASON_TRIGGER_PIN, LOW);

  // Lecture de la durée du signal d'écho
  unsigned long duration = pulseIn(ULTRASON_ECHO_PIN, HIGH, ULTRASON_MAX_DURATION);
  return duration;
}

float readUltrasonDistanceCm() {
  unsigned long duration = readUltrasonDuration();
  if (duration == 0) {
    return -1.0f; // aucun objet détecté ou hors portée
  }
  float distanceCm = duration / 58.0f;
  return distanceCm;
}

void printUltrasonDistance() {
  float distance = readUltrasonDistanceCm();
  if (distance < 0) {
    Serial.println("Ultrason: hors portée ou aucun écho");
  } else {
    Serial.print("Ultrason: ");
    Serial.print(distance, 1);
    Serial.println(" cm");
  }
}
