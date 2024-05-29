#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
// Replace with your network credentials
const char* ssid = "D21";
const char* password = "Gty@6464";

// Replace with the public IP address of your Colab notebook
const char* mqtt_server = "34.125.57.249";
const int mqtt_port = 1883; // Non-secure port

WiFiClient espClient;
PubSubClient client(espClient);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
const char* topic = "bank/security";

void setup() {
  Serial.begin(115200);
 
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(240);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
  timeClient.begin();
                             timeClient.setTimeOffset(19800); 
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  // Connect to MQTT broker
  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");
    if (client.connect("CAPTAIN")) {
      Serial.println("connected");
      client.subscribe(topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      delay(1000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void loop() {
  timeClient.update()
  if (!client.connected()) {
    while (!client.connected()) {
      Serial.print("Connecting to MQTT...");
      if (client.connect("CAPTAIN")) {
        Serial.println("connected");
        client.subscribe(topic);
      } else {
        Serial.print("failed, rc=");
        Serial.print(client.state());
        delay(1000);
      }
    }
  }
  
  // Publish a message every 5 seconds
  static unsigned long lastMsg = 0;
  if (millis() - lastMsg > 2000) {
    lastMsg = millis();
    String message = "CAPTAIN says hello at: "+(timeClient.getFormattedTime());
    client.publish(topic, message.c_str());
  }

  client.loop();
}
