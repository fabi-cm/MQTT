#include <WiFi.h>
#include <PubSubClient.h>
#include "SensorHumedad.h"

#define PIN_SENSOR 39  // GPIO39

const char* WIFI_SSID = "FLIA COPA MEJIA";
const char* WIFI_PASS = "DcopaMJ2609";
const char* MQTT_BROKER = "broker.hivemq.com";
const char* TOPIC_HUMEDAD = "casa/planta/humedad";

WiFiClient espClient;
PubSubClient client(espClient);
SensorHumedad sensor(PIN_SENSOR);

void setupWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) delay(500);
}

void reconnect() {
  while (!client.connect("ESP32_Sensor")) {
    delay(5000);
  }
}

void setup() {
  Serial.begin(115200);
  setupWiFi();
  client.setServer(MQTT_BROKER, 1883);
}

void loop() {
  if (!client.connected()) reconnect();
  
  float humedad = sensor.leerHumedad();
  Serial.print("Humedad leída: ");
  Serial.println(humedad);

  char msg[8];
  dtostrf(humedad, 4, 2, msg); // Convierte float a string
  client.publish(TOPIC_HUMEDAD, msg);
  delay(5000);  // Publica cada 10 segundos
}