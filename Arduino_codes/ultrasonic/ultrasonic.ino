#include <WiFi.h>
#include <HTTPClient.h>

// ---------------- ULTRASONIC ----------------
#define TRIG_PIN 18
#define ECHO_PIN 17

// ---------------- LED ----------------
#define LED_PIN 14

// ---------------- VARIABLES ----------------
float duration;
float distance;
int objectDetected = 0;  // 1 = detected, 0 = not detected

// ---------------- WIFI ----------------
const char* ssid = "HUAWEI72";
const char* password = "masandemkha2";

// ---------------- SERVER ----------------
const char* serverName = "http://192.168.43.239:5000/ultr-data";
const char* esp32_id = "Group_12_ULTRASONIC";

// ==================================================
void setup() {

  Serial.begin(115200);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);

  digitalWrite(LED_PIN, LOW);

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

// ==================================================
void loop() {

  // -------- TRIGGER ULTRASONIC --------
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // -------- READ ECHO --------
  duration = pulseIn(ECHO_PIN, HIGH);

  // -------- CALCULATE DISTANCE --------
  distance = duration * 0.017;

  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  // -------- DETECTION --------
  if (distance > 0 && distance < 50) {
    objectDetected = 1;
    digitalWrite(LED_PIN, HIGH);
    Serial.println("Object Detected");
  } else {
    objectDetected = 0;
    digitalWrite(LED_PIN, LOW);
    Serial.println("No Object");
  }

  // ---------------- HTTP POST ----------------
  if (WiFi.status() == WL_CONNECTED) {

    HTTPClient http;
    http.begin(serverName);
    http.addHeader("Content-Type", "application/json");

    String postData = "{\"id\":\"" + String(esp32_id) +
                      "\",\"object_detected\":" + String(objectDetected) +
                      ",\"distance\":" + String(distance) + "}";

    Serial.println("Sending: " + postData);

    int httpResponseCode = http.POST(postData);

    if (httpResponseCode > 0) {
      Serial.print("HTTP Response: ");
      Serial.println(httpResponseCode);
    } else {
      Serial.print("POST Error: ");
      Serial.println(httpResponseCode);
    }

    http.end();
  }

  delay(2000);
}
