// Standart library
#include <cstdint> // uint-types

// BME280
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
// ESP8266
#include <ESP8266WiFi.h>
#include <Wire.h>
#include <SPI.h>
// ThingSpeak
#include "ThingSpeak.h"

// eigener Code
#include "secrets.h" // wichtig: in .gitignore! Enthält SSID, Wlan-Passwort und ThingSpeak Login!


// ThingSpeak
uint64_t myChannelNumber = SECRET_CHANNEL_ID; // defined in secrets.h
const char* myWriteAPIKey = SECRET_WRITE_APIKEY; // defined in secrets.h

// Wlan
const char* ssid = SECRET_SSID; // defined in secrets.h
const char* pass = SECRET_PASS; // defined in secrets.h

// Interval in Sekunden
const uint16_t interval = 60; // Wartezeit nach einer Schleifeniteration

WiFiClient client; // web communication

// Instanz des bme280 Sensors
Adafruit_BME280 bme; // I2C


void setup() {
  Serial.begin(115200);
  while(!Serial); // time to get serial running
  
  //WiFi.init(&Serial); // entkommentieren für debugging (postet Wlan Rohtext)

  WiFi.mode(WIFI_STA); // Legt den Zustand von ESP im Wlan fest (STA = station = client)
  
  // Verbindung mit Thingspeak aufbauen (obwohl Wlan Verbindung noch nicht steht)
  ThingSpeak.begin(client);

  // BME initialisieren (0x76 = Adresse)
  uint32_t status = bme.begin(0x76);

  // Schreibt Nachricht in seriellen Anschluss, wenn Verbindungsprobleme mit BME auftreten
  if (!status) {
      Serial.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
      Serial.print("SensorID was: 0x"); Serial.println(bme.sensorID(),16);
      Serial.print(" ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
      Serial.print(" ID of 0x56-0x58 represents a BMP 280.\n");
      Serial.print(" ID of 0x60 represents a BME 280.\n");
      Serial.print(" ID of 0x61 represents a BME 680.\n");
      while (1) delay(10);
  }

  // Nachricht ausgeben, wenn bis hierhin alles funktioniert hat
  Serial.println("System works fine! Start measuring...");
}

void loop() {
  // Verbinden oder Wiederverbinden mit WiFi
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(SECRET_SSID);

    WiFi.begin(ssid, pass); // Connect to WPA/WPA2 network
    
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(.);
    }
    
    Serial.print("\nConnected with ip: ");
    Serial.println(WiFi.localIP());
  }

  // Eventuell mal nice to have:
  // Measure Signal Strength (RSSI) of Wi-Fi connection
  //int64_t rssi = WiFi.RSSI();

  // setzt die neuen Messwerte für die einzelnen Felder entsprechend... (Temperatur, Humidity, Pressure)
  ThingSpeak.setField(1, bme.readTemperature());
  ThingSpeak.setField(2, bme.readHumidity());
  ThingSpeak.setField(3, bme.readPressure());

  // ... und sendet sie zur ThingSpeak Website
  int32_t httpCode = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  
  // prüfen ob Fehler aufgetreten sind und entsprechend melden
  if (httpCode == 200)
    Serial.println("Channel write successful.");
  else
    Serial.println("Problem writing to channel. HTTP error code"  + String(httpCode));

  // Intervall-Zeit abwarten und mit nächster Iteration fortfahren
  delay(interval * 1000); 
}
