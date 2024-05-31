#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <eloquent_esp32cam.h>
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
int modepin=13;
int capin=15;

using eloq::camera;
using eloq::wifi;
using eloq::telegram;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

void wifset()
{
   Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(240);
    Serial.println("Connecting...");
  }
  Serial.println("Connected to WiFi");
}
void setup() {
    pinMode(modepin,INPUT);
        pinMode(capin,INPUT);
  Serial.begin(115200);
  mot_setup();
 wifset();
 
 timeClient.begin();    timeClient.setTimeOffset(19800);
 while (!telegram.begin().isOk())
      Serial.println(telegram.exception.toString());
}
void mot_setup()
{
   camera.pinout.aithinker();
    camera.brownout.disable();
    camera.resolution.vga();
    camera.quality.high();

    while (!camera.begin().isOk())
        Serial.println(camera.exception.toString());

}
void loop() {
 timeClient.update();
 if((timeClient.getHours()<17)&&(timeClient.getHours()>9)&&(digitalRead(modepin)==1))
 {
  DayMode();
 }
 else{
  NightMode();
 }

}
void DayMode()
{

    if(digitalRead(capin))
    {
      captureAndUploadImage();
    }

}

void NightMode()
{
  if(digitalRead(capin))
  {
     // capture picture
    if (!camera.capture().isOk()) {
        Serial.println(camera.exception.toString());
        return;
    }
    else
    {
          if (telegram.to(TELEGRAM_CHAT).send(camera.frame).isOk())
        Serial.println("Photo sent to Telegram");
    else
        Serial.println(telegram.exception.toString());
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
