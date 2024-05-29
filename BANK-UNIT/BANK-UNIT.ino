
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
#define USER_EMAIL "rakasvan1@gmail.com"
#define USER_PASSWORD "Gta@653"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

short h,m,s;
bool ack_flag=false;
// int rdypin=14;  //takes input from esp32 to display ready message
unsigned int altpin=13; // D7 takes i/p from esp32 for alert signal
unsigned int ackpin=2;   //D4 gives a ack signal for the esp32
// int pushbtn=3; //push button for img cap
unsigned long long sendDataPrevMillis = 0;
unsigned int tm1=0,tm2=0;  
unsigned long long alt_time=0;
unsigned long count = 0;
unsigned long long sescount=0;

bool first=true;
int ftime=0;
unsigned long long total_time=0;
unsigned long long seq_time=0;
// unsigned long long alert_time=0;

bool alt_flag=false;
void setup()
{    
     Serial.begin(115200);

                             WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
                             timeClient.begin();
                             timeClient.setTimeOffset(19800); 

  pinMode(altpin,INPUT);
  pinMode(ackpin,OUTPUT);
  digitalWrite(altpin,LOW);
    digitalWrite(ackpin,LOW);
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
{      total_time=micros();
  String m1="BANK-UNIT:\nSession No: "+((String) sescount)+"\n",m2="",m3="";
   timeClient.update();
     seq_time=micros();
     if(first)
    {      
    m1+=Firebase.RTDB.getInt(&fbdo, ("/INIT/seq"), &ftime) ? "get INITIATING sequence ok: "+((String) ftime)+"\n" : fbdo.errorReason().c_str();
    first=false;
    }
    else if(ftime==timeco())
    {
      sendDataPrevMillis=micros();
    }
else if (Firebase.ready() && (micros() - sendDataPrevMillis > 10000000 || sendDataPrevMillis == 0))
  {  

    sendDataPrevMillis = micros();
    
     tm1=(timeClient.getHours()*10000)+(timeClient.getMinutes()*100)+timeClient.getSeconds();
        m2 +=(String) tm1;
           m1+= Firebase.RTDB.setInt(&fbdo, ("/BANK-UNIT/seq"), tm1) ? "SEQUENCE SENT SUCCESSFULLY: "+m2+"\n" : fbdo.errorReason().c_str(); 
      seq_time=micros()-seq_time;
      m1+="Time Sending for Sequence: "+((String) seq_time)+"\n"; 
  }
  else if((digitalRead(altpin)==true)&&alt_flag==false)
 {    alt_flag=true;
     tm2=(timeClient.getHours()*10000)+(timeClient.getMinutes()*100)+timeClient.getSeconds();
       m3+=(String) tm2;
      m1 += Firebase.RTDB.setBool(&fbdo, F("/BANK-UNIT/alert"), true)? "THREAT SIGNAL SENT!! AT: "+m3+"\n" : fbdo.errorReason().c_str();
    digitalWrite(ackpin,HIGH);         //telling the esp32cam that we recieved the threat signal
    alt_time=micros();
     seq_time=micros()-seq_time;
      m1+="Time Sending for Alert: "+((String) seq_time)+"\n";
    // delay(10000;)
    // 
  }
  else if((alt_flag==true)&&(micros()-alt_time>=10000000))
{
      alt_flag=false;
       digitalWrite(ackpin,LOW);  
       int tm3=(timeClient.getHours()*10000)+(timeClient.getMinutes()*100)+timeClient.getSeconds();
       m3=(String) tm3;
       m1 += Firebase.RTDB.setBool(&fbdo, F("/BANK-UNIT/alert"), false)? "NEUTRAL SIGNAL SENT AT: "+m3+"\n" : fbdo.errorReason().c_str();
       m1+="ACK signal sent to esp32: "+((String) (timeClient.getHours()*10000)+(timeClient.getMinutes()*100)+timeClient.getSeconds() )+"\n";
        seq_time=micros()-seq_time;
      m1+="Time Sending for Neutral: "+((String) seq_time)+"\n";
}
    if((m1.length()>0)&&(m1!="BANK-UNIT:\nSession No: "+((String) sescount)+"\n"))
    {     total_time=micros()-total_time;
      Serial.print(m1+"Total Exec Time For this Session: "+((String) total_time))+"\n";
      Serial.println("Session end......................................\n");
      }
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

  // Serial.printf("Set string... %s\n", Firebase.RTDB.setString(&fbdo, F("/test/string"), F("Hello World!")) ? "ok" : fbdo.errorReason().c_str());

  //   Serial.printf("Get string... %s\n", Firebase.RTDB.getString(&fbdo, F("/test/string")) ? fbdo.to<const char *>() : fbdo.errorReason().c_str());

  //   Serial.println();    
