// 16_Telegram_Notifications.ino


#define MAX_RESOLUTION_VGA
#define DEBUG_ENABLE 1

// Replace with your WiFi credentials
#define WIFI_SSID "D21"
#define WIFI_PASS "Gty@6464"

// Replace with your Telegram credentials
#define TELEGRAM_TOKEN "6074310627:AAFdo4kdVlI8gcPvqmWvqZWcGaSJ0cYa1dI"
#define CHAT_ID "-1001540744854"

#include "esp32cam.h"
// #include "esp32cam/JpegDecoder.h"
// #include "esp32cam/motion/Detector.h"
// #include "esp32cam/motion/SimpleChange.h"
#include "esp32cam/http/TelegramChat.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

using namespace Eloquent::Esp32cam;
unsigned short pirpin=13;
unsigned short altpin=15;
unsigned short ackpin=14;
Cam cam;
// JpegDecoder decoder;
// Motion::SimpleChange algorithm;
// Motion::Detector detector(algorithm);
Http::TelegramChat telegram(TELEGRAM_TOKEN, CHAT_ID);


void setup() {
    Serial.begin(115200);
//  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); 
    Serial.println("Init");
     pinMode(pirpin,INPUT);
     digitalWrite(pirpin,LOW);

      pinMode(altpin,OUTPUT);
     digitalWrite(altpin,LOW);

      pinMode(ackpin,INPUT);
     digitalWrite(ackpin,LOW);
    // see 2_CameraSettings for more details
    cam.aithinker();
    cam.highQuality();
    cam.vga();
    // cam.highestSaturation();
    // cam.disableAutomaticWhiteBalance();
    // cam.disableAutomaticExposureControl();
    // cam.disableGainControl();

    // See 10_Image_Wise_Motion_Detection for details
    // detector.trainFor(30);
    // detector.retrainAfter(33ULL * 600);
    // detector.triggerAbove(0.2);
    // detector.denoise();
    // detector.debounceSeconds(5);

    // algorithm.differBy(20);
    // algorithm.smooth(0.9);
    // algorithm.onlyUpdateBackground();

    while (!cam.begin())
        Serial.println(cam.getErrorMessage());

    while (!cam.connect(WIFI_SSID, WIFI_PASS))
        Serial.println(cam.getErrorMessage());

    /**
     * Initialize bot
     */
    while (!telegram.begin())
        Serial.println(telegram.getErrorMessage());
}


void loop() 
{
    if(digitalRead(pirpin==true))
    {   digitalWrite(altpin,HIGH);
         camdetect();
    }
    else if(digitalRead(ackpin==true))
    {
      digitalWrite(altpin,LOW);
      Serial.println("evry fine........");
    }
}
void camdetect()
{  String message="";
  if (!cam.capture()) 
    {  
        Serial.println(cam.getErrorMessage());
        return;
    }

    // if (!decoder.decode(cam))
    //  {
    //     Serial.println(decoder.getErrorMessage());
    //     return;
    // }

    // if (!detector.update(decoder.luma))
    // {
    //     Serial.println(detector.getErrorMessage());
    //     return;
    // }

    // if (detector.triggered())
    //  { 
        message +="Motion detected. Sending to Telegram...\n";
          message +=telegram.sendText("Motion detected") ? "Text OK\n" : "Text ERROR\n";
           message +=telegram.sendPhoto(cam) ? "Picture OK\n" : "Picture ERROR\n";
        /**
         * ...send message and picture to Telegram chat
         */
       if(message.length()>0)
       {
        Serial.print(message);
       }
    // }
}