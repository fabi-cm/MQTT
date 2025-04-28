#include <WiFi.h>
#include <PubSubClient.h>
#include "ActuadorRiego.h"

#define PIN_RELE 4  // GPIO4 controla el relé

const char* WIFI_SSID = "FLIA COPA MEJIA";
const char* WIFI_PASS = "DcopaMJ2609";
const char* MQTT_BROKER = "broker.hivemq.com";
const char* TOPIC_RIEGO = "casa/planta/regar";

WiFiClient espClient;
PubSubClient client(espClient);
ActuadorRiego riego(PIN_RELE);

void setupWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) delay(500);
}

void callback(char* topic, byte* payload, unsigned int length) {
  String mensaje;
  for (int i = 0; i < length; i++) mensaje += (char)payload[i];

  if (mensaje == "ON") {
    riego.comenzarRiego();
    Serial.println("Riego activado");
  } else if (mensaje == "OFF") {
    riego.detenerRiego();
    Serial.println("Riego detenido");
  }
}

void reconnect() {
  while (!client.connect("ESP32_Actuador")) {
    delay(5000);
  }
  client.subscribe(TOPIC_RIEGO);
}

void setup() {
  Serial.begin(115200);
  setupWiFi();
  client.setServer(MQTT_BROKER, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();
}