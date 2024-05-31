 
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

#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

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
#define BOT_TOKEN "XXXXXXXXX:XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
LiquidCrystal_I2C lcd(0x27, 16, 2); 

short h,m,s;
bool ack_flag=false,acs_flag=false;
int modepin=D5;  //cvv
// int rdypin=14;  //takes input from esp32 to display ready message
unsigned int altpin=D6; // D7 takes i/p from esp32 for alert signal
unsigned int ackpin=D7;   //D4 gives a ack signal for the esp32
int pushbtn=D8; //push button for img cap
unsigned long long alt_time=0;
unsigned long long sescount=0;
bool first=true,alt_flag=false,mode_flag=false,mf=true;
unsigned int ftime=0,nextime=0;
unsigned int resptime=0,captime=0;

void PINOUT()
{ pinMode(modepin,OUTPUT);
  pinMode(pushbtn,INPUT);
  pinMode(altpin,INPUT);
  pinMode(ackpin,OUTPUT);
  digitalWrite(altpin,LOW);
  digitalWrite(ackpin,LOW);
  digitalWrite(pushbtn,LOW);
  digitalWrite(modepin,LOW);
}
void wifinit()
{  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
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
}
void fireinit()
{
  
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
void setup()
{     lcd.init();                      // Initialize the LCD
  lcd.backlight();                 // Turn on the backlight
 PINOUT();
 wifinit();
     Serial.begin(115200);    
            timeClient.begin();
            timeClient.setTimeOffset(19800); 
fireinit();
}

void loop()
{  
   timeClient.update();
String LOGS="";
if(mf)
{
LOGS+=Firebase.RTDB.getBool(&fbdo, F("/BANK-UNIT/MODE"), &mode_flag)? "MODE SIGNAL RECIEVED AT: "+(timeClient.getFormattedTime())+"\n" : fbdo.errorReason().c_str();
mf=false;
}   
   if(mode_flag)
   {
      digitalWrite(modepin,HIGH);
   }
   else
   {
     digitalWrite(modepin,LOW);
   }
   if((timeClient.getHours()<17)&&(timeClient.getHours()>9)&&mode_flag)
 {
  LOGS+=DayMode();
 }
 else
 {
  LOGS+=NightMode();
 }
 if(LOGS!="")
 {
  Serial.println(LOGS);
 }
++sescount;
}

String DayMode()
{     String dlog="DAY MODE SESSION";
  if((digitalRead(altpin)==1)||alt_flag)
  {  alt_flag=true;
      dlog+="Person Detected";
    stimer();
  } 
  
   dlog+=Seque();

if(dlog!="DAY MODE SESSION")
{
   return dlog;
}
return ""; 
    // if((m1.length()>0)&&(m1!="BANK-UNIT:\nSession No: "+((String) sescount)+"\n"))
    //  {     
    //    Serial.print(m1);
    //    Serial.println("Session end......................................\n");
    //   }
    //   ++sescount;
}

void stimer()
{   
  if(resptime==0) 
  {
   resptime=addSeconds(timeClient.getHours(),timeClient.getMinutes(),timeClient.getSeconds(),300);
   }
 else if(timeco()!=resptime)
    {      
            lcdw("Respond in:",((String) timeDifference(timeco(),resptime))+"Secs");
     if(digitalRead(pushbtn)==1)
      {   
        if(captime==0)
        {
          captime=addSeconds(timeClient.getHours(),timeClient.getMinutes(),timeClient.getSeconds(),5);
        }
        else if(timeDifference(timeco(),captime)>3)
        {
         lcdw("Stand Still for:",((String) timeDifference(timeco(),captime))+"Secs"); 
        }
        else if(timeDifference(timeco(),captime)<3)
        {
          digitalWrite(ackpin,HIGH);
           lcdw("Stand Still for:",((String) timeDifference(timeco(),captime))+"Secs"); 
        }
        else if(timeco()==captime)
        {
           digitalWrite(ackpin,LOW);
           Firebase.RTDB.getBool(&fbdo, F("/BANK-UNIT/alert"), &acs_flag)? "ACCESS FLAG REC AT: "+(timeClient.getFormattedTime())+"\n" : fbdo.errorReason().c_str();
           if(acs_flag)
           {
            lcdw("ACCESS GRANTED!","");
           }
           else
           {
             lcdw("ACCESS DENIED!!","");
           }
        }

      }
    }
    else if(resptime==timeco())
    {         digitalWrite(ackpin,HIGH);
          lcdw("Timeout!!","ACCESS DENIED!!");
          Firebase.RTDB.setBool(&fbdo, F("/BANK-UNIT/TIME_OUT"), true)? "TIME-OUT SIGNAL SENT AT: "+timeClient.getFormattedTime()+"\n" : fbdo.errorReason().c_str();
           alt_flag=false;  resptime=0;
           delay(3000);
           digitalWrite(ackpin,LOW);
    }

}
      
void  lcdw(String l1,String l2)
{
   lcd.setCursor(0,0);
  lcd.print(l1);
  lcd.setCursor(0,1);
  lcd.print(l2);  
}

String Seque()
{
  String m1=("BANK-UNIT:\nSession No: "+((String) sescount)+"\n"),m2="",m3="";
    
     if(first)
    {      
    m1+=Firebase.RTDB.getInt(&fbdo, ("/INIT/seq"), &ftime) ? "get INITIATING sequence ok: "+((String) ftime)+"\n" : fbdo.errorReason().c_str();
    first=false;
    }
    else if(ftime==timeco())
    {
      // sendDataPrevMillis=micros();
      nextime=addSeconds(timeClient.getHours(),timeClient.getMinutes(),timeClient.getSeconds(),10);
    }
else if (Firebase.ready() && (nextime==timeco()))
  {  
    // sendDataPrevMillis = micros();
    
     nextime=addSeconds(timeClient.getHours(),timeClient.getMinutes(),timeClient.getSeconds(),10);
        // m2 +=(String) tm1;
           m1+= Firebase.RTDB.setInt(&fbdo, ("/BANK-UNIT/seq"), nextime) ? "SEQUENCE SENT SUCCESSFULLY: "+(timeClient.getFormattedTime())+"\n" : fbdo.errorReason().c_str(); 
    
      // nextime=addSeconds(timeClient.getHours(),timeClient.getMinutes(),timeClient.getSeconds(),10); 
  }
  if(m1!=("BANK-UNIT:\nSession No: "+((String) sescount)+"\n"))
  {
    return m1;
  }
  return "";
}

String NightMode()
{  String Nlogs="NIGHT MODE SESSION";
    Nlogs+=Seque();
  if((digitalRead(altpin)==true)&&(alt_flag==false))
 {         Nlogs += Firebase.RTDB.setBool(&fbdo, F("/BANK-UNIT/alert"), true)? "THREAT SIGNAL SENT AT: "+timeClient.getFormattedTime()+"\n" : fbdo.errorReason().c_str();
   alt_flag=true;
   alt_time=micros();
   digitalWrite(ackpin,HIGH); 
  }
  else if((alt_flag==true)&&(micros()-alt_time>=10000000))
{
      alt_flag=false;
       digitalWrite(ackpin,LOW);  
       Nlogs+= Firebase.RTDB.setBool(&fbdo, F("/BANK-UNIT/alert"), false)? "NEUTRAL SIGNAL SENT AT: "+timeClient.getFormattedTime()+"\n" : fbdo.errorReason().c_str();
       Nlogs+="ACK signal sent to esp32: "+((String) (timeClient.getHours()*10000)+(timeClient.getMinutes()*100)+timeClient.getSeconds() )+"\n";
}
  if(Nlogs!="NIGHT MODE SESSION")
  {
    return Nlogs;
  }
  return "";
}

int timeDifference(int a, int b) {
    int seconds_a = (a % 100) + ((a / 100) % 100) * 60 + (a / 10000) * 3600;
    int seconds_b = (b % 100) + ((b / 100) % 100) * 60 + (b / 10000) * 3600;

    return std::abs(seconds_a - seconds_b);
}

// void still()
// {  
//   for(int i=5;i>0;i++)
//   {  lcd.setCursor(0,0);
//     lcd.print("Stand Still for:");
//     lcd.setCursor(0,1);
//     lcd.print(i);  lcd.print("Secs");
//     delay(1000);

//   }
//   bnm=true;
// }
int timeco()
{  
  int miui=(timeClient.getHours()*10000)+(timeClient.getMinutes()*100)+timeClient.getSeconds();
  return miui;
}

int totalSeconds(int time) {
    int hours = time / 10000;
    int minutes = (time / 100) % 100;
    int seconds = time % 100;

    return hours * 3600 + minutes * 60 + seconds;
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
int TimeDiff_SEP_INT(int h1, int s, int s1, int h2, int m2, int s2)      //past t1<t2 present
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
