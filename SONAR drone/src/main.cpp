#include <Arduino.h>
#include "servo_module.h"

String inputString = "";
boolean stringComplete = false;

void setup() {
  Serial.begin(115200);
  Serial.println("=== Servo Control ===");
  Serial.println("Entrez une position (0-180) et appuyez sur Entree");
  Serial.println("Tapez 'q' pour arreter le servo");
  initServo();
}

void loop() {
  if (Serial.available()) {
    char inChar = Serial.read();
    
    // Ajouter le caractère à la chaîne
    if (inChar == '\n' || inChar == '\r') {
      stringComplete = true;
    } else {
      inputString += inChar;
    }
  }

  // Traiter la chaîne si elle est complète
  if (stringComplete) {
    inputString.trim();
    
    if (inputString.length() > 0) {
      if (inputString.equalsIgnoreCase("q")) {
        stopServo();
        Serial.println("Servo arrête.");
      } else {
        // Convertir la chaîne en nombre
        int position = inputString.toInt();
        
        // Vérifier que c'est un nombre valide
        if (position >= 0 && position <= 180) {
          Serial.print("Deplacement vers ");
          Serial.print(position);
          Serial.println("°");
          moveServo(position);
        } else {
          Serial.println("Erreur: entrez une valeur entre 0 et 180");
        }
      }
    }
    
    // Réinitialiser la chaîne
    inputString = "";
    stringComplete = false;
  }

  if (!isServoStopped()) {
    // Code pour autres actions si nécessaire
  }
}

