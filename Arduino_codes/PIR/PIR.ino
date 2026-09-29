#include <WiFi.h>
#include <HTTPClient.h>

// PIR sensor pin
#define PIR_PIN 25
const int red_led = 27;
const int green_led = 16;

// WiFi credentials
const char* ssid = "HUAWEI72";
const char* password = "masandemkha2";

// Flask server endpoint
const char* serverName = "http://192.168.43.239:5000/pir-data";

// ESP32 ID
const char* esp32_id = "Group_12_PIR";
int motionDetected = 0;

void setup() {

  Serial.begin(115200);
  pinMode(red_led, OUTPUT);
  pinMode(green_led, OUTPUT);

  pinMode(PIR_PIN, INPUT);

  // Connect to WiFi
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void loop() {

    motionDetected = digitalRead(PIR_PIN);

 if (motionDetected == HIGH) {
    Serial.println("Motion Detected");
    digitalWrite(red_led,HIGH);
    digitalWrite(green_led,LOW);
}
else if (motionDetected == LOW) {
    Serial.println("No Motion");
    digitalWrite(red_led,LOW);
    digitalWrite(green_led,HIGH);
}
else {
    Serial.println("Sensor Error");
}
 //-----------------------HTTP POST---------------------------------------------------------------------------------------------------------------------------------//

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverName);
    http.addHeader("Content-Type", "application/json");

      //--------The postData is a JSON-formatted string where each sensor value is associated with a key--------//
    String postData = "{\"id\":\"" + String(esp32_id) + 
                
                  ",\",\"motion_detected\":" + String(motionDetected) + 
                  "}";


    int httpResponseCode = http.POST(postData);
    
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println(httpResponseCode);
      Serial.println(response);
    } 
    
    else {
      Serial.print("Error on sending POST: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  }
  delay(2000);
}