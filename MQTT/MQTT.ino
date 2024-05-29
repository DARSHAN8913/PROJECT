#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid = "D21";
const char* password = "Gty@6464";
const char* mqtt_server = "734567180a95421988f60bf004739114.s1.eu.hivemq.cloud"; // Update with your MQTT broker's address
const int mqtt_port = 8884; // Default MQTT port
const char* mqtt_username = "NODEMCU1"; // Update with your MQTT broker's username
const char* mqtt_password = "Star@3232"; // Update with your MQTT broker's password
const char* mqtt_client_id = "MCU1";

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  // delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(240);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(mqtt_client_id, mqtt_username, mqtt_password)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 2 seconds");
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
