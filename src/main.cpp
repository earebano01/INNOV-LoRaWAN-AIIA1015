#include <Arduino.h>
#include <TinyGPS++.h>
#include <SPI.h>
#include <LoRa.h>
#include <DHT.h>
#include <MKRWAN.h>
#include <TimeLib.h> // For daysInMonth() function

// Define your timezone offset in minutes
const int timezone_minutes = 240; // For Atlantic Standard Time (AST), 4 hours behind UTC
// Define your timezone offset in hours
const int timezone_hours = timezone_minutes / 60;

LoRaModem modem;
TinyGPSPlus gps;

String appEui = "0000000000000000";
String appKey = "529C0F725A62FB4ED298461F8C6139F2";

#define DHTPIN 2
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

float hum;
float temp;

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);
  while (!Serial);

  if (!modem.begin(US915)) {
    Serial.println("Failed to start module");
    while (1);
  }

  int connected = modem.joinOTAA(appEui, appKey);
  while (!connected) {
    Serial.println("Retry...");
    if (!modem.joinOTAA(appEui, appKey)) {
      Serial.println("Fail");
    } else {
      break;
    }
  }

  dht.begin();
}

void loop() {
  while (Serial1.available() > 0) {
    gps.encode(Serial1.read());
  }

  if (gps.location.isUpdated() && gps.date.isUpdated() && gps.time.isUpdated() && gps.speed.isUpdated() && gps.course.isUpdated()) {
    temp = dht.readTemperature();
    hum = dht.readHumidity();

    if (isnan(temp) || isnan(hum)) {
      Serial.println("Failed to read from DHT sensor!");
      delay(2000);
      return;
    }

    String payload = String(temp) + "," + String(hum) + "," +
                     String(gps.time.hour() - 4) + ":" + String(gps.time.minute()) + ":" + String(gps.time.second()) + "," +
                     String(gps.location.lat(), 6) + "," + String(gps.location.lng(), 6) + "," +
                     String(gps.speed.knots()) + "," + String(gps.course.deg()) + "," +
                     String(gps.date.day()) + "," + String(gps.date.month()) + "," + String(gps.date.year());

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
    delay(5000);

    modem.beginPacket();
    modem.print(payload);
    if (modem.endPacket() == false) {
      Serial.println("Error sending packet");
    }

    delay(5000);
  }
}