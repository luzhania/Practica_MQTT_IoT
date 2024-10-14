#include <WiFi.h>
#include <PubSubClient.h>
#define relayPin 26

// Configuración WiFi
const char* ssid = "Galaxy S9+7c14";
const char* password = "betitox007.,";

// Configuración MQTT
const char* mqtt_server = "broker.hivemq.com"; // Broker MQTT público
const int mqtt_port = 1883; // Puerto del broker
const char* mqtt_topic = "titos/place/sonido"; // Tema MQTT

// Pines del sensor KY-037
const int soundSensorPin = 35; // Asegúrate de usar un pin analógico

WiFiClient espClient;
PubSubClient client(espClient);

// Función para conectarse al WiFi
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Conectando a ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("Conectado a WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

// Función para reconectar MQTT en caso de desconexión
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a client ID based on the ESP32's unique chip ID
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);

    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Subscribe to the topic to receive LED control messages
      client.subscribe("titos/place/test");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(soundSensorPin, INPUT);
  pinMode(relayPin, OUTPUT);
  
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setKeepAlive(180);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Leer el valor del sensor KY-037
  int sensorValue = analogRead(soundSensorPin);
  // int sensorData = digitalRead(soundSensorPin);
  Serial.print("Nivel de sonido: ");
  Serial.println(sensorValue);

  // Convertir valor a String
  String payload = String(sensorValue);

  // Publicar los datos del sensor en el tema MQTT
  if (client.publish(mqtt_topic, payload.c_str())) {
    Serial.println("Datos enviados exitosamente");
  } else {
    Serial.println("Error al enviar los datos");
  }
    digitalWrite(relayPin, LOW);

  delay(3000); // Esperar 5 segundos antes de la siguiente lectura
}
