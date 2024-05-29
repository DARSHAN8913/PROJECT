#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "base64.h"
#include "esp_camera.h"

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
const int smtp_port = 465;
const char* email = "YOUR_EMAIL@gmail.com";
const char* email_password = "YOUR_EMAIL_PASSWORD";
const char* recipient_email = "RECIPIENT_EMAIL@gmail.com";
WiFiServer server(80);

// Camera configuration
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

const int camera_xclk_freq = 20000000;

// Function prototypes
void handleClient(WiFiClient client);
void captureAndSendEmail();

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  server.begin();

  // Initialize camera
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
  config.xclk_freq_hz = camera_xclk_freq;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_UXGA;
  config.jpeg_quality = 10;
  config.fb_count = 2;

  // Initialize camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
}

void loop() {
  WiFiClient client = server.available();
  if (client) {
    handleClient(client);
  }

  captureAndSendEmail();
  delay(60000); // Capture and send image every 1 minute
}

void handleClient(WiFiClient client) {
  // Read incoming message from client
  String message = client.readStringUntil('\r');
  Serial.println("Received message: " + message);

  // Process message and trigger actions accordingly
  // Example: If message contains "AUTHENTICATE", trigger authentication process
  // Example: If message contains "INTRUSION_ALERT", trigger intrusion alert process

  // Respond to client (optional)
  client.println("Message received and processed successfully.");
  client.flush();
  client.stop();
}

void captureAndSendEmail() {
  WiFiClientSecure client;
  if (!client.connect(smtp_server, smtp_port)) {
    Serial.println("Failed to connect to SMTP server");
    return;
  }
  
  if (!client.verify(smtp_server, fingerprint)) {
    Serial.println("Certificate mismatch");
    return;
  }

  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Failed to capture image");
    return;
  }

  String emailMessage = "To: " + String(recipient_email) + "\r\n" +
                        "From: " + String(email) + "\r\n" +
                        "Subject: ESP32-CAM Image\r\n" +
                        "Content-Type: image/jpeg\r\n" +
                        "Content-Disposition: attachment; filename=image.jpg\r\n" +
                        "\r\n";
  
  client.println("EHLO " + String(smtp_server));
  client.println("AUTH LOGIN");
  client.println(base64::encode(email));
  client.println(base64::encode(email_password));
  client.println("MAIL FROM:<" + String(email) + ">");
  client.println("RCPT TO:<" + String(recipient_email) + ">");
  client.println("DATA");
  client.println(emailMessage);
  
  client.write(fb->buf, fb->len);

  client.println(".");
  client.println("QUIT");

  esp_camera_fb_return(fb);

  // delay(500);
  Serial.println("Image sent successfully");
}
