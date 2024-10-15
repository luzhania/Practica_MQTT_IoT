#include <WiFi.h>
#include <PubSubClient.h>
#include <vector>
#define RELAY_PIN 26
#define SOUND_SENSOR_PIN 35

using namespace std;
class WiFiConnection
{
private:
  const char *SSID;
  const char *PASSWORD;

public:
  WiFiConnection(const char *SSID, const char *PASSWORD)
      : SSID(SSID), PASSWORD(PASSWORD) {}

  void connect()
  {
    WiFi.begin(SSID, PASSWORD);
    while (WiFi.status() != WL_CONNECTED)
    {
      delay(1000);
      Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");
  }
};

class Observer {
public:
    virtual void update(int data) = 0;
};

class MQTTClient : public Observer {
private:
    PubSubClient client;
    const char* SERVER;
    const int PORT;
    const char* TOPIC;

public:
    MQTTClient(Client& espClient, const char* SERVER, int PORT, const char* TOPIC) 
        : client(espClient), SERVER(SERVER), PORT(PORT), TOPIC(TOPIC) {}

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
        if (client.publish(TOPIC, message.c_str())) {
            Serial.println("Data sent successfully");
        } else {
            Serial.println("Error sending data");
        }
    }

    void update(int data) override {
        publish(String(data));
    }

    void loop() {
        if (!client.connected()) {
            reconnect();
        }
        client.loop();
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

    void notifyObservers(int data) {
        for (Observer* observer : observers) {
            observer->update(data);
        }
    }
};


class SoundSensor : public Subject {
private:
    int pin;

public:
    SoundSensor(int pin) : pin(pin) {}

    int readValue() {
        return analogRead(pin);
    }

    void checkSoundLevel() {
        int sensorValue = readValue();
        Serial.print("Sound level: ");
        Serial.println(sensorValue);
        notifyObservers(sensorValue);
    }
};



WiFiClient espClient;
WiFiConnection wifi("HUAWEI-2.4G-M6xZ", "HT7KU2Xv");
MQTTClient mqtt(espClient, "broker.hivemq.com", 1883, "titos/place/sound");
SoundSensor soundSensor(SOUND_SENSOR_PIN);

void setup() {
    Serial.begin(115200);
    wifi.connect();
    mqtt.connect();
    
    soundSensor.attach(&mqtt);  // MQTT observará los cambios del sensor de sonido
    // soundSensor.attach(&relay); // El relé observará los cambios del sensor de sonido
}

void loop() {
    mqtt.loop();
    soundSensor.checkSoundLevel();
    delay(1000);
}