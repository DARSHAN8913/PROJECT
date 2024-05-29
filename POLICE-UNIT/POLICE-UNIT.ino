// using namespace std;
#include <Arduino.h>
// #if defined(ESP32) || defined(ARDUINO_RASPBERRY_PI_PICO_W)
// #include <WiFi.h>
#if defined(ESP8266)
#include <ESP8266WiFi.h>
#elif __has_include(<WiFiNINA.h>)
#include <WiFiNINA.h>
#elif __has_include(<WiFi101.h>)
#include <WiFi101.h>
#elif __has_include(<WiFiS3.h>)
#include <WiFiS3.h>
#endif

#include <WiFiUdp.h>
#include <NTPClient.h>
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>

#define WIFI_SSID "D21"
#define WIFI_PASSWORD "Gty@6464"
#define API_KEY "AIzaSyAYX_h_nf04LI8IxDzMmaccX7l6jozJiFM"
#define DATABASE_URL "https://enr2134-2ee7c-default-rtdb.asia-southeast1.firebasedatabase.app/" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app
#define USER_EMAIL "melapob154@molyg.com"
#define USER_PASSWORD "Hello@256"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

int currtime=0,prevtime=0;
unsigned int h,m,s;
// int rdypin=14;  //takes input from esp32 to display ready message
int buzpin=13; //takes i/p from esp32 for alert signal
// int pushbtn=3; //push button for img cap
unsigned long long sendDataPrevMillis = 0,sescount=0;

int tm=0,t2=0,tm1=0;  
bool bf=false;
bool ft=true;
int adj=12000000;
void setup()
{    
     Serial.begin(115200);

                             WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
                             timeClient.begin();
                             timeClient.setTimeOffset(19800); 

  pinMode(buzpin,OUTPUT);
  digitalWrite(buzpin,LOW);
  Serial.print("Connecting to Wi-Fi");
  unsigned long ms = millis();
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(240);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  config.api_key = API_KEY;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  config.database_url = DATABASE_URL;

  config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h
  Firebase.reconnectNetwork(true);
  fbdo.setBSSLBufferSize(4096 , 1024 );
  fbdo.setResponseSize(2048);
 Firebase.begin(&config, &auth);
 Firebase.setDoubleDigits(5);
  config.timeout.serverResponse = 10 * 1000;
}


void loop()
{ int td=0;   
     currtime=timeco();
   String message="POLICE-UNIT\nSession No: "+((String) sescount)+"\n";
  timeClient.update();
    if(ft)
    {    tm1=addSeconds(timeClient.getHours(),timeClient.getMinutes(),timeClient.getSeconds(),20);
      message+=Firebase.RTDB.setInt(&fbdo, ("/INIT/seq"), tm1) ? "INITIATE AT: "+((String) tm1)+"\n" : fbdo.errorReason().c_str();
      ft=false;
    }
    else if(tm1==timeco())
    {
       sendDataPrevMillis = micros();
    }
  else if (Firebase.ready() && (((micros() - sendDataPrevMillis) > 12000000 )))
  {  
    sendDataPrevMillis = micros();
    //  tm=(timeClient.getHours()*10000)+(timeClient.getMinutes()*100)+timeClient.getSeconds();
                  
          message+=Firebase.RTDB.getInt(&fbdo, ("/BANK-UNIT/seq"), &tm) ? "get sequence ok: "+((String) tm)+"\n" : fbdo.errorReason().c_str();
          message+=Firebase.RTDB.getBool(&fbdo, ("/BANK-UNIT/alert"), &bf)? "get bool ok: "+((String) bf)+"\n" : fbdo.errorReason().c_str();
                    
          h=tm/10000;
          m=(tm-h*10000)/100;
          s=tm-h*10000-m*100;
    
             td= timeDifferenceInSeconds(h,m,s,timeClient.getHours(),timeClient.getMinutes(),timeClient.getSeconds());
                 

          if(td>10||bf)
          {      message+="Delayed Time diff: "+((String) td)+"\n";
                   digitalWrite(buzpin,HIGH);
                   message+="Current Time: "+((String) currtime)+"\n"+"Prev Time: "+((String) prevtime)+"\n";
          }
          else{
            message+="Not Delayed Time diff: "+((String) td)+"\n";
               digitalWrite(buzpin,LOW);
          }
          // if(digitalRead(buzpin)){
          //   // ? "ok" : fbdo.errorReason().c_str();
          //             }
         prevtime=tm;
         adj=10000000;
  }
   if((message.length()>0)&&(message!="POLICE-UNIT\nSession No: "+((String) sescount)+"\n"))
     {     Serial.print(message);
      Serial.println("Session end......................................\n");
      }
//  if(digitalRead(buzpin)==true)
//  {    bf=true;
//     Firebase.RTDB.setBool(&fbdo, F("/BANK-UNIT/alert"), true); //? "ok" : fbdo.errorReason().c_str();
//     // delay(10000;)
//     // 
//   }
 ++sescount;
}
int timeco()
{  
  int miui=(timeClient.getHours()*10000)+(timeClient.getMinutes()*100)+timeClient.getSeconds();
  return miui;
}

int addSeconds(int hours, int mins, int secs, int ti) 
{
    int total_secs = hours * 3600 + mins * 60 + secs + ti;
    int new_hours = total_secs / 3600;
    if(new_hours==24)
    {
        new_hours=0;
    }
    int remaining_secs = total_secs % 3600;
    int new_mins = remaining_secs / 60;
    int new_secs = remaining_secs % 60;
    return new_hours * 10000 + new_mins * 100 + new_secs;
}
int timeDifferenceInSeconds(int h1, int s, int s1, int h2, int m2, int s2)      //past t1<t2 present
{
    // Convert both times into seconds
    int totalSeconds1 = h1 * 3600 + s * 60 + s1;
    int totalSeconds2 = h2 * 3600 + m2 * 60 + s2;

    // Calculate the time difference in seconds
    int timeDifference = totalSeconds2 - totalSeconds1;

    return timeDifference;
}
// String inttostr(int n){
//   Stringstream stream;
//   stream<<n; 
//   string n_as_string;
//   stream >> n_as_string;
//   return n_as_string;
// }

  // Serial.printf("Set string... %s\n", Firebase.RTDB.setString(&fbdo, F("/test/string"), F("Hello World!")) ? "ok" : fbdo.errorReason().c_str());

  //   Serial.printf("Get string... %s\n", Firebase.RTDB.getString(&fbdo, F("/test/string")) ? fbdo.to<const char *>() : fbdo.errorReason().c_str());

  //   Serial.println();    
