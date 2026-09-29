#include "esp_camera.h"
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>

// ================== WIFI ==================
const char* ssid = "HUAWEI72";
const char* password = "masandemkha2";

// ================== FLASK SERVER ==================
const char* serverName = "http://192.168.43.239:5000/data-camera"; 

// ================== CAMERA PINS ==================
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22
#define FLASH_LED_PIN 4
WebServer server(80);

String esp32_id = "CAM_01";

// ================== CAMERA INIT ==================
void startCamera() {
  camera_config_t config;

  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;

  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;

  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;

  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;

  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;

  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  if (psramFound()) {
    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 3;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 5;
    config.fb_count = 1;
  }

  if (esp_camera_init(&config) != ESP_OK) {
    Serial.println("Camera init failed");
    return;
  }
}

// ================== STREAM ==================
void handle_jpg_stream(void) {
  WiFiClient client = server.client();

  String response =
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n\r\n";
  server.sendContent(response);

  while (client.connected()) {
    camera_fb_t * fb = esp_camera_fb_get();
    if (!fb) continue;

    server.sendContent("--frame\r\n");
    server.sendContent("Content-Type: image/jpeg\r\n\r\n");
    server.sendContent((const char *)fb->buf, fb->len);
    server.sendContent("\r\n");

    esp_camera_fb_return(fb);
  }
}

// ================== WEB PAGE ==================
void handle_root() {
  String html = "<html><body><h2>ESP32-CAM</h2>";
  html += "<img src=\"/stream\">";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

// ================== SETUP ==================
void setup() {
  Serial.begin(115200);
  pinMode(FLASH_LED_PIN, OUTPUT);

  startCamera();

  WiFi.begin(ssid, password);
  Serial.print("Connecting");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected");
  Serial.print("Camera URL: http://");
  Serial.println(WiFi.localIP());

  server.on("/", handle_root);
  server.on("/stream", HTTP_GET, handle_jpg_stream);

  server.begin();
}

// ================== LOOP ==================
void loop() {
  server.handleClient();

  static unsigned long lastTime = 0;

  digitalWrite(FLASH_LED_PIN, HIGH); // Turn flash ON
  //delay(10000);


  // Send data to Flask every 2 seconds
  if (millis() - lastTime > 2000) {
    lastTime = millis();

    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;

      http.begin(serverName);
      http.addHeader("Content-Type", "application/json");

      // Send only device ID (clean version)
      String postData = "{\"id\":\"" + esp32_id + "\"}";

      int httpResponseCode = http.POST(postData);

      Serial.print("Response Code: ");
      Serial.println(httpResponseCode);

      http.end();
    }
  }
}