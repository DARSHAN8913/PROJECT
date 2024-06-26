#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <eloquent_esp32cam.h>
#include <eloquent_esp32cam/motion/detection.h>
#include <eloquent_esp32cam/extra/esp32/telegram.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#define TELEGRAM_TOKEN "1234567890:AABBCCDDEEFFGGHHIILLMMN-NOOPPQQRRSS"
#define TELEGRAM_CHAT "0123456789"
// WiFi credentials
const char* ssid = "D21";
const char* password = "Gty@6464";
// Google Drive folder ID
const char* googleDriveFolderID = "1v81gFcLLlbi5LD6ueh2pS7luIyLcrtWO";
// Google Drive API endpoint
const char* googleDriveAPI = "www.googleapis.com";
// Google Drive API path to upload files
const char* googleDriveUploadPath = "/upload/drive/v3/files?uploadType=media&supportsAllDrives=true&parents=";
int pir_pin=12;
int tx_pin=13;
int rx_pin=15;

using eloq::camera;
using eloq::motion::detection;
using eloq::wifi;
using eloq::telegram;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
void setup() {
    pinMode(pir_pin,INPUT);
  Serial.begin(115200);
   
 
  mot_setup();

  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(240);
    Serial.println("Connecting...");
  }
  Serial.println("Connected to WiFi");
 timeClient.begin();    timeClient.setTimeOffset(19800);
 while (!telegram.begin().isOk())
      Serial.println(telegram.exception.toString());
}
void mot_setup()
{
   camera.pinout.wrover();
    camera.brownout.disable();
    camera.resolution.vga();
    camera.quality.high();

    // see example of motion detection for config values
    detection.skip(5);
    detection.stride(1);
    detection.threshold(5);
    detection.ratio(0.2);
    detection.rate.atMostOnceEvery(5).seconds();

    // init camera
    while (!camera.begin().isOk())
        Serial.println(camera.exception.toString());

}
void loop() {
 timeClient.update();
 if((timeClient.getHours()<17)&&(timeClient.getHours()>9))
 {
  DayMode();
 }
 else{
  NightMode();
 }
  // if (Serial.available()) {
  //   String input = Serial.readStringUntil('\n');
  //   if (input == "capture") {
  //     captureAndUploadImage();
  //   }
  // }

}
void DayMode()
{
  if(digitalRead(pir_pin))
  {
    //set Tx to high tht starts the timer 
    //keep it until Ack via Rx is not recieved
    //after this check for button trigger via Rx
    // for ex:
    if(digitalRead(rx_pin))
    {
      captureAndUploadImage();
    }
  }
}

void NightMode()
{
  if(digitalRead(pir_pin))
  {
     // capture picture
    if (!camera.capture().isOk()) {
        Serial.println(camera.exception.toString());
        return;
    }
    // run motion detection
    if (!detection.run().isOk()) {
        Serial.println(detection.exception.toString());
        return;
    }
    // on motion, perform action
    if (detection.triggered())
       {
        digitalWrite(tx_pin,HIGH);
       }
  }
}

void captureAndUploadImage() {
  camera_fb_t *fb = NULL; // Pointer to the framebuffer
  fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    return;
  }

  // Connect to Google Drive API
  WiFiClientSecure client;
  if (!client.connect(googleDriveAPI, 443)) {
    Serial.println("Connection to Google Drive API failed");
    return;
  }

  // Construct the POST request
  String requestBody = "";
  requestBody += "POST ";
  requestBody += googleDriveUploadPath;
  requestBody += googleDriveFolderID;
  requestBody += " HTTP/1.1\r\n";
  requestBody += "Host: ";
  requestBody += googleDriveAPI;
  requestBody += "\r\n";
  requestBody += "Content-Type: image/jpeg\r\n";
  requestBody += "Content-Length: ";
  requestBody += String(fb->len);
  requestBody += "\r\n\r\n";
  
  // Send the POST request header
  client.print(requestBody);

  // Send the image data
  client.write(fb->buf, fb->len);

  // Wait for response from the server
  delay(1000);
  while (client.available()) {
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }

  // Return the framebuffer to the pool
  esp_camera_fb_return(fb);
}
