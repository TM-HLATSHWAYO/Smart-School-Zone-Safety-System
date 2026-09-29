#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino.h>
#include "DHT.h"

// Define DHT sensor type and pin
#define DHTTYPE DHT22   
#define DHTPIN 27       

// Define LED pin
#define LEDPIN 2   // GPIO2 is often connected to the onboard LED

// WiFi credentials
const char* ssid = "HUAWEI72";
const char* password = "masandemkha2";

// Server endpoint
const char* serverName = "http://192.168.43.239:5000/sensor-data";

// ESP32 identifier
const char* esp32_id = "Group_12";

// Initialize DHT sensor
DHT dht(DHTPIN, DHTTYPE);

void setup() {
    Serial.begin(115200);
    dht.begin();
    pinMode(LEDPIN, OUTPUT);  // Set LED pin as output

    Serial.println("Connecting to WiFi...");
    WiFi.disconnect(true);
    delay(1000);
    WiFi.begin(ssid, password);

    unsigned long startAttemptTime = millis();
    const unsigned long wifiTimeout = 15000;

    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < wifiTimeout) {
        Serial.print(".");
        delay(500);
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nConnected to WiFi!");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("\nFailed to connect to WiFi.");
    }
}

void loop() {
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    if (isnan(temperature) || isnan(humidity)) {
        Serial.println("Failed to read from DHT sensor!");
    } else {
        Serial.print("Temperature: ");
        Serial.print(temperature);
        Serial.println("°C");

        Serial.print("Humidity: ");
        Serial.print(humidity);
        Serial.println("%");

      
        if (humidity > 70.0) {   
            digitalWrite(LEDPIN, HIGH);  // Turn LED ON
            Serial.println("Humidity high → LED ON");
        } else {
            digitalWrite(LEDPIN, LOW);   // Turn LED OFF
            Serial.println("Humidity normal → LED OFF");
        }
    }

    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin(serverName);
        http.addHeader("Content-Type", "application/json");

        String postData = "{\"id\":\"" + String(esp32_id) + 
                          "\",\"temperature\":" + String(temperature) + 
                          ",\"humidity\":" + String(humidity) + "}";

        int httpResponseCode = http.POST(postData);

        if (httpResponseCode > 0) {
            String response = http.getString();
            Serial.println(httpResponseCode);
            Serial.println(response);
        } else {
            Serial.print("Error on sending POST: ");
            Serial.println(httpResponseCode);
        }
        http.end();
    }

    delay(2000);
}
