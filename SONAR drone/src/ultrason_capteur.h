#ifndef ULTRASON_CAPTEUR_H
#define ULTRASON_CAPTEUR_H

#include <Arduino.h>

// Configuration des broches du capteur ultrason HC-SR04
static const uint8_t ULTRASON_TRIGGER_PIN = 10;
static const uint8_t ULTRASON_ECHO_PIN = 11;

// Initialisation du capteur ultrason
void initUltrason();

// Mesure la distance en microsecondes (durée du signal d'écho)
unsigned long readUltrasonDuration();

// Mesure la distance en centimètres
float readUltrasonDistanceCm();

// Mesure et affiche la distance sur le port série
void printUltrasonDistance();

#endif // ULTRASON_CAPTEUR_H
