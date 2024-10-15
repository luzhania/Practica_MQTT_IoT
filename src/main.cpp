#include <WiFi.h>
#include <PubSubClient.h>
#include <vector>

#define RELAY_PIN 26
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

class Observer {
public:
  virtual void update(const String& message) = 0;
};

class MQTTSensorPublisher : public Observer {
private:
  PubSubClient client;
  const char* SERVER;
  const unsigned int PORT;
  const char* publishTopic;

public:
  MQTTSensorPublisher(Client& espClient, const char* SERVER, unsigned int PORT, const char* publishTopic)
      : client(espClient), SERVER(SERVER), PORT(PORT), publishTopic(publishTopic) {}

  void connect() {
    client.setServer(SERVER, PORT);
    reconnect();
  }

  void reconnect() {
    while (!client.connected()) {
      String clientId = "ESP32Client-" + String(random(0xffff), HEX);
      if (client.connect(clientId.c_str())) {
        Serial.println("Connected to MQTT broker");
      } else {
        Serial.print("Error MQTT connection, rc=");
        Serial.println(client.state());
        delay(5000);
      }
    }
  }

  void publish(const String& message) {
    if (client.publish(publishTopic, message.c_str())) {
      Serial.println("Data sent successfully");
    } else {
      Serial.println("Error sending data");
    }
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

  void update(const String& message) override {
    publish(message);
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
  PubSubClient& client;

public:
  MQTTActuatorController(PubSubClient& client) : client(client) {}

  void callback(char* topic, byte* payload, unsigned int length) {
    String message;
    for (int i = 0; i < length; i++) {
      message += (char)payload[i];
    }
    Serial.print("Message received: ");
    Serial.println(message);

    notifyObservers(message);
  }

  void setCallback() {
    client.setCallback([this](char* topic, byte* payload, unsigned int length) {
      callback(topic, payload, length);
    });
  }
};

WiFiClient espClient;
WiFiConnection wifi("HUAWEI-2.4G-M6xZ", "HT7KU2Xv");
MQTTSensorPublisher mqttSensorPublisher(espClient, "broker.hivemq.com", 1883, "titos/place/sound");
MQTTActuatorController mqttActuatorController(mqttSensorPublisher.getClient());
RelayObserver relayObserver(RELAY_PIN);
SoundSensor soundSensor(SOUND_SENSOR_PIN);

void setup() {
  Serial.begin(115200);
  wifi.connect();
  mqttSensorPublisher.connect();

  mqttActuatorController.attach(&relayObserver);
  mqttActuatorController.setCallback();
  
  soundSensor.attach(&mqttSensorPublisher);
}

void loop() {
  mqttSensorPublisher.loop();
  soundSensor.checkSoundLevel();
  delay(1000);
}