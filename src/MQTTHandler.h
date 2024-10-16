#include <Arduino.h>
#include <PubSubClient.h>
#pragma once

class MQTTHandler {
private:
    PubSubClient client;
    const char* SERVER;
    const unsigned int PORT;
    const char* TOPIC;

public:
    MQTTHandler(Client& espClient, const char* SERVER, unsigned int PORT, const char* TOPIC)
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
                client.subscribe(TOPIC);
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