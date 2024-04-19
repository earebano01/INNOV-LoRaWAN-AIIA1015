// Inclure les bibliothèques nécessaires
#include <Arduino.h> // Bibliothèque principale Arduino
#include <TinyGPS++.h> // Bibliothèque pour manipuler les données GPS
#include <SPI.h> // Bibliothèque pour la communication SPI
#include <LoRa.h> // Bibliothèque pour la communication LoRa
#include <DHT.h> // Bibliothèque pour le capteur de température et d'humidité DHT
#include <MKRWAN.h> // Bibliothèque pour le module LoRa MKRWAN
// #include <TimeLib.h> // Bibliothèque pour la fonction daysInMonth()

// Définir le décalage horaire en minutes
const int timezone_minutes = 240; // Pour l'heure normale de l'Atlantique (AST), 4 heures de retard par rapport à l'UTC
// Définir le décalage horaire en heures
const int timezone_hours = timezone_minutes / 60; // Conversion en heures

// Initialiser les objets nécessaires
LoRaModem modem; // Objet pour le module LoRa
TinyGPSPlus gps; // Objet pour les données GPS

// Définir les identifiants de l'application LoRa
String appEui = ""; // Identifiant de l'application
String appKey = ""; // Clé de l'application

// Définir les broches pour le capteur DHT
#define DHTPIN 2 // Broche de données du capteur DHT
#define DHTTYPE DHT11 // Type de capteur DHT utilisé

DHT dht(DHTPIN, DHTTYPE); // Objet pour le capteur DHT

float hum; // Variable pour stocker l'humidité
float temp; // Variable pour stocker la température

// Configuration initiale
void setup() {
  Serial.begin(9600); // Initialiser la communication série
  Serial1.begin(9600); // Initialiser la communication série avec le GPS
  // while (!Serial); // Attendre que le port série soit disponible

  // Initialiser le module LoRa
  if (!modem.begin(US915)) {
    Serial.println("Failed to start module");
    while (1); // Boucle infinie en cas d'échec
  }

  // Rejoindre le réseau LoRaWAN
  int connected = modem.joinOTAA(appEui, appKey);
  while (!connected) {
    Serial.println("Retry..."); // Message de réessai
    if (!modem.joinOTAA(appEui, appKey)) {
      Serial.println("Fail"); // Message en cas d'échec
    } else {
      break; // Sortir de la boucle si la connexion est réussie
    }
  }

  dht.begin(); // Démarrer le capteur DHT
}

// Boucle principale
void loop() {
  // Lire les données GPS disponibles
  while (Serial1.available() > 0) {
    gps.encode(Serial1.read());
  }

  // Vérifier si les données GPS sont mises à jour
  if (gps.location.isUpdated() && gps.date.isUpdated() && gps.time.isUpdated() && gps.speed.isUpdated() && gps.course.isUpdated()) {
    // Lire la température et l'humidité du capteur DHT
    temp = dht.readTemperature();
    hum = dht.readHumidity();

    // Vérifier si les lectures du capteur sont valides
    if (isnan(temp) || isnan(hum)) {
      Serial.println("Failed to read from DHT sensor!"); // Message d'erreur en cas de lecture invalide
      delay(2000); // Attendre 2 secondes
      return; // Sortir de la fonction loop
    }
    
    String payload = String(gps.location.lat(), 6) + "," + String(gps.location.lng(), 6) + "," +
                     String(temp) + "," + String(hum) + "," +
                     String(gps.time.hour() - 4) + ":" + String(gps.time.minute()) + ":" + String(gps.time.second()) + "," +
                     String(gps.date.day()) + "," + String(gps.date.month()) + "," + String(gps.date.year());

    // Afficher les données sur le moniteur série
    Serial.print("\n===================\n");
    Serial.print("DATA SENT\n");
    Serial.print("===================\n");
    Serial.print("Temperature: ");
    Serial.print(temp);
    Serial.println(" °C");
    Serial.print("Humidity: ");
    Serial.print(hum);
    Serial.println(" %\n");
    Serial.println("\nTime: " + String(gps.time.hour() - 4) + ":" + String(gps.time.minute()) + ":" + String(gps.time.second()) + " UTC");
    Serial.println("Status: Active");
    Serial.println("Latitude: " + String(gps.location.lat(), 6));
    Serial.println("Longitude: " + String(gps.location.lng(), 6));
    Serial.println("Speed over ground: " + String(gps.speed.knots()) + " knots");
    Serial.println("Course over ground: " + String(gps.course.deg()) + " degrees");
    Serial.print("Date: ");
    Serial.print(gps.date.day());
    Serial.print("/");
    Serial.print(gps.date.month());
    Serial.print("/");
    Serial.println(gps.date.year());
    delay(5000); // Attendre 5 secondes

    // Envoyer la charge utile via LoRa
    modem.beginPacket();
    modem.print(payload);
    if (modem.endPacket() == false) {
      Serial.println("Error sending packet"); // Message en cas d'échec d'envoi
    }

    delay(5000); // Attendre 5 secondes
  }
}
