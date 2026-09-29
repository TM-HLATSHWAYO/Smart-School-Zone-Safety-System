#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino.h>


// Replace with your network credentials
const char* ssid = "HUAWEI72";
const char* password = "masandemkha2";

// Replace with your server's IP address and port
const char* serverName = "http://192.168.43.239:5000/ldr-data";

// Replace with your ESP32 identifier
const char* esp32_id = "Group_12_LDR";

const int LDR_1 = 39;   // GPIO34 FOR LDR 1
const int LED_1  = 12;  // GPIO12 FOR LED

void setup() {
    Serial.begin(115200);

    pinMode(LED_1, OUTPUT); 

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
        Serial.println("\n Connected to WiFi!");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
        Serial.print("MAC Address: ");
        Serial.println(WiFi.macAddress());
    } else {
        Serial.println("\n Failed to connect to WiFi.");
        Serial.println("Check SSID/password, 2.4GHz band, or try a mobile hotspot.");
    }
}

void loop() {
    int ldr_1_Value = analogRead(LDR_1);

    if (ldr_1_Value > 3000) {
        digitalWrite(LED_1, HIGH);
        Serial.print("Dark: ");
    } else {
        digitalWrite(LED_1, LOW);
        Serial.print("Bright: ");
    }
    Serial.println(ldr_1_Value);

    // ----------------------- HTTP POST ----------------------------------------//
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin(serverName);
        http.addHeader("Content-Type", "application/json");

        String postData = "{\"id\":\"" + String(esp32_id) + "\"" + 
                          ",\"ldr_1_value\":" + String(ldr_1_Value) +
                          "}";

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

    delay(500);
}