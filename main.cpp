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

// own code
#include "secrets.h" // important: in .gitignore! Contains wlan ssid & password!


// ThingSpeak
uint64_t myChannelNumber = SECRET_CHANNEL_ID; // defined in secrets.h
const char* myWriteAPIKey = SECRET_WRITE_APIKEY; // defined in secrets.h

// Wlan
const char* ssid = SECRET_SSID; // defined in secrets.h
const char* pass = SECRET_PASS; // defined in secrets.h

// periodic status reports
const uint16_t interval = 60; // Update statistics and measures every 60 seconds

WiFiClient client; // web communication

// instance of bme280 sensor
Adafruit_BME280 bme; // I2C


void setup() {
  Serial.begin(115200);
  while(!Serial); // time to get serial running
  
  //WiFi.init(&Serial); // decomment for debugging (posts Wlan Messages)

  // Connect to wlan...
  WiFi.mode(WIFI_STA);
  // ... and internet
  ThingSpeak.begin(client);

  // initialize BME
  uint32_t status = bme.begin(0x76);

  // following if statement shouldn't be neccessary, usefull for debugging purposes
  /*if (!status) {
      Serial.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
      Serial.print("SensorID was: 0x); Serial.println(bme.sensorID(),16");
      Serial.print(" ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
      Serial.print(" ID of 0x56-0x58 represents a BMP 280.\n");
      Serial.print(" ID of 0x60 represents a BME 280.\n");
      Serial.print(" ID of 0x61 represents a BME 680.\n");
      while (1) delay(10);
  }*/

  // write message if everything worked fine
  Serial.println("System set up!");
}

void loop() {
  // Connect or reconnect to WiFi
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

  // Measure Signal Strength (RSSI) of Wi-Fi connection
  //int64_t rssi = WiFi.RSSI();

  // write values to Field 1-3 (Temperature, Humidity, Pressure) of a ThingSpeak Channel
  ThingSpeak.setField(1, bme.readTemperature());
  ThingSpeak.setField(2, bme.readHumidity());
  ThingSpeak.setField(3, bme.readPressure());

  // send values to ThingSpeak website
  int32_t httpCode = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
    
  if (httpCode == 200)
    Serial.println("Channel write successful.");
  else
    Serial.println("Problem writing to channel. HTTP error code"  + String(httpCode));

  // Wait [interval]-seconds to update the channel again
  delay(interval * 1000); 
}
