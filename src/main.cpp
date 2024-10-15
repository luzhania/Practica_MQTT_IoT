#include <WiFi.h>
#include <PubSubClient.h>
#include <vector>
#include "Utilities.h"

#define RELAY_PIN 32
#define SOUND_SENSOR_PIN 35

using namespace std;

class WiFiConnection {
private:
  const char* SSID;
  const char* PASSWORD;

public:
  WiFiConnection(const char* SSID, const char* PASSWORD)
      : SSID(SSID), PASSWORD(PASSWORD) {}

  void connect() {
    WiFi.begin(SSID, PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");
  }
};

class MQTTHandler {
private:
    PubSubClient client;
    const char* SERVER;
    const unsigned int PORT;
    const char* topic;

public:
    MQTTHandler(Client& espClient, const char* SERVER, unsigned int PORT, const char* topic)
        : client(espClient), SERVER(SERVER), PORT(PORT), topic(topic) {}

    void connect() {
        client.setServer(SERVER, PORT);
        reconnect();
    }

    void reconnect() {
        while (!client.connected()) {
            String clientId = "ESP32Client-" + String(random(0xffff), HEX);
            if (client.connect(clientId.c_str())) {
                Serial.println("Connected to MQTT broker");
                client.subscribe(topic); // Suscribirse al tópico
            } else {
                Serial.print("Error MQTT connection, rc=");
                Serial.println(client.state());
                delay(5000);
            }
        }
    }

    void publish(const String& message, const char* topic) {
        if (client.publish(topic, message.c_str())) {
            Serial.println("Data sent successfully");
        } else {
            Serial.println("Error sending data");
        }
    }

    void setCallback(std::function<void(char*, byte*, unsigned int)> callback) {
        client.setCallback(callback);
    }

    void loop() {
        if (!client.connected()) {
            reconnect();
        }
        client.loop();
    }

    PubSubClient& getClient() {
        return client;
    }
};

class Observer {
public:
  virtual void update(const String& message) = 0;
};

class MQTTSensorPublisher : public Observer {
private:
  MQTTHandler& mqttHandler;

public:
  MQTTSensorPublisher(MQTTHandler& mqttHandler)
      : mqttHandler(mqttHandler) {}

  void update(const String& message) override {
    mqttHandler.publish(message, "titos/place/sound");
  }

  void connect() {
    mqttHandler.connect();
  }

  void loop() {
    mqttHandler.loop();
  }
};

class RelayObserver : public Observer {
private:
  int relayPin;

public:
  RelayObserver(int relayPin) : relayPin(relayPin) {
    pinMode(relayPin, OUTPUT);
  }

  void update(const String& message) override {
    if (message == "ON") {
      digitalWrite(relayPin, HIGH);
      Serial.println("Relay ON");
    } else if (message == "OFF") {
      digitalWrite(relayPin, LOW);
      Serial.println("Relay OFF");
    }
  }
};

class Subject {
private:
  vector<Observer*> observers;

public:
  void attach(Observer* observer) {
    observers.push_back(observer);
  }

  void detach(Observer* observer) {
    observers.erase(remove(observers.begin(), observers.end(), observer), observers.end());
  }

  void notifyObservers(const String& message) {
    for (Observer* observer : observers) {
      observer->update(message);
    }
  }
};

class SoundSensor : public Subject {
private:
  unsigned int pin;

public:
  SoundSensor(unsigned int pin) : pin(pin) {}

  void checkSoundLevel() {
    unsigned int sensorValue = analogRead(pin);
    Serial.print("Sound level: ");
    Serial.println(sensorValue);
    notifyObservers(String(sensorValue));
  }
};

class MQTTActuatorController : public Subject {
private:
  MQTTHandler& mqttHandler;

public:
  MQTTActuatorController(MQTTHandler& mqttHandler) 
      : mqttHandler(mqttHandler) {}

  void setCallback() {
    mqttHandler.setCallback([this](char* topic, byte* payload, unsigned int length) {
      String message;
      for (int i = 0; i < length; i++) {
        message += (char)payload[i];
      }
      Serial.print("Message received: ");
      Serial.println(message);
      notifyObservers(message);
    });
  }

  void connect() {
    mqttHandler.connect();
  }

  void loop() {
    mqttHandler.loop();
  }
};

WiFiClient espClient;
WiFiConnection wifi("Galaxy S9+7c14", "betitox007.,");
MQTTHandler mqttActuatorHandler(espClient, "broker.hivemq.com", 1883, "titos/place/actuator");
MQTTSensorPublisher mqttSensorPublisher(mqttActuatorHandler);
MQTTActuatorController mqttActuatorController(mqttActuatorHandler);
RelayObserver relayObserver(RELAY_PIN);
SoundSensor soundSensor(SOUND_SENSOR_PIN);

void setup() {
  Serial.begin(115200);
  wifi.connect();
  
  mqttActuatorController.connect();

  mqttActuatorController.attach(&relayObserver);
  mqttActuatorController.setCallback();
  
  soundSensor.attach(&mqttSensorPublisher);
}

void loop() {
  mqttSensorPublisher.loop();
  mqttActuatorController.loop();

  Utilities::nonBlockingDelay(500, []()
                              { soundSensor.checkSoundLevel(); });
}