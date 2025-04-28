#include <WiFi.h>
#include <PubSubClient.h>
#include "SensorHumedad.h"
#include "ActuadorRiego.h"

// Configuración
#define PIN_SENSOR 39     // GPIO39 (ADC)
#define PIN_RELE 4        // GPIO4 para relé

const char* WIFI_SSID = "FLIA COPA MEJIA";
const char* WIFI_PASS = "DcopaMJ2609";
const char* MQTT_BROKER = "broker.hivemq.com";
const char* TOPIC_HUMEDAD = "casa/planta/humedad";
const char* TOPIC_RIEGO = "casa/planta/regar";
const char* TOPIC_MODO = "casa/planta/modo";

WiFiClient espClient;
PubSubClient client(espClient);
SensorHumedad sensor(PIN_SENSOR);
ActuadorRiego riego(PIN_RELE);

bool modoAutomatico = true;
bool estadoRiegoManual = false;

void setupWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) delay(500);
}

void callback(char* topic, byte* payload, unsigned int length) {
  String mensaje;
  for (int i = 0; i < length; i++) mensaje += (char)payload[i];

  // Control manual del relé (si el modo NO es automático)
  if (String(topic) == TOPIC_RIEGO) {
    if (!modoAutomatico) {
      if (mensaje == "ON") {
        riego.comenzarRiego();
        estadoRiegoManual = true;
        Serial.println("Riego MANUAL activado");
      } else if (mensaje == "OFF") {
        riego.detenerRiego();
        estadoRiegoManual = false;
        Serial.println("Riego MANUAL desactivado");
      }
    }
  }

  if (String(topic) == TOPIC_MODO) {
    modoAutomatico = (mensaje == "AUTO");
    Serial.println("Modo: " + String(modoAutomatico ? "AUTOMÁTICO" : "MANUAL"));
    
    if (modoAutomatico) {
      estadoRiegoManual = false;
    }
  }
}

void reconnectMQTT() {
  while (!client.connect("ESP32_Planta_Unificado")) {
    delay(5000);
  }
  client.subscribe(TOPIC_RIEGO);
  client.subscribe(TOPIC_MODO);
}

void setup() {
  Serial.begin(115200);
  setupWiFi();
  client.setServer(MQTT_BROKER, 1883);
  client.setCallback(callback);
  reconnectMQTT();
}

void loop() {
  if (!client.connected()) reconnectMQTT();
  client.loop();

  static unsigned long lastMsg = 0;
  if (millis() - lastMsg > 5000) {
    lastMsg = millis();
    
    float humedad = sensor.leerHumedad();
    char msg[8];
    dtostrf(humedad, 4, 2, msg);
    client.publish(TOPIC_HUMEDAD, msg);
    Serial.println("Humedad: " + String(humedad) + "%");

    if (modoAutomatico) {
      if (humedad < 30 && !estadoRiegoManual) {
        riego.comenzarRiego();
        Serial.println("Riego AUTOMÁTICO activado");
      } else if (humedad >= 30) {
        riego.detenerRiego();
      }
    }
  }
}