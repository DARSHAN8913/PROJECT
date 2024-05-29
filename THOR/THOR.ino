#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// Replace with your network credentials
const char* ssid = "D21";
const char* password = "Gty@6464";

// Replace with the public IP address of your Colab notebook
const char* mqtt_server = "34.125.101.33";
const int mqtt_port = 1883; // Non-secure port

WiFiClient espClient;
PubSubClient client(espClient);
const char* topic = "bank/security";

// NTP Client settings
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

void setup() {
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(240);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.print("MQTT Server IP: ");
  Serial.println(mqtt_server);

  // Check MQTT server reachability
  Serial.print("Pinging MQTT server...");
  IPAddress serverIP;
  if (WiFi.hostByName(mqtt_server, serverIP)) {
    Serial.print("Server IP: ");
    Serial.println(serverIP);
  } else {
    Serial.println("Unable to resolve MQTT server IP.");
  }

  // Initialize NTPClient
  timeClient.begin();
  timeClient.setTimeOffset(19800);

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  // Connect to MQTT broker
  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");
    if (client.connect("THOR")) {
      Serial.println("connected");
      client.subscribe(topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      delay(2000); // Reduced delay for reconnection
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
  if (!client.connected()) {
    while (!client.connected()) {
      Serial.print("Connecting to MQTT...");
      if (client.connect("THOR")) {
        Serial.println("connected");
        client.subscribe(topic);
      } else {
        Serial.print("failed, rc=");
        Serial.print(client.state());
        delay(2000); // Reduced delay for reconnection
      }
    }
  }
  
  // Publish a message every 5 seconds
  static unsigned long lastMsg = 0;
  if (millis() - lastMsg > 5000) {
    lastMsg = millis();
    timeClient.update();
    String formattedTime = timeClient.getFormattedTime();
    String message = "THOR says hello at " + formattedTime;
    client.publish(topic, message.c_str());
  }

  client.loop();
}
